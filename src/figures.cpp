#include "glm/ext/matrix_transform.hpp"
#include "glm/fwd.hpp"
#include <format>
#include <memory>

#include <string>
#include <iostream>
#include "figures.hpp"
#include "shaders.hpp"
#include "meshes.hpp"
#include "figures_constants.hpp"

Angle::Angle(float a_roll, float a_pitch, float a_yaw) :roll{ a_roll }, pitch{ a_pitch }, yaw{ a_yaw } { }

Shape::Shape(glm::vec3 a_center, float a_size, float a_degree_angle, BufferType a_data_type, std::vector<glm::vec3> a_positions, std::vector<unsigned int> a_elem_indices, bool a_wireframe)
	:m_center{ a_center }, m_size{ a_size, a_size, a_size }, m_rad_angles{ glm::radians(a_degree_angle), 0.0f, 0.0f }
{
	Buffer_data data{ a_data_type };

	if (!a_positions.empty()) {
		data.positions = a_positions;
	}
	if (!a_elem_indices.empty() && data.buffer_type == BufferType::ELEMENT) {
		data.indicies = a_elem_indices;
	}

	m_transform_scale    = glm::scale(glm::mat4(1.0f), m_size);
	m_transform_rotation = glm::rotate(glm::mat4(1.0f), m_rad_angles.roll, glm::vec3(0.0f, 0.0f, 1.0f));
	m_transform_pos      = glm::translate(glm::mat4(1.0f), glm::vec3(m_center.x, m_center.y, m_center.z));
	m_texture_scalar     = glm::mat2{ a_size };

	m_shape_obj = std::make_shared<Mesh>(data, a_wireframe);
}

Shape::Shape(glm::vec3 a_center, float a_size, float a_degree_angle, BufferType a_data_type, const VBO_COLLECTION& a_VBOs, bool a_wireframe)
	:m_center{ a_center }, m_size{ a_size, a_size, a_size }, m_rad_angles{ glm::radians(a_degree_angle), 0.0f, 0.0f }, m_reused_VBOs{ a_VBOs }
{
	m_transform_scale    = glm::scale(glm::mat4(1.0f), m_size);
	m_transform_rotation = glm::rotate(glm::mat4(1.0f), m_rad_angles.roll, glm::vec3(0.0f, 0.0f, 1.0f));
	m_transform_pos      = glm::translate(glm::mat4(1.0f), glm::vec3(m_center.x, m_center.y, m_center.z));
	m_texture_scalar     = glm::mat2{ a_size };

	m_shape_obj = std::make_shared<Mesh>(a_data_type, a_VBOs, a_wireframe);
}

void Shape::set_normals(const std::vector<glm::vec3>& a_normals) {
	if (!m_shape_obj->m_data.normals.empty())
		m_shape_obj->m_data.normals.clear();

	for (const auto& normal : a_normals)
		m_shape_obj->m_data.normals.push_back(normal);

	m_shape_obj->set_normals();
}

void Shape::set_texture_coords(std::vector<glm::vec2> a_texture_coords) {
	m_shape_obj->set_texture_coords(a_texture_coords);
}

void Shape::set_material_map(const MaterialMap& a_material_map) {
	m_material_map = a_material_map;
}

void Shape::draw(ShaderProgram& a_shader) {
	a_shader.use();

	if (a_shader.get_depth_testing()) {
		glEnable(GL_DEPTH_TEST);
	} else {
		glDisable(GL_DEPTH_TEST);
	}

	m_shape_obj->draw_vertices();
}

void Shape::rotate(float degree_angle, Axis axis) {
	switch(axis) {
		case Axis::X : {
			m_rad_angles.roll += glm::radians(degree_angle);
			m_transform_rotation = glm::rotate(m_transform_rotation, glm::radians(degree_angle), glm::vec3(1.0f, 0.0f, 0.0f));
			break;
		}
		case Axis::Y : {
			m_rad_angles.pitch += glm::radians(degree_angle);
			m_transform_rotation = glm::rotate(m_transform_rotation, glm::radians(degree_angle), glm::vec3(0.0f, 1.0f, 0.0f));
			break;
		}
		case Axis::Z : {
			m_rad_angles.yaw += glm::radians(degree_angle);
			m_transform_rotation = glm::rotate(m_transform_rotation, glm::radians(degree_angle), glm::vec3(0.0f, 0.0f, 1.0f));
			break;
		}
	}
}

void Shape::set_pos(glm::vec3 a_center) {
	m_center = a_center;
	m_transform_pos = glm::mat4(1.0f);
	m_transform_pos = glm::translate(m_transform_pos, m_center);

}

