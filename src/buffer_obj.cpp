#include "assert.hpp"
#include "buffer_obj.hpp"

Buffer_data::Buffer_data(BufferType a_type)
	:buffer_type{ a_type }
{}

Buffer_data::Buffer_data(Buffer_data&& a_data) {
	verts = std::move(a_data.verts);
	colors = std::move(a_data.colors);
	texture = std::move(a_data.texture);
	indicies = std::move(a_data.indicies);
	buffer_type = a_data.buffer_type;
}

Buffer_data::Buffer_data(const Buffer_data& a_data) {
	verts       = a_data.verts;
	colors      = a_data.colors;
	texture     = a_data.texture;
	indicies    = a_data.indicies;
	buffer_type = a_data.buffer_type;
}

VAO::VAO(const Buffer_data& a_data, bool a_wireframe)
	:data{ a_data }, m_wireframe{ a_wireframe }, m_VBO_color{ EMPTY_VBO }, m_VBO_vert{ EMPTY_VBO },
	 m_VBO_texture{ EMPTY_VBO }, m_VBO_normal{ EMPTY_VBO }, m_EBO{ EMPTY_VBO }
{
	glGenVertexArrays(1, &m_VAO);

	if (!data.verts.empty()) {
		set_pos();
	}

	if (!data.colors.empty()) {
		set_color();
	}

	if (!data.normals.empty()) {
		set_normals();
	}

	if (!data.texture.empty()) {
		set_texture();
	}

	if (data.buffer_type == BufferType::ELEMENT && !data.indicies.empty()) {
		set_elements();
	}
}

