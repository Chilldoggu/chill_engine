#include <initializer_list>
#include <ios>
#include <format>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <stdexcept>

#include "shaders.hpp"
#include "assert.hpp"
#include "meshes.hpp"
#include "file_manager.hpp" // wstos

extern std::filesystem::path get_proj_path();

Uniform::Uniform(std::string a_name, int a_location, unsigned int a_program)
	:m_name{ a_name }, m_uniform_location{ a_location }, m_shader_program{ a_program }
{
	if (m_uniform_location == -1) {
		ERROR(std::format("[UNIFORM::UNIFORM] Wrong uniform attribute name \"{}\"", m_name), Error_action::throwing);
	}
}

std::string Uniform::get_name() const {
	return m_name;
}

ShaderSrc::ShaderSrc(ShaderType a_shader_type, const std::wstring& a_path) 
	:m_type{ a_shader_type }, m_path{ (get_proj_path() / a_path).wstring() }
{
	switch(m_type) {
		case ShaderType::VERTEX:
			m_obj = glCreateShader(GL_VERTEX_SHADER);
			break;
		case ShaderType::FRAGMENT:
			m_obj = glCreateShader(GL_FRAGMENT_SHADER);
			break;
		default:
			ERROR("[SHADERSRC::SHADERSRC] Shader type not compatible.", Error_action::throwing);
	}
 
	m_code = std::make_unique<char*>(load_code());
	compile_shader();
}

char* ShaderSrc::load_code() {
	using std::ios;
	std::ifstream shader_file{ m_path, ios::in | ios::binary };

	if (!shader_file.is_open()) {
		switch (m_type) {
		case ShaderType::VERTEX:
			ERROR(std::format("[SHADERSRC::LOAD_CODE] Vertex shader source file {} couldn't be loaded.", wstos(m_path)), Error_action::throwing); 
			break;
		case ShaderType::FRAGMENT:
			ERROR(std::format("[SHADERSRC::LOAD_CODE] Fragment shader source file {} couldn't be loaded.", wstos(m_path)), Error_action::throwing); 
			break;
		default: 
			ERROR(std::format("[SHADERSRC::LOAD_CODE] Unhandled shader type with source file {} couldn't be loaded.", wstos(m_path)), Error_action::throwing); 
			break;
		}
	}

	shader_file.seekg(0, shader_file.end);
	int len = shader_file.tellg();
	shader_file.seekg(0, shader_file.beg);

	char* code = new char[len];
	shader_file.read(code, len);
	code[len-1] = '\0';
	return code;
}

void ShaderSrc::compile_shader() {
	auto tmp = m_code.get();
	glShaderSource(m_obj, 1, tmp, nullptr);
	glCompileShader(m_obj);
	check_compilation();
}

void ShaderSrc::check_compilation() {
	glGetShaderiv(m_obj, GL_COMPILE_STATUS, &m_compilation_success);

	if (!m_compilation_success) {
		glGetShaderInfoLog(m_obj, 1024, nullptr, m_infoLog);
		switch (m_type) {
		case ShaderType::VERTEX:
			ERROR(std::format("[SHADERSRC::CHECK_COMPILATION] {} shader \"VERTEX\" can't compile.\nGLSL error message:\n{}", wstos(m_path), m_infoLog), Error_action::throwing); 
			break;
		case ShaderType::FRAGMENT: 
			ERROR(std::format("[SHADERSRC::CHECK_COMPILATION] {} shader \"FRAGMENT\" can't compile.\nGLSL error message:\n{}", wstos(m_path), m_infoLog), Error_action::throwing); 
			break;
		default:
			ERROR(std::format("[SHADERSRC::CHECK_COMPILATION] {} shader \"UNKNOWN\" can't compile.\nGLSL error message:\n{}", wstos(m_path), m_infoLog), Error_action::throwing); 
			break;
		}
	}
}

ShaderSrc::~ShaderSrc() {
	glDeleteShader(m_obj);
	m_obj = 0;
}

ShaderProgram::ShaderProgram(std::initializer_list<ShaderSrc> a_shaders) {
	if (!std::find_if(a_shaders.begin(), a_shaders.end(), [](const ShaderSrc& sh){ return sh.m_type == ShaderType::VERTEX; })) {
		ERROR("[SHADERPROGRAM::SHADERPROGRAM] Shader initializer list lacks vertex shader.", Error_action::throwing);
	}
	if (!std::find_if(a_shaders.begin(), a_shaders.end(), [](const ShaderSrc& sh){ return sh.m_type == ShaderType::FRAGMENT; })) {
		ERROR("[SHADERPROGRAM::SHADERPROGRAM] Shader initializer list lacks fragment shader.", Error_action::throwing);
	}

	for (auto& shader : a_shaders) {
		glAttachShader(m_shader_program, shader.m_obj);
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
			diffuse_map.activate();
		for (const auto& specular_map : specular_maps)
			specular_map.activate();
		for (const auto& emission_map : emission_maps)
			emission_map.activate();
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
			m_uniforms.at(std::format("{}.diffuse_maps[{}]", a_material_name, i)) = diffuse_maps[i].get_unit_id();
		for (size_t i = 0; i < specular_maps.size(); i++)
			m_uniforms.at(std::format("{}.specular_maps[{}]", a_material_name, i)) = specular_maps[i].get_unit_id();
		for (size_t i = 0; i < emission_maps.size(); i++)
			m_uniforms.at(std::format("{}.emission_maps[{}]", a_material_name, i)) = emission_maps[i].get_unit_id();
		m_uniforms.at(a_material_name + ".shininess") = a_material.get_shininess();
	}
}

void ShaderProgram::set_face_culling(bool a_option) {
	m_states.at("FACE_CULLING") = a_option;
}

void ShaderProgram::set_depth_testing(bool a_option) {
	m_states.at("DEPTH_TEST") = a_option;
}

void ShaderProgram::set_stencil_testing(bool a_option) {
	m_states.at("STENCIL_TEST") = a_option;
}

void ShaderProgram::check_linking() {
	glGetProgramiv(m_shader_program, GL_LINK_STATUS, &m_success);
	if (!m_success) {
		glGetProgramInfoLog(m_shader_program, 1024, nullptr, m_infoLog);
		ERROR(std::format("[SHADERPROGRAM::CHECK_LINKING] Shader linking error.\nGLSL error message:\n{}", m_infoLog), Error_action::throwing);
	}
}

void ShaderProgram::use() {
	glUseProgram(m_shader_program);

	if (m_states.at("FACE_CULLING")) {
		glEnable(GL_CULL_FACE);
	} else {
		glDisable(GL_CULL_FACE);
	}

	if (m_states.at("DEPTH_TEST")) {
		glEnable(GL_DEPTH_TEST);
	} else {
		glDisable(GL_DEPTH_TEST);
	}

	if (m_states.at("STENCIL_TEST")) {
		glEnable(GL_STENCIL_TEST);
	} else {
		glDisable(GL_STENCIL_TEST);
	}
}

ShaderProgram::~ShaderProgram() {
	glDeleteProgram(m_shader_program);
}

bool ShaderProgram::get_state(std::string a_state) const {
	return m_states.at(a_state);
}

unsigned int ShaderProgram::get_shader_program() const { 
	return m_shader_program;
}

void ShaderProgram::debug() const {
	for (const auto& [key, val] : m_uniforms) {
		std::cout << std::format("key: {}\n", key);
	}
}
