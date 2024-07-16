#pragma once

#include "glad/glad.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <map>
#include <string>
#include <vector>
#include <type_traits>

#include "assert.hpp"
#include "light.hpp"
#include "meshes.hpp"

#define INFO_LOG_SIZ 1024

enum class ShaderType {
	VERTEX,
	FRAGMENT
};

class Uniform {
public:
	Uniform(std::string a_name, int a_location, unsigned int a_program);
	Uniform();

	auto get_name() const -> std::string;

	template<typename T>
	auto operator=(T val) -> Uniform&;

private:
	int m_uniform_location;
	unsigned int m_shader_program;
	std::string m_name;
};

template<typename T>
Uniform& Uniform::operator=(T val) {
	glUseProgram(m_shader_program);
	if constexpr (std::is_same_v<T, float>) {
		glUniform1f(m_uniform_location, val);
	} else if constexpr (std::is_same_v<T, int>) {
		glUniform1i(m_uniform_location, val);
	} else if constexpr (std::is_same_v<T, glm::vec1>) {
		glUniform1f(m_uniform_location, val[0]);
	} else if constexpr (std::is_same_v<T, glm::vec2>) {
		glUniform2f(m_uniform_location, val[0], val[1]);
	} else if constexpr (std::is_same_v<T, glm::vec3>) {
		glUniform3f(m_uniform_location, val[0], val[1], val[2]);
	} else if constexpr (std::is_same_v<T, glm::vec4>) {
		glUniform4f(m_uniform_location, val[0], val[1], val[2], val[3]);
	} else if constexpr (std::is_same_v<T, glm::mat2>) {
		glUniformMatrix2fv(m_uniform_location, 1, GL_FALSE, glm::value_ptr(val));
	} else if constexpr (std::is_same_v<T, glm::mat3>) {
		glUniformMatrix3fv(m_uniform_location, 1, GL_FALSE, glm::value_ptr(val));
	} else if constexpr (std::is_same_v<T, glm::mat4>) {
		glUniformMatrix4fv(m_uniform_location, 1, GL_FALSE, glm::value_ptr(val));
	} else if constexpr (std::is_same_v<T, std::vector<float>>) {
		switch(val.size()) {
			case 1: glUniform1f(m_uniform_location, val[0]); break;
			case 2: glUniform2f(m_uniform_location, val[0], val[1]); break;
			case 3: glUniform3f(m_uniform_location, val[0], val[1], val[2]); break;
			case 4: glUniform4f(m_uniform_location, val[0], val[1], val[2], val[3]); break;
		}
	} else if constexpr (std::is_same_v<T, std::vector<int>>) {
		switch(val.size()) {
			case 1: glUniform1i(m_uniform_location, val[0]); break;
			case 2: glUniform2i(m_uniform_location, val[0], val[1]); break;
			case 3: glUniform3i(m_uniform_location, val[0], val[1], val[2]); break;
			case 4: glUniform4i(m_uniform_location, val[0], val[1], val[2], val[3]); break;
		}
	} else {
		ERROR("Bad uniform type.");
		throw Error_code::bad_type;
	}

	return *this;
}

struct ShaderSrc {
	int success;
	char infoLog[INFO_LOG_SIZ];
	char* code = nullptr;
	ShaderType shader_type;
	std::string filename;
	std::string shader_name;
	unsigned int shader_obj;

	ShaderSrc(ShaderType a_shader_type, const std::string& a_filename);

	auto load_code(const std::string& filename) -> void;
	auto compile_shader(char** code) -> void;
	auto check_compilation() -> void;
	~ShaderSrc();
};

class ShaderProgram {
public:
	ShaderProgram(std::initializer_list<ShaderSrc> a_shaders);
	ShaderProgram(const ShaderProgram& a_shader_prog);
	~ShaderProgram();

	auto operator[](const std::string& a_uniform_var) -> Uniform&;
	auto set_name(const std::string& a_name) -> void;
	auto set_uniform(const std::string& a_dirlight_name, const DirLight& a_light) -> void;
	auto set_uniform(const std::string& a_pointlight_name, const PointLight& a_light) -> void;
	auto set_uniform(const std::string& a_spotlight_name, const SpotLight& a_light) -> void;
	auto set_uniform(const std::string& a_material_name, const MaterialMap& a_material) -> void;
	auto set_depth_testing(bool option) -> void;
	auto use() -> void;

	auto get_depth_testing() const -> bool;
	auto get_shader_program() const -> unsigned int;

	auto debug() const -> void;

private:
	auto push_uniform(const std::string& a_uniform_var) -> void;
	auto push_uniform_struct(const std::string& uniform_var, std::initializer_list<std::string> a_members) -> void;
	template<typename It>
	auto push_uniform_struct(const std::string& a_uniform_var, It a_member_name_first, It a_member_name_last) -> void;
	auto check_linking() -> void;

	int m_success;
	char m_infoLog[INFO_LOG_SIZ];
	bool m_depth_testing;
	unsigned int m_shader_program;
	std::string m_name;
	std::map<std::string, Uniform> m_uniforms;
};
