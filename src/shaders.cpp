#include "shaders.hpp"

#include <ios>
#include <format>
#include <fstream>
#include <iostream>
#include <algorithm>

#include "assert.hpp"

Uniform::Uniform(std::string a_name, int a_location, unsigned int a_program)
	:name{ a_name }, uniform_location{ a_location }, shader_program{ a_program }
{  }

std::string Uniform::get_name() const {
	return this->name;
}

Shader_src::Shader_src(ShaderType a_shader_type, std::string a_filename) 
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

void Shader_src::load_code(std::string filename) {
	using std::ios;
	std::ifstream file_vertex_shader{ "../" + filename, ios::in | ios::binary };

	if (!file_vertex_shader.is_open()) {
		switch (shader_type) {
			case ShaderType::VERTEX: {
				ERROR(std::format("Vertex shader source file {} couldn't be loaded.", filename).data());
			}
			case ShaderType::FRAGMENT: {
				ERROR(std::format("Fragment shader source file {} couldn't be loaded.", filename).data());
			}
			default: {
				ERROR(std::format("Unhandled shader type with source file {} couldn't be loaded.", filename).data());
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
	:m_shader_program{ glCreateProgram() }, m_model_name{ "" }, m_view_name{ "" }, m_projection_name{ "" }, m_normal_name{ "" }, 
	 m_material_name{ "" }, m_light_name{ "" }, m_name{ "" }, m_depth_testing{ true }
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

Shader_program::Shader_program(Shader_program& a_shader_prog)
	:m_shader_program{ a_shader_prog.m_shader_program }, m_model_name{ a_shader_prog.m_model_name }, m_view_name{ a_shader_prog.m_view_name },
	 m_projection_name{ a_shader_prog.m_projection_name }, m_normal_name{ a_shader_prog.m_normal_name }, m_material_name{ a_shader_prog.m_material_name },
	 m_light_name{ a_shader_prog.m_light_name }, m_name{ a_shader_prog.m_name }, m_depth_testing{ a_shader_prog.m_depth_testing }
{ }

Uniform& Shader_program::operator[](std::string uniform_var) {
	auto iter = std::find_if(this->m_uniforms.begin(), this->m_uniforms.end(), [=](const Uniform& uni){ return uni.get_name() == uniform_var; });
	if (iter == this->m_uniforms.end()) {
		ERROR(std::format("Uniform \"{}\" couldn't be found.", uniform_var).data());
		throw Error_code::bad_match;
	}
	return *iter;
}

Uniform& Shader_program::push_uniform(std::string uniform_var) {
	int location = glGetUniformLocation(m_shader_program, uniform_var.c_str());
	if (location == -1) {
		ERROR(std::format("Wrong uniform attribute name \"{}\" for shader {}", uniform_var, m_name).data());
		throw Error_code::bad_match;
	}

	m_uniforms.emplace_back(uniform_var, location, this->m_shader_program);
	return m_uniforms.back();
}

Uniform& Shader_program::push_model(std::string uniform_var) {
	m_model_name = uniform_var;
	return push_uniform(uniform_var);
}

Uniform& Shader_program::push_view(std::string uniform_var) {
	m_view_name = uniform_var;
	return push_uniform(uniform_var);
}

Uniform& Shader_program::push_projection(std::string uniform_var) {
	m_projection_name = uniform_var;
	return push_uniform(uniform_var);
}

Uniform& Shader_program::push_normal(std::string uniform_var) {
	m_normal_name = uniform_var;
	return push_uniform(uniform_var);
}

// material is struct so we can't easily get it's location without specifying it's fields because
// struct is a namespace for uniforms.
void Shader_program::push_material(std::string uniform_var) {
	m_material_name = uniform_var;
	push_uniform(uniform_var + ".diffuse_map");  // Sampler2D
	push_uniform(uniform_var + ".specular_map"); // Sampler2D
	push_uniform(uniform_var + ".emission_map"); // Sampler2D
	push_uniform(uniform_var + ".shininess");
	(*this)["material.diffuse_map"]  = DIFFUSE_MAP_ID;
	(*this)["material.specular_map"] = SPECULAR_MAP_ID;
	(*this)["material.emission_map"] = EMISSION_MAP_ID;
}

void Shader_program::push_light(std::string uniform_var) {
	m_light_name = uniform_var;
	push_uniform(uniform_var + ".pos_dir");
	push_uniform(uniform_var + ".color");
	push_uniform(uniform_var + ".ambient_intens");
	push_uniform(uniform_var + ".diffuse_intens");
	push_uniform(uniform_var + ".specular_intens");
}

void Shader_program::push_point_light(std::string uniform_var) {
	push_light(uniform_var);
	push_uniform(uniform_var + ".linear");
	push_uniform(uniform_var + ".constant");
	push_uniform(uniform_var + ".quadratic");
}

void Shader_program::push_spotlight(std::string uniform_var) {
	push_point_light(uniform_var);
	push_uniform(uniform_var + ".cutoff");
	push_uniform(uniform_var + ".spot_dir");
}

void Shader_program::set_name(std::string a_name) {
	m_name = a_name;
}

void Shader_program::set_light(const Light& a_light) {
	(*this)[m_light_name + ".color"] = a_light.get_color();
	(*this)[m_light_name + ".pos_dir"] = a_light.get_pos_dir();
	(*this)[m_light_name + ".ambient_intens"] = a_light.get_ambient();
	(*this)[m_light_name + ".diffuse_intens"] = a_light.get_diffuse();
	(*this)[m_light_name + ".specular_intens"] = a_light.get_specular();
}

void Shader_program::set_spotlight(const SpotLight& a_light) {
	set_point_light(a_light);
	(*this)[m_light_name + ".cutoff"] = a_light.get_cutoff();
	(*this)[m_light_name + ".spot_dir"] = a_light.get_spot_dir();
}

void Shader_program::set_point_light(const PointLight& a_light) {
	set_light(a_light);
	(*this)[m_light_name + ".linear"] = a_light.get_linear();
	(*this)[m_light_name + ".constant"] = a_light.get_constant();
	(*this)[m_light_name + ".quadratic"] = a_light.get_quadratic();
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

bool Shader_program::get_depth_testing() const {
	return m_depth_testing;
}

std::string Shader_program::get_model_name() const {
	return m_model_name;
}

std::string Shader_program::get_view_name() const {
	return m_view_name;
}

std::string Shader_program::get_projection_name() const {
	return m_projection_name;
}

std::string Shader_program::get_normal_name() const {
	return m_normal_name;
}

std::string Shader_program::get_material_name() const {
	return m_material_name;
}

std::string Shader_program::get_light_name() const {
	return m_light_name;
}

unsigned int Shader_program::get_shader_program() const { 
	return m_shader_program;
}
