#include "glm/ext/matrix_transform.hpp"
#include "glm/fwd.hpp"
#include <format>
#include <initializer_list>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <cmath>
#include <string>
#include <iostream>
#include <filesystem>

#include "assert.hpp"
#include "figures.hpp"
#include "shaders.hpp"
#include "buffer_obj.hpp"
#include "figures_constants.hpp"

namespace fs = std::filesystem;

static fs::path get_asset_path() {
	fs::path p = fs::current_path();
	if (p.filename() == "build") {
		p = p.parent_path();
	}
	
	for (auto& i : fs::directory_iterator{ p }) {
		if (i.is_directory() && i.path().filename() == "assets") {
			p = i.path();
			break;
		}
	}

	if (p.filename() != "assets") {
		ERROR("No assets directory.");
		throw Error_code::file_not_found;
	}

	return p;
}

Texture::Texture() :m_name{ "" }, m_type{ TextureType::NONE }, m_texture_id{ 0 }, m_texture_unit{ 0 }, m_width{ 0 }, m_height{ 0 }, m_deletable{ false } {}

Texture::Texture(std::string a_name, TextureType a_type, int a_texture_unit) {
	// generate_texture deletes older texture if m_deletable is true
	// before generating a new one.
	m_deletable = false;
	generate_texture(a_name, a_type, a_texture_unit);
}

Texture::Texture(const Texture& a_texture) 
	:m_name{ a_texture.m_name }, m_type{ a_texture.m_type }, m_texture_id{ a_texture.m_texture_id },
	 m_texture_unit{ a_texture.m_texture_unit }, m_width{ a_texture.m_width }, m_height{ a_texture.m_height }, m_deletable{ false } { }

Texture::Texture(Texture&& a_texture) {
	m_name         = std::move(a_texture.m_name);
	m_type         = a_texture.m_type;
	m_width        = a_texture.m_width;
	m_height       = a_texture.m_height;
	m_deletable    = a_texture.m_deletable;
	m_texture_id   = a_texture.m_texture_id;
	m_texture_unit = a_texture.m_texture_unit;
	a_texture.m_deletable = false;
}

Texture::~Texture() {
	if (m_deletable && m_type != TextureType::NONE)
		glDeleteTextures(1, &m_texture_id);
}

Texture& Texture::operator=(const Texture& a_texture) {
    m_type         = a_texture.m_type;
    m_name         = a_texture.m_name;
    m_width        = a_texture.m_width;
    m_height       = a_texture.m_height;
    m_texture_id   = a_texture.m_texture_id;
    m_texture_unit = a_texture.m_texture_unit;

    m_deletable = false;
	
	return *this;
}

Texture& Texture::operator=(Texture&& a_texture) {
    m_name         = std::move(a_texture.m_name);
    m_type         = a_texture.m_type;
    m_width        = a_texture.m_width;
    m_height       = a_texture.m_height;
    m_deletable    = a_texture.m_deletable;
    m_texture_id   = a_texture.m_texture_id;
    m_texture_unit = a_texture.m_texture_unit;

	a_texture.m_deletable = false;

	return *this;
}