void VAO::set_pos(const std::vector<float>& a_pos) {
	if (!a_pos.empty())
		data.verts = a_pos;

	glBindVertexArray(m_VAO);

	if (m_VBO_vert == EMPTY_VBO) 
		glGenBuffers(1, &m_VBO_vert);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO_vert);
	glBufferData(GL_ARRAY_BUFFER, data.verts.size() * sizeof(data.verts[0]), data.verts.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(ATTRIB_POS_LOCATION, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(ATTRIB_POS_LOCATION);
}

void VAO::set_color(const std::vector<float>& a_color) {
	if (!a_color.empty())
		data.colors = a_color;

	glBindVertexArray(m_VAO);

	if (m_VBO_color == EMPTY_VBO) 
		glGenBuffers(1, &m_VBO_color);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO_color);
	glBufferData(GL_ARRAY_BUFFER, data.colors.size() * sizeof(data.colors[0]), data.colors.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(ATTRIB_COLOR_LOCATION, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(ATTRIB_COLOR_LOCATION);
}

void VAO::set_normals(const std::vector<float>& a_normals) {
	if (!a_normals.empty())
		data.normals = a_normals;

	glBindVertexArray(m_VAO);

	if (m_VBO_normal == EMPTY_VBO) 
		glGenBuffers(1, &m_VBO_normal);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO_normal);
	glBufferData(GL_ARRAY_BUFFER, data.normals.size() * sizeof(float), data.normals.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(ATTRIB_NORMAL_LOCATION, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(ATTRIB_NORMAL_LOCATION);
}

void VAO::set_texture(const std::vector<float>& a_texture) {
	if (!a_texture.empty())
		data.texture = a_texture;

	glBindVertexArray(m_VAO);

	if (m_VBO_texture == EMPTY_VBO)
		glGenBuffers(1, &m_VBO_texture);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO_texture);
	glBufferData(GL_ARRAY_BUFFER, data.texture.size() * sizeof(float), data.texture.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(ATTRIB_TEX_LOCATION, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(ATTRIB_TEX_LOCATION);
}

void VAO::set_elements(const std::vector<int>& a_elem_indicies) {
	glBindVertexArray(m_VAO);

	if (!a_elem_indicies.empty())
		data.indicies = a_elem_indicies;

	if (m_EBO == EMPTY_VBO)
		glGenBuffers(1, &m_EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, data.indicies.size() * sizeof(int), data.indicies.data(), GL_STATIC_DRAW);
}

void VAO::reuse_pos(unsigned int a_VBO_vert) {
	data.verts.clear();

	m_VBO_vert = EMPTY_VBO;

	glBindVertexArray(m_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, a_VBO_vert);

	glVertexAttribPointer(ATTRIB_POS_LOCATION, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(ATTRIB_POS_LOCATION);
}

void VAO::reuse_color(unsigned int a_VBO_color) {
	data.colors.clear();

	m_VBO_color = EMPTY_VBO;

	glBindVertexArray(m_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, a_VBO_color);

	glVertexAttribPointer(ATTRIB_COLOR_LOCATION, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(ATTRIB_COLOR_LOCATION);
}

void VAO::reuse_normals(unsigned int a_VBO_normal) {
	data.normals.clear();

	m_VBO_normal = EMPTY_VBO;

	glBindVertexArray(m_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, a_VBO_normal);

	glVertexAttribPointer(ATTRIB_NORMAL_LOCATION, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(ATTRIB_NORMAL_LOCATION);
}

void VAO::reuse_texture(unsigned int a_VBO_texture) {
	data.texture.clear();

	m_VBO_texture = EMPTY_VBO;

	glBindVertexArray(m_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, a_VBO_texture);

	glVertexAttribPointer(ATTRIB_TEX_LOCATION, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(ATTRIB_TEX_LOCATION);
}

void VAO::reuse_elements(unsigned int a_EBO) {
	data.indicies.clear();

	m_EBO = EMPTY_VBO;

	glBindVertexArray(m_VAO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, a_EBO);
}

void VAO::update_color() {
	if (data.colors.empty()) return;

	glBindBuffer(GL_ARRAY_BUFFER, m_VBO_color);
	glBufferData(GL_ARRAY_BUFFER, data.colors.size() * sizeof(float), data.colors.data(), GL_STATIC_DRAW);
}

void VAO::update_pos() {
	if (data.verts.empty()) return;

	glBindBuffer(GL_ARRAY_BUFFER, m_VBO_vert);
	glBufferData(GL_ARRAY_BUFFER, data.verts.size() * sizeof(float), data.verts.data(), GL_STATIC_DRAW);
}

void VAO::update_texture() {
	if (data.texture.empty()) return;

	glBindBuffer(GL_ARRAY_BUFFER, m_VBO_texture);
	glBufferData(GL_ARRAY_BUFFER, data.texture.size() * sizeof(float), data.texture.data(), GL_STATIC_DRAW);
}

void VAO::draw_vertices() {
	glBindVertexArray(m_VAO);

	if (m_wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	switch(data.buffer_type) {
		case (BufferType::VERTEX) : {
			glDrawArrays(GL_TRIANGLES, 0, data.vert_sum);
			break;
		}
		case (BufferType::ELEMENT) : {
			glDrawElements(GL_TRIANGLES, data.indicies.size(), GL_UNSIGNED_INT, 0);
			break;
		}
		default : {
			ERROR("Unhandled draw type for buffer object type.");
			throw Error_code::bad_match;
		}
	}

	glBindVertexArray(0);
}

unsigned int VAO::get_VBO_pos() const {
	return m_VBO_vert;
}

unsigned int VAO::get_VBO_color() const {
	return m_VBO_color;
}

unsigned int VAO::get_VBO_texture() const {
	return m_VBO_texture;
}

unsigned int VAO::get_VBO_indicies() const {
	return m_EBO;
}

VAO::~VAO() {
	glDeleteVertexArrays(1, &m_VAO);
	glDeleteBuffers(1, &m_VBO_vert);
	glDeleteBuffers(1, &m_VBO_color);
	glDeleteBuffers(1, &m_VBO_texture);
	glDeleteBuffers(1, &m_EBO);
}
