#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <map>
#include <string>
#include <vector>

#include "chill_engine/light.hpp"
#include "chill_engine/meshes.hpp"

#define INFO_LOG_SIZ 1024

namespace chill_engine {
enum class ShaderType {
	VERTEX,
	FRAGMENT,
	NONE,
};

enum class ShaderState {
	DEPTH_TEST,
	STENCIL_TEST,
	FACE_CULLING,
	POINT_SIZE,
};

class Uniform {
public:
	Uniform() = default;
	Uniform(const std::string& a_name, int a_location, GLuint a_program);

	auto get_name() const -> std::string;

	template<typename T>
	auto operator=(const T& val) -> Uniform&;

private:
	int m_uniform_location = -1;
	unsigned int m_shader_program = EMPTY_VBO;
	std::string m_name = "";
};

class ShaderSrc {
public:
	ShaderSrc() = default;
	ShaderSrc(ShaderType a_shader_type, const std::wstring& a_path);
	ShaderSrc(const ShaderSrc& a_shader_src);
	ShaderSrc(ShaderSrc&& a_shader_src) noexcept;
	~ShaderSrc();

	auto operator=(const ShaderSrc& a_shader_src) -> ShaderSrc&;
	auto operator=(ShaderSrc&& a_shader_src) noexcept -> ShaderSrc&;

	auto get_type() const -> ShaderType;
	auto get_path() const -> std::wstring;
	auto get_id() const -> GLuint;

private:
	ShaderType m_type = ShaderType::NONE;
	std::wstring m_path = L"";
	GLuint m_id = EMPTY_VBO;
};

class ShaderProgram {
public:
	ShaderProgram() = default;
	ShaderProgram(const ShaderSrc& a_vertex_shader, const ShaderSrc& a_fragment_shader);
	ShaderProgram(const ShaderProgram& a_shader_program);
	ShaderProgram(ShaderProgram&& a_shader_program) noexcept;
	~ShaderProgram();

	auto operator=(const ShaderProgram& a_shader_program) -> ShaderProgram&;
	auto operator=(ShaderProgram&& a_shader_program) noexcept -> ShaderProgram&;

	auto operator[](const std::string& a_uniform_var) -> Uniform&;
	void set_state(ShaderState a_state, bool a_option);
	auto set_uniform(const std::string& a_dirlight_name, const DirLight& a_light) -> void;
	auto set_uniform(const std::string& a_pointlight_name, const PointLight& a_light) -> void;
	auto set_uniform(const std::string& a_spotlight_name, const SpotLight& a_light) -> void;
	auto set_uniform(const std::string& a_material_name, const MaterialMap& a_material) -> void;
	auto set_binding_point(const std::string& a_uniform_block_name, int a_binding_point) -> void;
	auto use() -> void;

	auto get_id() const -> GLuint;
	auto get_vert_shader() const -> ShaderSrc;
	auto get_frag_shader() const -> ShaderSrc;
	auto is_state(ShaderState a_state) const -> bool;

	auto debug() const -> void;

private:
	auto push_uniform(const std::string& a_uniform_var) -> void;
	template<typename T>
	auto push_uniform_struct(const std::string& uniform_var, std::initializer_list<T> a_member_names) -> void;
	template<typename Container>
	auto push_uniform_struct(const std::string& a_uniform_var, const Container& a_mamber_list) -> void;

	GLuint m_id = EMPTY_VBO;
	ShaderSrc m_vertex_sh = {};
	ShaderSrc m_fragment_sh = {};
	std::map<std::string, Uniform> m_uniforms;
	std::map<ShaderState, bool> m_states{
		{ ShaderState::DEPTH_TEST,   true },
		{ ShaderState::STENCIL_TEST, false },
		{ ShaderState::FACE_CULLING, true },
		{ ShaderState::POINT_SIZE, true }
	};
}; 

// Template definitions
#include "shaders_templates.cpp"
}