void Texture::generate_texture(std::string a_name, TextureType a_type, int a_texture_unit) {
	if (m_deletable && a_type != TextureType::NONE)
		glDeleteTextures(1, &m_texture_id);

	m_name = a_name;
	m_type = a_type;
	m_texture_unit = a_texture_unit;
	m_deletable = true;

	fs::path p = get_asset_path();
	p /= m_name;

	glGenTextures(1, &m_texture_id);
	glBindTexture(GL_TEXTURE_2D, m_texture_id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // GL_NEAREST variations available
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // GL_NEAREST available

	int nrChannels{ 0 };
	stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load(p.c_str(), &m_width, &m_height, &nrChannels, 0);
	if (data) {
		std::string extension = p.extension().string();
		if (extension == ".jpeg" || extension == ".jpg") {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
		} else if (extension == ".png") {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
		} else { // If extension not handled just guess
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
		}
	} else {
		ERROR(std::format("Couldn't load texture data {}.", m_name).data());
		throw Error_code::init_error;
	}
	stbi_image_free(data);
}

void Texture::activate() {
	if (m_type != TextureType::NONE) {
		glActiveTexture(GL_TEXTURE0 + m_texture_unit);
		glBindTexture(GL_TEXTURE_2D, m_texture_id);
	}
}

int Texture::get_width() const {
	return m_width;
}

int Texture::get_height() const {
	return m_height;
}

TextureType Texture::get_type() const {
	return m_type;
}

std::string Texture::get_name() const {
	return m_name;
}

unsigned int Texture::get_texture_id() const {
	return m_texture_id;
}

Point3D::Point3D(float a_x, float a_y, float a_z) :x{ a_x }, y{ a_y }, z{ a_z } { }

Point3D& Point3D::operator+=(const Point3D& p) {
	x += p.x;
	y += p.y;
	z += p.z;
	return *this;
}

Point3D& Point3D::operator-=(const Point3D& p) {
	x -= p.x;
	y -= p.y;
	z -= p.z;
	return *this;
}

Point3D& Point3D::operator*=(const Point3D& p) {
	x *= p.x;
	y *= p.y;
	z *= p.z;
	return *this;
}

Point3D& Point3D::operator/=(const Point3D& p) {
	x /= p.x;
	y /= p.y;
	z /= p.z;
	return *this;
}

Angle::Angle(float a_roll, float a_pitch, float a_yaw) :roll{ a_roll }, pitch{ a_pitch }, yaw{ a_yaw } { }

Material::Material() :ambient(0.0f), diffuse(0.0f), specular(0.0f), shininess(0.0f) { }

Material::Material(glm::vec3 a_ambient, glm::vec3 a_diffuse, glm::vec3 a_specular, float a_shininess)
	:ambient(a_ambient), diffuse(a_diffuse), specular(a_specular), shininess(a_shininess) { }

MaterialMap::MaterialMap(std::string a_diffuse_map, std::string a_specular_map, std::string a_emission_map, float a_shininess)
	:diffuse_map { (a_diffuse_map  != "") ? new Texture{ a_diffuse_map,  TextureType::TEX_2D, DIFFUSE_MAP_ID  } : new Texture{} },
	 specular_map{ (a_specular_map != "") ? new Texture{ a_specular_map, TextureType::TEX_2D, SPECULAR_MAP_ID } : new Texture{} },
	 emission_map{ (a_emission_map != "") ? new Texture{ a_emission_map, TextureType::TEX_2D, EMISSION_MAP_ID } : new Texture{} },
	 shininess{ a_shininess } { }

MaterialMap::MaterialMap(const MaterialMap& a_material_map)
	:diffuse_map { new Texture{ *a_material_map.diffuse_map } },
	 specular_map{ new Texture{ *a_material_map.specular_map } },
	 emission_map{ new Texture{ *a_material_map.emission_map } },
	 shininess{ a_material_map.shininess } { }

void MaterialMap::set_diffuse_map(std::string a_diffuse_map) {
	diffuse_map.reset(new  Texture{ a_diffuse_map,  TextureType::TEX_2D, DIFFUSE_MAP_ID });
}

void MaterialMap::set_specular_map(std::string a_specular_map) {
	specular_map.reset(new Texture{ a_specular_map, TextureType::TEX_2D, SPECULAR_MAP_ID });
}

void MaterialMap::set_emission_map(std::string a_emission_map) {
	emission_map.reset(new Texture{ a_emission_map, TextureType::TEX_2D, EMISSION_MAP_ID });
}

void MaterialMap::set_shininess(float a_shininess) {
	shininess = a_shininess;
}

VBO_FIGURES::~VBO_FIGURES() {
	glDeleteBuffers(1, &VERTS);
	glDeleteBuffers(1, &TEXTURE);
	glDeleteBuffers(1, &INDICIES);
	glDeleteBuffers(1, &NORMALS);
}

Shape::Shape(Point3D a_center, float a_size, float a_degree_angle, BufferType a_data_type, std::vector<float> a_verts, std::vector<int> a_elem_indices, bool a_wireframe)
	:m_center{ std::move(a_center) }, m_size{ a_size, a_size, a_size }, m_rad_angles{ glm::radians(a_degree_angle), 0.0f, 0.0f },
	 m_material_map_used{ false }
{
	Buffer_data data{ a_data_type };
	if (!a_verts.empty()) {
		data.verts = a_verts;
	}
	if (!a_elem_indices.empty() && data.buffer_type == BufferType::ELEMENT) {
		data.indicies = a_elem_indices;
	}

	m_transform_scale    = glm::scale(glm::mat4(1.0f), m_size);
	m_transform_rotation = glm::rotate(glm::mat4(1.0f), m_rad_angles.roll, glm::vec3(0.0f, 0.0f, 1.0f));
	m_transform_pos      = glm::translate(glm::mat4(1.0f), glm::vec3(m_center.x, m_center.y, m_center.z));
	m_texture_scalar     = glm::mat2{ a_size };

	m_shape_obj = std::make_unique<VAO>(data, a_wireframe);
	m_shape_obj->data.vert_sum = a_verts.size() / 3;
}

Shape::Shape(Point3D a_center, float a_size, float a_degree_angle, BufferType a_data_type, const VBO_FIGURES& a_VBOs, bool a_wireframe)
	:m_center{ std::move(a_center) }, m_size{ a_size, a_size, a_size }, m_rad_angles{ glm::radians(a_degree_angle), 0.0f, 0.0f },
	 m_material_map_used{ false }
{
	Buffer_data data{ a_data_type };

	m_transform_scale    = glm::scale(glm::mat4(1.0f), m_size);
	m_transform_rotation = glm::rotate(glm::mat4(1.0f), m_rad_angles.roll, glm::vec3(0.0f, 0.0f, 1.0f));
	m_transform_pos      = glm::translate(glm::mat4(1.0f), glm::vec3(m_center.x, m_center.y, m_center.z));
	m_texture_scalar     = glm::mat2{ a_size };

	m_shape_obj = std::make_unique<VAO>(data, a_wireframe);
	m_shape_obj->reuse_pos(a_VBOs.VERTS);
	m_shape_obj->reuse_normals(a_VBOs.NORMALS);
	m_shape_obj->reuse_texture(a_VBOs.TEXTURE);
	if (a_VBOs.INDICIES != EMPTY_VBO && data.buffer_type == BufferType::ELEMENT)
		m_shape_obj->reuse_elements(a_VBOs.INDICIES);
	
	m_shape_obj->data.vert_sum = a_VBOs.vert_sum;
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

	if (!m_shape_obj->data.colors.empty())
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

void Shape::set_normals(const std::vector<float>& a_normals) {
	if (!m_shape_obj->data.normals.empty())
		m_shape_obj->data.normals.clear();

	for (const auto& normal : a_normals)
		m_shape_obj->data.normals.push_back(normal);

	m_shape_obj->set_normals();
}

void Shape::set_texture_buf(std::vector<float> a_texture_cords, float a_ratio) {
	m_texture_ratio = a_ratio;

	if (!m_shape_obj->data.texture.empty())
		m_shape_obj->data.texture.clear();

	if (!a_texture_cords.empty()) {
		for (auto cord : a_texture_cords) {
			m_shape_obj->data.texture.push_back(cord * m_texture_ratio);
		}
	} else {
		// Put coordinates to <(0.0, 0.0), (1.0, 1.0)> window.
		// texture_ratio = 2.f; // Usually you would want to map texture 1:1 to figure but if you feeling wacky then change it
		int i{ 0 };
		for (const auto& vert : m_shape_obj->data.verts) {
			m_shape_obj->data.texture.push_back((vert / m_size[(i++ % 3)]  + 0.5f) * m_texture_ratio); // skip z coord
			// else shape_obj->data.vert_texture.push_back(vert / size * texture_ratio);
		}
	}

	m_shape_obj->set_texture();
}

void Shape::set_material_map(const MaterialMap& a_material_map) {
	m_material_map_used = true;
	
	m_material_map.diffuse_map.reset(new Texture(*a_material_map.diffuse_map));
	m_material_map.specular_map.reset(new Texture(*a_material_map.specular_map));
	m_material_map.emission_map.reset(new Texture(*a_material_map.emission_map));
	m_material_map.shininess = a_material_map.shininess;
}

void Shape::toggle_material_map() {
	m_material_map_used = !m_material_map_used;
}

void Shape::draw(Shader_program& a_shader) {
	a_shader.use();

	if (std::string model_name = a_shader.get_model_name(); model_name != "")
		a_shader[model_name] = get_model();

	if (a_shader.get_depth_testing()) {
		glEnable(GL_DEPTH_TEST);
	} else {
		glDisable(GL_DEPTH_TEST);
	}

	if (m_material_map_used) {
		a_shader[a_shader.get_material_name() + ".shininess"] = m_material_map.shininess;
		m_material_map.diffuse_map->activate();
		m_material_map.specular_map->activate();
		m_material_map.emission_map->activate();
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

void Shape::set_pos(Point3D a_center) {
	m_center = a_center;
	m_transform_pos = glm::mat4(1.0f);
	m_transform_pos = glm::translate(m_transform_pos, glm::vec3(m_center.x, m_center.y, m_center.z));

}

void Shape::set_pos(std::vector<float> a_vec) {
	if (a_vec.size() != 3)
		return;

	m_center.x = a_vec[0];
	m_center.y = a_vec[1];
	m_center.z = a_vec[2];
	m_transform_pos = glm::mat4{ 1.0f };
	m_transform_pos = glm::translate(m_transform_pos, glm::vec3(a_vec[0], a_vec[1], a_vec[2]));
}

void Shape::move(Point3D a_vec) {
	m_center += a_vec;
	m_transform_pos = glm::translate(m_transform_pos, glm::vec3(a_vec.x, a_vec.y, a_vec.z));
}

void Shape::move(std::vector<float> a_vec) {
	if (a_vec.size() != 3)
		return;

	m_center.x += a_vec[0];
	m_center.y += a_vec[1];
	m_center.z += a_vec[2];
	m_transform_pos = glm::translate(m_transform_pos, glm::vec3(a_vec[0], a_vec[1], a_vec[2]));
}

void Shape::resize(float a_size) {
	m_size = glm::vec3(a_size, a_size, a_size);
	m_texture_scalar = glm::mat2{ a_size };
	m_transform_scale = glm::scale(m_transform_scale, m_size);
}

void Shape::resize(std::vector<float> a_size) {
	if (a_size.size() != 3)
		return;

	m_size = glm::vec3(a_size[0], a_size[1], a_size[2]);
	m_transform_scale = glm::scale(m_transform_scale, m_size);
}

void Shape::reset() {
	m_transform_scale = glm::mat4(1.0f);
	m_transform_pos = glm::mat4(1.0f);
	m_transform_rotation = glm::mat4(1.0f);
}

Buffer_data Shape::get_obj_data() const {
	return m_shape_obj->data;
}

glm::vec3 Shape::get_size() const {
	return m_size;
}

Point3D Shape::get_pos() const {
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
	int i{ 0 };
	for(const auto& x : this->m_shape_obj->data.verts) {
		std::cout << x << ((++i % 3) ? ", " : "\n");
	}
	std::cout << std::endl;
}

void Shape::print_texture_verts() const {
	int i{ 0 };
	for(const auto& x : this->m_shape_obj->data.texture) {
		std::cout << x << ((++i % 3) ? ", " : "\n");
	}
	std::cout << std::endl;
}

Triangle2D::Triangle2D(Point3D a_center, float a_size, float a_degree_angle, bool a_wireframe)
	:Shape(a_center, a_size, a_degree_angle, BufferType::VERTEX,
		     { // Verticies
				  0.0f,  0.5f, 0.0f,
				  0.5f, -0.5f, 0.0f,
				 -0.5f, -0.5f, 0.0f,
		     }, {  }, a_wireframe)
{ }

Rectangle2D::Rectangle2D(Point3D a_center, float a_size, float a_degree_angle, bool a_wireframe)
	:Shape(a_center, a_size, a_degree_angle, BufferType::ELEMENT,
		  { // Verticies
			   0.5f,  0.5f, 0.0f,
			   0.5f, -0.5f, 0.0f,
			  -0.5f, -0.5f, 0.0f,
			  -0.5f,  0.5f, 0.0f,
		  },
		  { // Element indicies
			   0, 1, 3,
			   1, 2, 3,
		  }, a_wireframe)
{ }

Cube::Cube(Point3D a_center, float a_size, float a_degree_angle, bool a_wireframe)
	:Shape(a_center, a_size, a_degree_angle, BufferType::VERTEX, CUBE_VERT_CORDS, { }, a_wireframe) {  }

Cube::Cube(Point3D a_center, float a_size, float a_degree_angle, const VBO_FIGURES& a_VBOs, bool a_wireframe)
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
*/