void Shape::move(glm::vec3 a_vec) {
	m_center += a_vec;
	m_transform_pos = glm::translate(m_transform_pos, a_vec);
}

void Shape::resize(float a_size) {
	m_size = glm::vec3(a_size, a_size, a_size);
	m_texture_scalar = glm::mat2{ a_size };
	m_transform_scale = glm::scale(m_transform_scale, m_size);
}

void Shape::resize(glm::vec3 a_size) {
	m_size = a_size;
	m_transform_scale = glm::scale(m_transform_scale, m_size);
}

void Shape::reset() {
	m_transform_scale = glm::mat4(1.0f);
	m_transform_pos = glm::mat4(1.0f);
	m_transform_rotation = glm::mat4(1.0f);
}

MaterialMap& Shape::get_material_map() {
	return m_material_map;
}

Buffer_data Shape::get_obj_data() const {
	return m_shape_obj->m_data;
}

glm::vec3 Shape::get_size() const {
	return m_size;
}

glm::vec3 Shape::get_pos() const {
	return m_center;
}

glm::mat4 Shape::get_model() const {
	return m_transform_pos * m_transform_rotation * m_transform_scale;
}

glm::mat3 Shape::get_normal_mat() const {
	// Get normal matix from M matrix
	return glm::transpose(glm::inverse(glm::mat3(get_model())));
}

// WARNING:
bool Shape::inside_viewport() const {
	return false;
}

void Shape::print_verts() const {
	for(const auto& position : m_shape_obj->m_data.positions) {
		std::cout << std::format("({}, {}, {})\n", position[0], position[1], position[2]);
	}
	std::cout << std::endl;
}

void Shape::print_texture_verts() const {
	for(const auto& texture_coord : this->m_shape_obj->m_data.texture_coords) {
		std::cout << std::format("({}, {})\n", texture_coord[0], texture_coord[1]);
	}
	std::cout << std::endl;
}

Triangle2D::Triangle2D(glm::vec3 a_center, float a_size, float a_degree_angle, bool a_wireframe)
	:Shape(a_center, a_size, a_degree_angle, BufferType::VERTEX,
		     { // Verticies
				glm::vec3(0.0f,  0.5f, 0.0f),
				glm::vec3(0.5f, -0.5f, 0.0f),
				glm::vec3(-0.5f, -0.5f, 0.0f),
		     }, {  }, a_wireframe)
{ }

Rectangle2D::Rectangle2D(glm::vec3 a_center, float a_size, float a_degree_angle, bool a_wireframe)
	:Shape(a_center, a_size, a_degree_angle, BufferType::ELEMENT,
		  { // Verticies
			 glm::vec3(0.5f,  0.5f, 0.0f),
			 glm::vec3(0.5f, -0.5f, 0.0f),
			 glm::vec3(-0.5f, -0.5f, 0.0f),
			 glm::vec3(-0.5f,  0.5f, 0.0f),
		  },
		  { // Element indicies
			   0, 1, 3,
			   1, 2, 3,
		  }, a_wireframe)
{ }

Cube::Cube(glm::vec3 a_center, float a_size, float a_degree_angle, bool a_wireframe)
	:Shape(a_center, a_size, a_degree_angle, BufferType::VERTEX, CUBE_POSITIONS, { }, a_wireframe) {  }

Cube::Cube(glm::vec3 a_center, float a_size, float a_degree_angle, const VBO_COLLECTION& a_VBOs, bool a_wireframe)
	:Shape(a_center, a_size, a_degree_angle, BufferType::VERTEX, a_VBOs, a_wireframe) { }

