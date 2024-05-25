#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <vector>

#define EMPTY_VBO 0
#define ATTRIB_POS_LOCATION    0
#define ATTRIB_COLOR_LOCATION  1
#define ATTRIB_TEX_LOCATION    2
#define ATTRIB_NORMAL_LOCATION 3

enum class BufferType {
	VERTEX,
	ELEMENT,
	NONE
};

struct Buffer_data {
	int vert_sum = 0;
	BufferType buffer_type;
	std::vector<int> indicies = {};
	std::vector<float> verts = {};
	std::vector<float> colors = {};
	std::vector<float> texture = {};
	std::vector<float> normals = {};

	explicit Buffer_data(BufferType a_type);
	Buffer_data(Buffer_data&& a_data);
	Buffer_data(const Buffer_data& a_data);
	Buffer_data() = delete;
};

class VAO {
public:
	Buffer_data data;

	VAO(const Buffer_data& a_data, bool a_wireframe = false);
	~VAO();

	auto set_pos(const std::vector<float>& a_pos = {}) -> void;
	auto set_color(const std::vector<float>& a_color = {}) -> void;
	auto set_normals(const std::vector<float>& a_normals = {}) -> void;
	auto set_texture(const std::vector<float>& a_texture = {}) -> void;
	auto set_elements(const std::vector<int>& a_elem_indicies = {}) -> void;

	auto reuse_pos(unsigned int a_VBO_vert) -> void;
	auto reuse_color(unsigned int a_VBO_color) -> void;
	auto reuse_normals(unsigned int a_VBO_normal) -> void;
	auto reuse_texture(unsigned int a_VBO_texture) -> void;
	auto reuse_elements(unsigned int a_EBO) -> void;

	auto update_pos() -> void;
	auto update_color() -> void;
	auto update_texture() -> void;
	auto draw_vertices() -> void;

	auto get_VBO_pos() const -> unsigned int;
	auto get_VBO_color() const -> unsigned int;
	auto get_VBO_texture() const -> unsigned int;
	auto get_VBO_indicies() const -> unsigned int;

private:
	unsigned int m_VAO;
	unsigned int m_VBO_vert;
	unsigned int m_VBO_color;
	unsigned int m_VBO_texture;
	unsigned int m_VBO_normal;
	unsigned int m_EBO;
	bool m_wireframe;
};
