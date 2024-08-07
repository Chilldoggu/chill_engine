#include "shaders.hpp"

#include <initializer_list>
#include <ios>
#include <format>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <stdexcept>

#include "assert.hpp"

Uniform::Uniform(std::string a_name, int a_location, unsigned int a_program)
	:m_name{ a_name }, m_uniform_location{ a_location }, m_shader_program{ a_program }
{
	if (m_uniform_location == -1) {
		ERROR(std::format("Wrong uniform attribute name \"{}\"", m_name).data());
		throw Error_code::bad_match;
	}
}

std::string Uniform::get_name() const {
	return m_name;
}

ShaderSrc::ShaderSrc(ShaderType a_shader_type, const std::string& a_filename) 
	:shader_type{ a_shader_type }, filename{ a_filename }
{
	switch(shader_type) {
		case ShaderType::VERTEX:
			shader_obj = glCreateShader(GL_VERTEX_SHADER);
			break;
		case ShaderType::FRAGMENT:
			shader_obj = glCreateShader(GL_FRAGMENT_SHADER);
			break;
		default:
			ERROR("Shader type not compatible.");
			throw Error_code::glsl_bad_shader_type;
	}

	load_code(filename);
	compile_shader(&code);
}

void ShaderSrc::load_code(const std::string& filename) {
	using std::ios;
	std::ifstream file_vertex_shader{ "../" + filename, ios::in | ios::binary };

	if (!file_vertex_shader.is_open()) {
		switch (shader_type) {
			case ShaderType::VERTEX:
				ERROR(std::format("Vertex shader source file {} couldn't be loaded.", filename).data());
				break;
			case ShaderType::FRAGMENT:
				ERROR(std::format("Fragment shader source file {} couldn't be loaded.", filename).data());
				break;
			default:
				ERROR(std::format("Unhandled shader type with source file {} couldn't be loaded.", filename).data());
				break;
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

void ShaderSrc::compile_shader(char** code) {
	glShaderSource(shader_obj, 1, code, nullptr);
	glCompileShader(shader_obj);
	check_compilation();
}

void ShaderSrc::check_compilation() {
	glGetShaderiv(shader_obj, GL_COMPILE_STATUS, &success);

	if (!success) {
		glGetShaderInfoLog(shader_obj, 1024, nullptr, infoLog);
		if (shader_type == ShaderType::VERTEX) {
			ERROR(std::format("{} shader \"VERTEX\" can't compile.\nGLSL error message:\n{}", filename, infoLog).data());
		} else if (shader_type == ShaderType::FRAGMENT) {
			ERROR(std::format("{} shader \"FRAGMENT\" can't compile.\nGLSL error message:\n{}", filename, infoLog).data());
		} else {
			ERROR(std::format("{} shader \"UNKNOWN\" can't compile.\nGLSL error message:\n{}", filename, infoLog).data());
		}
		throw Error_code::glsl_bad_compilation;
	}
}

ShaderSrc::~ShaderSrc() {
	glDeleteShader(shader_obj);
	shader_obj = 0;
	delete[] code;
}

ShaderProgram::ShaderProgram(std::initializer_list<ShaderSrc> a_shaders) {
	if (!std::find_if(a_shaders.begin(), a_shaders.end(), [](const ShaderSrc& sh){ return sh.shader_type == ShaderType::VERTEX; })) {
		ERROR("Shader initializer list lacks vertex shader.");
		throw Error_code::glsl_bad_shader_type;
	}
	if (!std::find_if(a_shaders.begin(), a_shaders.end(), [](const ShaderSrc& sh){ return sh.shader_type == ShaderType::FRAGMENT; })) {
		ERROR("Shader initializer list lacks fragment shader.");
		throw Error_code::glsl_bad_shader_type;
	}

	for (auto& shader : a_shaders) {
		glAttachShader(m_shader_program, shader.shader_obj);
	}
	glLinkProgram(m_shader_program);
	check_linking();
}

Uniform& ShaderProgram::operator[](const std::string& uniform_var) {
	try {
		return m_uniforms.at(uniform_var);
	} catch (std::out_of_range) {
		push_uniform(uniform_var);
		return m_uniforms[uniform_var];
	}
}

void ShaderProgram::push_uniform_struct(const std::string& a_uniform_var, std::initializer_list<std::string> a_member_names) {
	for (const auto& member_name : a_member_names) {
		std::string full_name = a_uniform_var + "." + member_name;
		m_uniforms[full_name] = Uniform(full_name, glGetUniformLocation(m_shader_program, full_name.c_str()), m_shader_program);
	}
}

template<typename It>
void ShaderProgram::push_uniform_struct(const std::string& a_uniform_var, It a_member_name_first, It a_member_name_last) {
	for (auto member_name = a_member_name_first; member_name != a_member_name_last; member_name++) {
		std::string full_name = a_uniform_var + "." + *member_name;
		m_uniforms[full_name] = Uniform(full_name, glGetUniformLocation(m_shader_program, full_name.c_str()), m_shader_program);
	}
}

void ShaderProgram::push_uniform(const std::string& uniform_var) {
	m_uniforms[uniform_var] = Uniform(uniform_var, glGetUniformLocation(m_shader_program, uniform_var.c_str()), m_shader_program);
}

void ShaderProgram::set_name(const std::string& a_name) {
	m_name = a_name;
}

// WARNING: Exception abuse
void ShaderProgram::set_uniform(const std::string& a_dirlight_name, const DirLight& a_light) {
	try {
		m_uniforms.at(a_dirlight_name + ".color") = a_light.get_color();
		m_uniforms.at(a_dirlight_name + ".dir") = a_light.get_dir();
		m_uniforms.at(a_dirlight_name + ".ambient_intens") = a_light.get_ambient();
		m_uniforms.at(a_dirlight_name + ".diffuse_intens") = a_light.get_diffuse();
		m_uniforms.at(a_dirlight_name + ".specular_intens") = a_light.get_specular();
	} catch (std::out_of_range) {
		push_uniform_struct(a_dirlight_name, { "dir", "color", "ambient_intens", "diffuse_intens", "specular_intens" });
	}
}

// WARNING: Exception abuse
void ShaderProgram::set_uniform(const std::string& a_pointlight_name, const PointLight& a_light) {
	try {
		m_uniforms.at(a_pointlight_name + ".color") = a_light.get_color();
		m_uniforms.at(a_pointlight_name + ".pos") = a_light.get_pos();
		m_uniforms.at(a_pointlight_name + ".ambient_intens") = a_light.get_ambient();
		m_uniforms.at(a_pointlight_name + ".diffuse_intens") = a_light.get_diffuse();
		m_uniforms.at(a_pointlight_name + ".specular_intens") = a_light.get_specular();
		m_uniforms.at(a_pointlight_name + ".linear") = a_light.get_linear();
		m_uniforms.at(a_pointlight_name + ".constant") = a_light.get_constant();
		m_uniforms.at(a_pointlight_name + ".quadratic") = a_light.get_quadratic();
	} catch (std::out_of_range) {
		push_uniform_struct(a_pointlight_name, { "pos", "color", "ambient_intens", "diffuse_intens", "specular_intens", "linear", "constant", "quadratic" });
	}
}

// WARNING: Exception abuse
void ShaderProgram::set_uniform(const std::string& a_spotlight_name, const SpotLight& a_light) {
	try {
		m_uniforms.at(a_spotlight_name + ".color") = a_light.get_color();
		m_uniforms.at(a_spotlight_name + ".pos") = a_light.get_pos();
		m_uniforms.at(a_spotlight_name + ".ambient_intens") = a_light.get_ambient();
		m_uniforms.at(a_spotlight_name + ".diffuse_intens") = a_light.get_diffuse();
		m_uniforms.at(a_spotlight_name + ".specular_intens") = a_light.get_specular();
		m_uniforms.at(a_spotlight_name + ".linear") = a_light.get_linear();
		m_uniforms.at(a_spotlight_name + ".constant") = a_light.get_constant();
		m_uniforms.at(a_spotlight_name + ".quadratic") = a_light.get_quadratic();
		m_uniforms.at(a_spotlight_name + ".inner_cutoff") = a_light.get_inner_cutoff();
		m_uniforms.at(a_spotlight_name + ".outer_cutoff") = a_light.get_outer_cutoff();
		m_uniforms.at(a_spotlight_name + ".spot_dir") = a_light.get_spot_dir();
	} catch (std::out_of_range) {
		push_uniform_struct(a_spotlight_name, { "pos", "color", "ambient_intens", "diffuse_intens", "specular_intens", "linear", "constant", "quadratic", "spot_dir", "inner_cutoff", "outer_cutoff" });
	}
}

// WARNING: Exception abuse
void ShaderProgram::set_uniform(const std::string& a_material_name, const MaterialMap& a_material) {
	auto diffuse_maps = a_material.get_diffuse_maps();
	auto specular_maps = a_material.get_specular_maps();
	auto emission_maps = a_material.get_emission_maps();
	try {
		m_uniforms.at(a_material_name + ".shininess") = a_material.get_shininess();

		for (const auto& diffuse_map : diffuse_maps)
			diffuse_map->activate();
		for (const auto& specular_map : specular_maps)
			specular_map->activate();
		for (const auto& emission_map : emission_maps)
			emission_map->activate();
	} catch (std::out_of_range) {
		std::vector<std::string> maps = {};
		for (size_t i = 0; i < diffuse_maps.size(); i++)
			maps.push_back(std::format("diffuse_maps[{}]", i));
		for (size_t i = 0; i < specular_maps.size(); i++)
			maps.push_back(std::format("specular_maps[{}]", i));
		for (size_t i = 0; i < emission_maps.size(); i++)
			maps.push_back(std::format("emission_maps[{}]", i));
		maps.push_back("shininess");

		push_uniform_struct(a_material_name, maps.begin(), maps.end());

		for (size_t i = 0; i < diffuse_maps.size(); i++)
			m_uniforms.at(std::format("{}.diffuse_maps[{}]", a_material_name, i)) = diffuse_maps[i]->get_texture_unit();
		for (size_t i = 0; i < specular_maps.size(); i++)
			m_uniforms.at(std::format("{}.specular_maps[{}]", a_material_name, i)) = specular_maps[i]->get_texture_unit();
		for (size_t i = 0; i < emission_maps.size(); i++)
			m_uniforms.at(std::format("{}.emission_maps[{}]", a_material_name, i)) = emission_maps[i]->get_texture_unit();
		m_uniforms.at(a_material_name + ".shininess") = a_material.get_shininess();
	}
}

void ShaderProgram::set_depth_testing(bool a_option) {
	m_depth_testing = a_option;
}

void ShaderProgram::set_stencil_testing(bool a_option) {
	m_stencil_testing = a_option;
}

void ShaderProgram::check_linking() {
	glGetProgramiv(m_shader_program, GL_LINK_STATUS, &m_success);
	if (!m_success) {
		glGetProgramInfoLog(m_shader_program, 1024, nullptr, m_infoLog);
		ERROR(std::format("Shader linking error.\nGLSL error message:\n{}", m_infoLog).data());
		throw Error_code::glsl_bad_linking;
	}
}

void ShaderProgram::use() {
	glUseProgram(m_shader_program);

	if (m_depth_testing) {
		glEnable(GL_DEPTH_TEST);
	} else {
		glDisable(GL_DEPTH_TEST);
	}

	if (m_stencil_testing) {
		glEnable(GL_STENCIL_TEST);
	} else {
		glDisable(GL_STENCIL_TEST);
	}
}

ShaderProgram::~ShaderProgram() {
	glDeleteProgram(m_shader_program);
}

bool ShaderProgram::get_depth_testing() const {
	return m_depth_testing;
}

bool ShaderProgram::get_stencil_testing() const {
	return m_stencil_testing;
}

unsigned int ShaderProgram::get_shader_program() const { 
	return m_shader_program;
}

void ShaderProgram::debug() const {
	for (const auto& [key, val] : m_uniforms) {
		std::cout << std::format("key: {}\n", key);
	}
}