/*********************
 *    DEPRACATED     *
 *********************

void Shape::set_material(const Material& a_material) {
	m_material_used = true;

	m_material.ambient = a_material.ambient;
	m_material.diffuse = a_material.diffuse;
	m_material.specular = a_material.specular;
	m_material.shininess = a_material.shininess;
}

void Shape::overwrite_pos() {
	m_shape_obj->data.verts = {
		m_center.x + std::cos(m_rad_angles.roll) * m_size.x, m_center.y + std::sin(m_rad_angles.roll) * m_size.y, 0.0f,
		m_center.x + std::cos(m_rad_angles.roll) * m_size.x, m_center.y + std::sin(m_rad_angles.roll) * m_size.y, 0.0f,
		m_center.x + std::cos(m_rad_angles.roll) * m_size.x, m_center.y + std::sin(m_rad_angles.roll) * m_size.y, 0.0f,
	};
}

void Shape::overwrite_texture_pos() {
	m_shape_obj->data.texture = {
		((std::cos(m_rad_angles.roll) / 2) + 0.5f) * m_texture_ratio, ((std::sin(m_rad_angles.roll) / 2) + 0.5f) * m_texture_ratio,
		((std::cos(m_rad_angles.roll) / 2) + 0.5f) * m_texture_ratio, ((std::sin(m_rad_angles.roll) / 2) + 0.5f) * m_texture_ratio,
		((std::cos(m_rad_angles.roll) / 2) + 0.5f) * m_texture_ratio, ((std::sin(m_rad_angles.roll) / 2) + 0.5f) * m_texture_ratio,
	};
}

void Shape::set_ambient(const glm::vec3& a_ambient) {
	m_material.ambient = a_ambient;
}

void Shape::set_diffuse(const glm::vec3& a_diffuse) {
	m_material.diffuse = a_diffuse;
}

void Shape::set_specular(const glm::vec3& a_specular) {
	m_material.specular = a_specular;
}

void Shape::set_shininess(float a_shininess) {
	m_material.shininess = a_shininess;
}

void Shape::init_texture_stack(std::initializer_list<std::string> a_texture_names) {
	if (!m_texture_stack.empty()) m_texture_stack.clear();
	for (auto& texture_name : a_texture_names)
		m_texture_stack.emplace_back(texture_name, Texture::Type::TEX_2D);
}

// The higher ratio the smaller texture
void Shape::_texture_resize(float a_ratio) {
	m_texture_ratio = a_ratio;
	overwrite_texture_pos();
	m_shape_obj->update_texture();
}

Shape::Shape(const Shape& a_shape)
	:m_center{ a_shape.m_center }, m_size{ a_shape.m_size }, m_rad_angles{ a_shape.m_rad_angles }, m_shape_obj{ new VAO(*a_shape.m_shape_obj) },
	 m_texture_ratio{ a_shape.m_texture_ratio }, m_material_map{ a_shape.m_material_map }, m_reused_VBOs{ a_shape.m_reused_VBOs }
{
	if (m_reused_VBOs.VERTS != EMPTY_VBO) m_shape_obj->reuse_pos(m_reused_VBOs.VERTS);
	if (m_reused_VBOs.NORMALS != EMPTY_VBO) m_shape_obj->reuse_pos(m_reused_VBOs.NORMALS);
	if (m_reused_VBOs.TEXTURE != EMPTY_VBO) m_shape_obj->reuse_pos(m_reused_VBOs.TEXTURE);
	if (m_reused_VBOs.INDICIES != EMPTY_VBO && m_shape_obj->data.buffer_type == BufferType::ELEMENT) m_shape_obj->reuse_pos(m_reused_VBOs.INDICIES);
}

void Shape::set_color(const std::vector<float>& a_color, bool a_multi, std::vector<std::vector<int>> a_color_order) {
	// Shift every component to the right
	auto rshift_color_vector = [](std::vector<float>& a_color_dst) { 
		auto tmp = a_color_dst[a_color_dst.size() - 1];
		for (auto j = a_color_dst.size() - 1; j  > 0; j--) {
			a_color_dst[j] = a_color_dst[j-1];
		}
		a_color_dst[0] = tmp;
	};

	if (!m_shape_obj->mdata.colors.empty())
		m_shape_obj->data.colors.clear();

	if (!a_multi) {
		for (auto i = 0; i < m_shape_obj->data.vert_sum; i++) {
			for (auto j = 0; j < 3; j++) { // For each color component
				m_shape_obj->data.colors.push_back(a_color[j]);
			}
		}
	} else {
		if (a_color_order.empty()) {
			std::vector<float> color_dst = a_color;
			for (auto i = 0; i < m_shape_obj->data.vert_sum; i++) {
				for (auto j = 0; j < 3; j++) { // For each color component
					m_shape_obj->data.colors.push_back(color_dst[j]);
				}

				rshift_color_vector(color_dst);
			}
		} else {
			m_shape_obj->data.colors.resize(m_shape_obj->data.vert_sum * 3);

			std::vector<float> color_dst = a_color;
			for (int vertex = 0; vertex < a_color_order.size(); vertex++) {
				for (int buffer_pos = 0; buffer_pos < a_color_order[vertex].size(); buffer_pos++) {
					for (int i = 0; i < 3; i++) {
						m_shape_obj->data.colors[a_color_order[vertex][buffer_pos] * 3 + i] = color_dst[i];
					}
				}
				rshift_color_vector(color_dst);
			}
		}
	}

	m_shape_obj->set_color();
}


*/
