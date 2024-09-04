#include <initializer_list>
#include <ios>
#include <format>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <stdexcept>

#include "chill_engine/shaders.hpp"
#include "chill_engine/assert.hpp"
#include "chill_engine/meshes.hpp"
#include "chill_engine/file_manager.hpp" // wstos
#include "chill_engine/application.hpp"

namespace chill_engine {
namespace fs = std::filesystem;

extern fs::path guess_path(const std::wstring& a_path);

Uniform::Uniform(const std::string& a_name, int a_location, GLuint a_program)
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
	:m_type{ a_shader_type }
{
	fs::path p = guess_path(a_path);
	if (p == fs::path())
		ERROR(std::format("[SHADERSRC::SHADERSRC] Bad shader path: {}", wstos(a_path)), Error_action::throwing);

	m_path = p.wstring();

	switch (m_type) {
	case ShaderType::VERTEX:   m_id = glCreateShader(GL_VERTEX_SHADER); break;
	case ShaderType::FRAGMENT: m_id = glCreateShader(GL_FRAGMENT_SHADER); break;
	default:
		ERROR("[SHADERSRC::SHADERSRC] Shader type not compatible.", Error_action::throwing);
	}

	// =========
	// LOAD CODE
	// =========
	using std::ios;
	std::ifstream shader_file{ m_path, ios::in | ios::binary };

	if (!shader_file.is_open())
		ERROR(std::format("[SHADERSRC::SHADERSRC] Shader source file {} couldn't be loaded.", wstos(m_path)), Error_action::throwing);

	shader_file.seekg(0, shader_file.end);
	int len = shader_file.tellg();
	shader_file.seekg(0, shader_file.beg);

	char* code = new char[len];
	shader_file.read(code, len);
	code[len - 1] = '\0';

	// ============
	// COMPILE CODE
	// ============
	glShaderSource(m_id, 1, &code, nullptr);
	glCompileShader(m_id);
	delete[] code;

	// =================
	// CHECK COMPILATION
	// =================
	int success;
	glGetShaderiv(m_id, GL_COMPILE_STATUS, &success);

	if (!success) {
		char infoLog[INFO_LOG_SIZ] = {};
		glGetShaderInfoLog(m_id, 1024, nullptr, infoLog);
		ERROR(std::format("[SHADERSRC::SHADERSRC] shader {} can't compile. GLSL error message:\n{}", wstos(m_path), infoLog), Error_action::throwing);
	}
}

ShaderSrc::ShaderSrc(const ShaderSrc& a_shader_src) {
	Application::get_instance().get_rmanager().inc_ref_count(ResourceType::SHADER_SRCS, a_shader_src.m_id);

	m_type = a_shader_src.m_type;
	m_path = a_shader_src.m_path;
	m_id = a_shader_src.m_id;
}

ShaderSrc::ShaderSrc(ShaderSrc&& a_shader_src) {
	m_type = a_shader_src.m_type;
	m_path = a_shader_src.m_path;
	m_id = a_shader_src.m_id;

	m_type = ShaderType::NONE;
	m_path = L"";
	m_id = EMPTY_VBO;
}

ShaderSrc& ShaderSrc::operator=(const ShaderSrc& a_shader_src) {
	Application::get_instance().get_rmanager().inc_ref_count(ResourceType::SHADER_SRCS, a_shader_src.m_id);

	m_type = a_shader_src.m_type;
	m_path = a_shader_src.m_path;
	m_id = a_shader_src.m_id;

	return *this;
}

ShaderSrc& ShaderSrc::operator=(ShaderSrc&& a_shader_src) {
	m_type = a_shader_src.m_type;
	m_path = a_shader_src.m_path;
	m_id = a_shader_src.m_id;

	m_type = ShaderType::NONE;
	m_path = L"";
	m_id = EMPTY_VBO;

	return *this;
}

ShaderSrc::~ShaderSrc() {
	if (m_id != EMPTY_VBO) {
		Application::get_instance().get_rmanager().dec_ref_count(ResourceType::SHADER_SRCS, m_id);
		if (!Application::get_instance().get_rmanager().chk_ref_count(ResourceType::SHADER_SRCS, m_id)) {
			glDeleteShader(m_id);
		}
	}
}

ShaderProgram::ShaderProgram(std::string& a_name, const ShaderSrc& a_vertex_shader, const ShaderSrc& a_fragment_shader) 
	:m_name{ a_name }, m_vertex_sh{ a_vertex_shader }, m_fragment_sh{ a_fragment_shader } {
	m_id = glCreateProgram();

	glAttachShader(m_id, a_vertex_shader.m_id);
	glAttachShader(m_id, a_fragment_shader.m_id);

	glLinkProgram(m_id);

	// =============
	// CHECK LINKING
	// =============
	int success;
	glGetProgramiv(m_id, GL_LINK_STATUS, &success);
	if (!success) {
		char infoLog[INFO_LOG_SIZ] = {};
		glGetProgramInfoLog(m_id, 1024, nullptr, infoLog);
		ERROR(std::format("[SHADERPROGRAM::CHECK_LINKING] Shader linking error. GLSL error message:\n{}", infoLog), Error_action::throwing);
	}
}

ShaderProgram::ShaderProgram(const ShaderProgram& a_shader_program) {
	Application::get_instance().get_rmanager().inc_ref_count(ResourceType::SHADER_PROGRAMS, a_shader_program.m_id);

	m_id = a_shader_program.m_id;
	m_name = a_shader_program.m_name;
	m_vertex_sh = a_shader_program.m_vertex_sh;
	m_fragment_sh = a_shader_program.m_fragment_sh;
	m_uniforms = a_shader_program.m_uniforms;
	m_states = a_shader_program.m_states;
}

ShaderProgram::ShaderProgram(ShaderProgram&& a_shader_program) noexcept {
	m_id = a_shader_program.m_id;
	m_name = a_shader_program.m_name;
	m_vertex_sh = a_shader_program.m_vertex_sh;
	m_fragment_sh = a_shader_program.m_fragment_sh;
	m_uniforms = a_shader_program.m_uniforms;
	m_states = a_shader_program.m_states;

	a_shader_program.m_id = EMPTY_VBO;
	a_shader_program.m_name = "";
	a_shader_program.m_vertex_sh = ShaderSrc();
	a_shader_program.m_fragment_sh = ShaderSrc();
	a_shader_program.m_uniforms.clear();
	a_shader_program.m_states.clear();
}

ShaderProgram& ShaderProgram::operator=(const ShaderProgram& a_shader_program) {
	Application::get_instance().get_rmanager().inc_ref_count(ResourceType::SHADER_PROGRAMS, a_shader_program.m_id);

	m_id = a_shader_program.m_id;
	m_name = a_shader_program.m_name;
	m_vertex_sh = a_shader_program.m_vertex_sh;
	m_fragment_sh = a_shader_program.m_fragment_sh;
	m_uniforms = a_shader_program.m_uniforms;
	m_states = a_shader_program.m_states;

	return *this;
}

ShaderProgram& ShaderProgram::operator=(ShaderProgram&& a_shader_program) noexcept {
	m_id = a_shader_program.m_id;
	m_name = a_shader_program.m_name;
	m_vertex_sh = a_shader_program.m_vertex_sh;
	m_fragment_sh = a_shader_program.m_fragment_sh;
	m_uniforms = a_shader_program.m_uniforms;
	m_states = a_shader_program.m_states;

	a_shader_program.m_id = EMPTY_VBO;
	a_shader_program.m_name = "";
	a_shader_program.m_vertex_sh = ShaderSrc();
	a_shader_program.m_fragment_sh = ShaderSrc();
	a_shader_program.m_uniforms.clear();
	a_shader_program.m_states.clear();

	return *this;
}

ShaderProgram::~ShaderProgram() {
	if (this->get_id() != EMPTY_VBO) {
		Application::get_instance().get_rmanager().dec_ref_count(ResourceType::SHADER_PROGRAMS, m_id);
		if (!Application::get_instance().get_rmanager().chk_ref_count(ResourceType::SHADER_PROGRAMS, m_id)) {
			glDeleteProgram(m_id);
		}
	}
}

Uniform& ShaderProgram::operator[](const std::string& uniform_var) {
	try {
		return m_uniforms.at(uniform_var);
	}
	catch (std::out_of_range) {
		push_uniform(uniform_var);
		return m_uniforms[uniform_var];
	}
}

void ShaderProgram::use() {
	glUseProgram(m_id);

	if (m_states.at("FACE_CULLING")) {
		glEnable(GL_CULL_FACE);
	}
	else {
		glDisable(GL_CULL_FACE);
	}

	if (m_states.at("DEPTH_TEST")) {
		glEnable(GL_DEPTH_TEST);
	}
	else {
		glDisable(GL_DEPTH_TEST);
	}

	if (m_states.at("STENCIL_TEST")) {
		glEnable(GL_STENCIL_TEST);
	}
	else {
		glDisable(GL_STENCIL_TEST);
	}
}

void ShaderProgram::set_name(const std::string& a_name) {
	m_name = a_name;
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

void ShaderProgram::push_uniform_struct(const std::string& a_uniform_var, const std::initializer_list<std::string>& a_member_names) {
	for (const auto& member_name : a_member_names) {
		std::string full_name = a_uniform_var + "." + member_name;
		m_uniforms[full_name] = Uniform(full_name, glGetUniformLocation(m_id, full_name.c_str()), m_id);
	}
}

// TODO: Maybe variadic template?
template<typename It>
void ShaderProgram::push_uniform_struct(const std::string& a_uniform_var, const It& a_member_name_first, const It& a_member_name_last) {
	for (auto member_name = a_member_name_first; member_name != a_member_name_last; member_name++) {
		std::string full_name = a_uniform_var + "." + *member_name;
		m_uniforms[full_name] = Uniform(full_name, glGetUniformLocation(m_id, full_name.c_str()), m_id);
	}
}

void ShaderProgram::push_uniform(const std::string& uniform_var) {
	m_uniforms[uniform_var] = Uniform(uniform_var, glGetUniformLocation(m_id, uniform_var.c_str()), m_id);
}

// WARNING: Exception abuse
void ShaderProgram::set_uniform(const std::string& a_dirlight_name, const DirLight& a_light) {
	try {
		m_uniforms.at(a_dirlight_name + ".color") = a_light.get_color();
		m_uniforms.at(a_dirlight_name + ".dir") = a_light.get_dir();
		m_uniforms.at(a_dirlight_name + ".ambient_intens") = a_light.get_ambient();
		m_uniforms.at(a_dirlight_name + ".diffuse_intens") = a_light.get_diffuse();
		m_uniforms.at(a_dirlight_name + ".specular_intens") = a_light.get_specular();
	}
	catch (std::out_of_range) {
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
	}
	catch (std::out_of_range) {
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
	}
	catch (std::out_of_range) {
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
	}
	catch (std::out_of_range) {
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

bool ShaderProgram::get_state(std::string a_state) const {
	return m_states.at(a_state);
}

GLuint ShaderProgram::get_id() const {
	return m_id;
}

std::string ShaderProgram::get_name() const {
	return m_name;
}

void ShaderProgram::debug() const {
	for (const auto& [key, val] : m_uniforms) {
		std::cout << std::format("key: {}\n", key);
	}
} 
}