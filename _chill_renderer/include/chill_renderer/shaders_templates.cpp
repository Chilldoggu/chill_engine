#include <type_traits>

#include "shaders.hpp"

template<typename T>
Uniform& Uniform::operator=(const T& val) {
	glUseProgram(m_shader_program);
	if constexpr (std::is_same_v<T, float> || std::is_same_v<T, double>) {
		glUniform1f(m_uniform_location, static_cast<float>(val));
	}
	else if constexpr (std::is_same_v<T, int> || std::is_same_v<T, bool>) {
		glUniform1i(m_uniform_location, val);
	}
	else if constexpr (std::is_same_v<T, glm::vec1>) {
		glUniform1f(m_uniform_location, val[0]);
	}
	else if constexpr (std::is_same_v<T, glm::vec2>) {
		glUniform2f(m_uniform_location, val[0], val[1]);
	}
	else if constexpr (std::is_same_v<T, glm::vec3>) {
		glUniform3f(m_uniform_location, val[0], val[1], val[2]);
	}
	else if constexpr (std::is_same_v<T, glm::vec4>) {
		glUniform4f(m_uniform_location, val[0], val[1], val[2], val[3]);
	}
	else if constexpr (std::is_same_v<T, glm::mat2>) {
		glUniformMatrix2fv(m_uniform_location, 1, GL_FALSE, glm::value_ptr(val));
	}
	else if constexpr (std::is_same_v<T, glm::mat3>) {
		glUniformMatrix3fv(m_uniform_location, 1, GL_FALSE, glm::value_ptr(val));
	}
	else if constexpr (std::is_same_v<T, glm::mat4>) {
		glUniformMatrix4fv(m_uniform_location, 1, GL_FALSE, glm::value_ptr(val));
	}
	else if constexpr (std::is_same_v<T, std::vector<float>>) {
		switch (val.size()) {
		case 1: glUniform1f(m_uniform_location, val[0]); break;
		case 2: glUniform2f(m_uniform_location, val[0], val[1]); break;
		case 3: glUniform3f(m_uniform_location, val[0], val[1], val[2]); break;
		case 4: glUniform4f(m_uniform_location, val[0], val[1], val[2], val[3]); break;
		}
	}
	else if constexpr (std::is_same_v<T, std::vector<int>>) {
		switch (val.size()) {
		case 1: glUniform1i(m_uniform_location, val[0]); break;
		case 2: glUniform2i(m_uniform_location, val[0], val[1]); break;
		case 3: glUniform3i(m_uniform_location, val[0], val[1], val[2]); break;
		case 4: glUniform4i(m_uniform_location, val[0], val[1], val[2], val[3]); break;
		}
	}
	else {
		ERROR("[UNIFORM::OPERATOR=] Bad uniform type.", Error_action::throwing);
	}

	return *this;
}
