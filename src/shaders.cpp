#include "shaders.hpp"

#include <initializer_list>
#include <ios>
#include <format>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <stdexcept>

#include "assert.hpp"

Uniform::Uniform(UniformType a_type, std::string a_name, int a_location, unsigned int a_program)
	:m_type{ a_type }, m_name{ a_name }, m_uniform_location{ a_location }, m_shader_program{ a_program }
{
	if (m_uniform_location == -1) {
		ERROR(std::format("Wrong uniform attribute name \"{}\"", m_name).data());
		throw Error_code::bad_match;
	}
}

Uniform::Uniform()
	:m_type{ UniformType::STANDARD }, m_name{ "" }, m_uniform_location{ -1 }, m_shader_program{ 0 } { }

std::string Uniform::get_name() const {
	return m_name;
}

UniformType Uniform::get_type() const {
	return m_type;
};

Shader_src::Shader_src(ShaderType a_shader_type, const std::string& a_filename) 
	:shader_type{ a_shader_type }, filename{ a_filename }, success{ false }
{
	switch(shader_type) {
		case ShaderType::VERTEX : {
			shader_name = "VERTEX";
			shader_obj = glCreateShader(GL_VERTEX_SHADER);
			break;
		}
		case ShaderType::FRAGMENT : {
			shader_name = "FRAGMENT";
			shader_obj = glCreateShader(GL_FRAGMENT_SHADER);
			break;
		}
		default: {
			ERROR("Shader type not compatible.");
			throw Error_code::glsl_bad_shader_type;
		}
	}

	load_code(filename);
	compile_shader(&code);
}

void Shader_src::load_code(const std::string& filename) {
	using std::ios;
	std::ifstream file_vertex_shader{ "../" + filename, ios::in | ios::binary };

	if (!file_vertex_shader.is_open()) {
		switch (shader_type) {
			case ShaderType::VERTEX: {
				ERROR(std::format("Vertex shader source file {} couldn't be loaded.", filename).data());
				break;
			}
			case ShaderType::FRAGMENT: {
				ERROR(std::format("Fragment shader source file {} couldn't be loaded.", filename).data());
				break;
			}
			default: {
				ERROR(std::format("Unhandled shader type with source file {} couldn't be loaded.", filename).data());
				break;
			}
		}
		throw Error_code::file_init;
	}

	file_vertex_shader.seekg(0, file_vertex_shader.end);
	int len = file_vertex_shader.tellg();
	file_vertex_shader.seekg(0, file_vertex_shader.beg);

	code = new char[len];
	file_vertex_shader.read(code, len);
	code[len-1] = '\0';
}

void Shader_src::compile_shader(char** code) {
	glShaderSource(shader_obj, 1, code, nullptr);
	glCompileShader(shader_obj);
	check_compilation();
}

void Shader_src::check_compilation() {
	glGetShaderiv(shader_obj, GL_COMPILE_STATUS, &success);

	if (!success) {
		glGetShaderInfoLog(shader_obj, 1024, nullptr, infoLog);
		ERROR(std::format("{} shader \"{}\" can't compile.\nGLSL error message:\n{}", shader_name, filename, infoLog).data());
		throw Error_code::glsl_bad_compilation;
	}
}

Shader_src::~Shader_src() {
	glDeleteShader(shader_obj);
	shader_obj = 0;
	delete[] code;
}

Shader_program::Shader_program(std::initializer_list<Shader_src> a_shaders)
	:m_name{ "" }, m_shader_program{ glCreateProgram() }, m_depth_testing{ true }
{
	if (!std::find_if(a_shaders.begin(), a_shaders.end(), [](const Shader_src& sh){ return sh.shader_type == ShaderType::VERTEX; })) {
		ERROR("Shader initializer list lacks vertex shader.");
		throw Error_code::glsl_bad_shader_type;
	}
	if (!std::find_if(a_shaders.begin(), a_shaders.end(), [](const Shader_src& sh){ return sh.shader_type == ShaderType::FRAGMENT; })) {
		ERROR("Shader initializer list lacks fragment shader.");
		throw Error_code::glsl_bad_shader_type;
	}

	for (auto& shader : a_shaders) {
		glAttachShader(m_shader_program, shader.shader_obj);
	}
	glLinkProgram(m_shader_program);
	check_linking();
}

Shader_program::Shader_program(const Shader_program& a_shader_prog)
	:m_name{ a_shader_prog.m_name }, m_shader_program{ a_shader_prog.m_shader_program }, m_depth_testing{ a_shader_prog.m_depth_testing } { }

Uniform& Shader_program::operator[](const std::string& uniform_var) {
	try {
		return m_uniforms.at(uniform_var);
	} catch (std::out_of_range) {
		ERROR(std::format("{}::{} out of range.\n", m_name, uniform_var).c_str());
		throw Error_code::range_error;
	}
}

