#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "glm/fwd.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <string>
#include <vector>
#include <type_traits>

#include "assert.hpp"
#include "light.hpp"

#define INFO_LOG_SIZ 1024
#define DIFFUSE_MAP_ID  0
#define SPECULAR_MAP_ID 1
#define EMISSION_MAP_ID 2

enum class ShaderType {
	VERTEX,
	FRAGMENT
};

class Uniform {
public:
	Uniform(std::string a_name, int a_location, unsigned int a_program);

	auto get_name() const -> std::string;

	template<typename T>
	auto operator=(T val) -> Uniform&;

private:
	unsigned int shader_program;
	int uniform_location;
	std::string name;
};

template<typename T>
Uniform& Uniform::operator=(T val) {
	glUseProgram(this->shader_program);
	if constexpr (std::is_same_v<T, float>) {
		glUniform1f(this->uniform_location, val);
	} else if constexpr (std::is_same_v<T, int>) {
		glUniform1i(this->uniform_location, val);
	} else if constexpr (std::is_same_v<T, glm::vec1>) {
		glUniform1f(this->uniform_location, val[0]);
	} else if constexpr (std::is_same_v<T, glm::vec2>) {
		glUniform2f(this->uniform_location, val[0], val[1]);
	} else if constexpr (std::is_same_v<T, glm::vec3>) {
		glUniform3f(this->uniform_location, val[0], val[1], val[2]);
	} else if constexpr (std::is_same_v<T, glm::vec4>) {
		glUniform4f(this->uniform_location, val[0], val[1], val[2], val[3]);
	} else if constexpr (std::is_same_v<T, glm::mat2>) {
		glUniformMatrix2fv(this->uniform_location, 1, GL_FALSE, glm::value_ptr(val));
	} else if constexpr (std::is_same_v<T, glm::mat3>) {
		glUniformMatrix3fv(this->uniform_location, 1, GL_FALSE, glm::value_ptr(val));
	} else if constexpr (std::is_same_v<T, glm::mat4>) {
		glUniformMatrix4fv(this->uniform_location, 1, GL_FALSE, glm::value_ptr(val));
	} else if constexpr (std::is_same_v<T, std::vector<float>>) {
		switch(val.size()) {
			case 1: glUniform1f(this->uniform_location, val[0]); break;
			case 2: glUniform2f(this->uniform_location, val[0], val[1]); break;
			case 3: glUniform3f(this->uniform_location, val[0], val[1], val[2]); break;
			case 4: glUniform4f(this->uniform_location, val[0], val[1], val[2], val[3]); break;
		}
	} else if constexpr (std::is_same_v<T, std::vector<int>>) {
		switch(val.size()) {
			case 1: glUniform1i(this->uniform_location, val[0]); break;
			case 2: glUniform2i(this->uniform_location, val[0], val[1]); break;
			case 3: glUniform3i(this->uniform_location, val[0], val[1], val[2]); break;
			case 4: glUniform4i(this->uniform_location, val[0], val[1], val[2], val[3]); break;
		}
	} else {
		ERROR("Bad uniform type.");
		throw Error_code::bad_type;
	}

	return *this;
}

struct Shader_src {
	int success;
	char infoLog[INFO_LOG_SIZ];
	char* code = nullptr;
	ShaderType shader_type;
	std::string filename;
	std::string shader_name;
	unsigned int shader_obj;

	Shader_src(ShaderType a_shader_type, std::string a_filename);

	auto load_code(std::string filename) -> void;
	auto compile_shader(char** code) -> void;
	auto check_compilation() -> void;
	~Shader_src();
};

class Shader_program {
public:
	Shader_program(std::initializer_list<Shader_src> a_shaders);
	Shader_program(Shader_program& a_shader_prog);
	~Shader_program();

	auto operator[](std::string uniform_var) -> Uniform&;

	auto push_uniform(std::string uniform_var) -> Uniform&;
	auto push_transform(std::string uniform_var) -> Uniform&;
	auto push_model(std::string uniform_var) -> Uniform&;
	auto push_view(std::string uniform_var) -> Uniform&;
	auto push_projection(std::string uniform_var) -> Uniform&;
	auto push_normal(std::string uniform_var) -> Uniform&;
	auto push_material(std::string uniform_var) -> void;
	auto push_light(std::string uniform_var) -> void;
	auto push_point_light(std::string uniform_var) -> void;
	
	auto set_name(std::string a_name) -> void;
	auto set_light(const Light& a_light) -> void;
	auto set_point_light(const PointLight& a_light) -> void;
	auto set_depth_testing(bool option) -> void;
	auto check_linking() -> void;
	auto use() -> void;

	auto get_depth_testing() const -> bool;
	auto get_model_name() const -> std::string;
	auto get_view_name() const -> std::string;
	auto get_projection_name() const -> std::string;
	auto get_normal_name() const -> std::string;
	auto get_material_name() const -> std::string;
	auto get_light_name() const -> std::string;
	auto get_shader_program() const -> unsigned int;

private:
	int m_success;
	char m_infoLog[INFO_LOG_SIZ];
	bool m_depth_testing;
	unsigned int m_shader_program;
	std::string m_name;
	std::string m_model_name;
	std::string m_view_name;
	std::string m_projection_name;
	std::string m_normal_name;
	std::string m_material_name;
	std::string m_light_name;
	std::vector<Uniform> m_uniforms;
};