bool Shader_program::push_uniform(const std::string& uniform_var, UniformType a_type) {
	auto lamb_push_member = [&](std::initializer_list<std::string> member_names){
		for (const auto& member_name : member_names) {
			std::string full_name = uniform_var + "." + member_name;
			m_uniforms[full_name] = Uniform(a_type, full_name, glGetUniformLocation(m_shader_program, full_name.c_str()), m_shader_program);
		}
	};

	if (a_type == UniformType::STANDARD || a_type == UniformType::MODEL_MAT || a_type == UniformType::VIEW_MAT || a_type == UniformType::PROJECTION_MAT || a_type == UniformType::NORMAL_MAT) {
		m_uniforms[uniform_var] = Uniform(a_type, uniform_var, glGetUniformLocation(m_shader_program, uniform_var.c_str()), m_shader_program);
	} else if (a_type == UniformType::MATERIAL) {
		lamb_push_member({ "shininess", "diffuse_map", "specular_map", "emission_map" });
		m_uniforms[uniform_var + ".diffuse_map"]  = DIFFUSE_MAP_ID;
		m_uniforms[uniform_var + ".specular_map"] = SPECULAR_MAP_ID;
		m_uniforms[uniform_var + ".emission_map"] = EMISSION_MAP_ID;
	} else if (a_type == UniformType::LIGHT || a_type == UniformType::POINT_LIGHT || a_type == UniformType::SPOTLIGHT) {
		lamb_push_member({ "color", "pos_dir", "ambient_intens", "diffuse_intens", "specular_intens" });
		if (a_type == UniformType::POINT_LIGHT || a_type == UniformType::SPOTLIGHT) {
			lamb_push_member({ "linear", "constant", "quadratic" });
		}
		if (a_type == UniformType::SPOTLIGHT) {
			lamb_push_member({ "spot_dir", "inner_cutoff", "outer_cutoff" });
		}
	} else {
		return false;
	}
	return true;
}

void Shader_program::set_name(const std::string& a_name) {
	m_name = a_name;
}

void Shader_program::set_uniform(const Light& a_light) {
	std::string light_name = get_uniform_name(UniformType::LIGHT);
	if (light_name == "") {
		ERROR("Light uniform haven't been pushed.");
		throw Error_code::range_error;
	}
	m_uniforms[light_name + ".color"] = a_light.get_color();
	m_uniforms[light_name + ".pos_dir"] = a_light.get_pos_dir();
	m_uniforms[light_name + ".ambient_intens"] = a_light.get_ambient();
	m_uniforms[light_name + ".diffuse_intens"] = a_light.get_diffuse();
	m_uniforms[light_name + ".specular_intens"] = a_light.get_specular();
}


void Shader_program::set_uniform(const PointLight& a_light) {
	std::string point_light_name = get_uniform_name(UniformType::POINT_LIGHT);
	if (point_light_name == "") {
		ERROR("Point light uniform haven't been pushed.");
		throw Error_code::range_error;
	}
	m_uniforms[point_light_name + ".color"] = a_light.get_color();
	m_uniforms[point_light_name + ".pos_dir"] = a_light.get_pos_dir();
	m_uniforms[point_light_name + ".ambient_intens"] = a_light.get_ambient();
	m_uniforms[point_light_name + ".diffuse_intens"] = a_light.get_diffuse();
	m_uniforms[point_light_name + ".specular_intens"] = a_light.get_specular();
	m_uniforms[point_light_name + ".linear"] = a_light.get_linear();
	m_uniforms[point_light_name + ".constant"] = a_light.get_constant();
	m_uniforms[point_light_name + ".quadratic"] = a_light.get_quadratic();
}

void Shader_program::set_uniform(const SpotLight& a_light) {
	std::string spotlight_name = get_uniform_name(UniformType::SPOTLIGHT);
	if (spotlight_name == "") {
		ERROR("Spotlight uniform haven't been pushed.");
		throw Error_code::range_error;
	}
	m_uniforms[spotlight_name + ".color"] = a_light.get_color();
	m_uniforms[spotlight_name + ".pos_dir"] = a_light.get_pos_dir();
	m_uniforms[spotlight_name + ".ambient_intens"] = a_light.get_ambient();
	m_uniforms[spotlight_name + ".diffuse_intens"] = a_light.get_diffuse();
	m_uniforms[spotlight_name + ".specular_intens"] = a_light.get_specular();
	m_uniforms[spotlight_name + ".linear"] = a_light.get_linear();
	m_uniforms[spotlight_name + ".constant"] = a_light.get_constant();
	m_uniforms[spotlight_name + ".quadratic"] = a_light.get_quadratic();
	m_uniforms[spotlight_name + ".inner_cutoff"] = a_light.get_inner_cutoff();
	m_uniforms[spotlight_name + ".outer_cutoff"] = a_light.get_outer_cutoff();
	m_uniforms[spotlight_name + ".spot_dir"] = a_light.get_spot_dir();
}

void Shader_program::set_depth_testing(bool a_option) {
	m_depth_testing = a_option;
}

void Shader_program::check_linking() {
	glGetProgramiv(m_shader_program, GL_LINK_STATUS, &m_success);
	if (!m_success) {
		glGetProgramInfoLog(m_shader_program, 1024, nullptr, m_infoLog);
		ERROR(std::format("Shader linking error.\nGLSL error message:\n{}", m_infoLog).data());
		throw Error_code::glsl_bad_linking;
	}
}

void Shader_program::use() {
	glUseProgram(m_shader_program);
}

Shader_program::~Shader_program() {
	glDeleteProgram(m_shader_program);
}

std::string Shader_program::get_uniform_name(UniformType a_type) const {
	std::string ret{ "" };
	for (const auto& [key, val] : m_uniforms) {
		if (val.get_type() == a_type) {
			ret = key;
			break;
		}
	}
	std::string::size_type pos = ret.find('.');
	if (pos != std::string::npos) {
		return ret.substr(0, pos);
	} else {
		return ret;
	}
}

bool Shader_program::get_depth_testing() const {
	return m_depth_testing;
}

unsigned int Shader_program::get_shader_program() const { 
	return m_shader_program;
}

void Shader_program::debug() const {
	for (const auto& [key, val] : m_uniforms) {
		std::cout << std::format("key: {}\n", key);
	}
}
