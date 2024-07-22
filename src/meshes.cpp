#include <algorithm>
#include <memory>
#include <utility>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <filesystem>
#include <format>

#include "meshes.hpp"
#include "assert.hpp"
#include "shaders.hpp"

namespace fs = std::filesystem;

fs::path get_proj_path() {
	fs::path p = fs::current_path();
	if (p.filename() == "build") {
		p = p.parent_path();
	}

	return p;
}

Texture::Texture() :m_dir{ "" }, m_type{ TextureType::NONE }, m_texture_id{ 0 }, m_texture_unit{ 0 } { }

Texture::Texture(std::string a_dir, TextureType a_type, int a_texture_unit) {
	generate_texture(a_dir, a_type, a_texture_unit);
}

Texture::~Texture() {
	if (m_type != TextureType::NONE)
		glDeleteTextures(1, &m_texture_id);
}

void Texture::generate_texture(std::string a_dir, TextureType a_type, int a_texture_unit) {
	if (a_type != TextureType::NONE)
		glDeleteTextures(1, &m_texture_id);

	m_dir = a_dir;
	m_type = a_type;
	m_texture_unit = a_texture_unit;

	fs::path p = get_proj_path();
	p /= a_dir;

	glGenTextures(1, &m_texture_id);
	glBindTexture(GL_TEXTURE_2D, m_texture_id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // GL_NEAREST variations available
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // GL_NEAREST available

	int nrChannels{ };
	stbi_set_flip_vertically_on_load(true);
	int width{ };
	int height{ };
	unsigned char* data = stbi_load(p.c_str(), &width, &height, &nrChannels, 0);

	unsigned format;
	if (nrChannels == 3) {
		format = GL_RGB;
	} else if (nrChannels == 4) {
		// WARNING:
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		format = GL_RGBA;
	} else {
		ERROR(std::format("Unsupported texture format for {} number of channels.", nrChannels).c_str());
		throw Error_code::init_error;
	}

	// INFO: Testing
	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	} else {
		ERROR(std::format("Couldn't load texture data {}", m_dir).data());
		throw Error_code::init_error;
	}

	/* if (data) {
		std::string extension = p.extension().string();
		if (extension == ".jpeg" || extension == ".jpg") {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
		} else if (extension == ".png") {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
		} else if (extension == ".tga") {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
		} else { // If extension not handled just guess
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
		}
	} else {
		ERROR(std::format("Couldn't load texture data {}", m_dir).data());
		throw Error_code::init_error;
	} */

	stbi_image_free(data);
}

void Texture::activate() const {
	if (m_type != TextureType::NONE) {
		glActiveTexture(GL_TEXTURE0 + m_texture_unit);
		glBindTexture(GL_TEXTURE_2D, m_texture_id);
	}
}

TextureType Texture::get_type() const {
	return m_type;
}

std::string Texture::get_dir() const {
	return m_dir;
}

unsigned int Texture::get_texture_id() const {
	return m_texture_id;
}

int Texture::get_texture_unit() const {
	return m_texture_unit;
}

MaterialMap::MaterialMap(std::initializer_list<std::pair<std::string, TextureType>> a_texture_maps, float a_shininess) 
	:m_shininess{ a_shininess }, m_diffuse_maps{ }, m_specular_maps{ }, m_emission_maps{ }, m_texture_unit_counter{ } 
{
	int i{ 0 };
	for (const auto& a_texture_map : a_texture_maps) {
		m_texture_unit_counter.push_back(i);
		switch (a_texture_map.second) {
			case TextureType::DIFFUSE:
				m_diffuse_maps.push_back(std::make_shared<Texture>(a_texture_map.first, a_texture_map.second, i++));
				break;
			case TextureType::SPECULAR:
				m_specular_maps.push_back(std::make_shared<Texture>(a_texture_map.first, a_texture_map.second, i++));
				break;
			case TextureType::EMISSION:
				m_emission_maps.push_back(std::make_shared<Texture>(a_texture_map.first, a_texture_map.second, i++));
				break;
			case TextureType::NONE:
				ERROR(std::format("Bad texture type for {}", a_texture_map.first).c_str());
				throw Error_code::bad_type;
		}
	}
}

void MaterialMap::set_textures(std::vector<std::shared_ptr<Texture>> a_textures) {
	m_diffuse_maps.clear();
	m_specular_maps.clear();
	m_emission_maps.clear();
	m_texture_unit_counter.clear();

	for (const auto& texture_ptr : a_textures) {
		m_texture_unit_counter.push_back(texture_ptr->get_texture_unit());

		switch (texture_ptr->get_type()) {
			case TextureType::DIFFUSE:
				m_diffuse_maps.push_back(texture_ptr);
				break;
			case TextureType::SPECULAR:
				m_specular_maps.push_back(texture_ptr);
				break;
			case TextureType::EMISSION:
				m_emission_maps.push_back(texture_ptr);
				break;
			case TextureType::NONE:
				break;
		}
	}
}

// FIXME: Fragmentation in m_texture_unit_counter
void MaterialMap::set_diffuse_maps(std::vector<std::string> a_diffuse_maps_names) {
	for (const auto m_diffuse_map : m_diffuse_maps)
		m_texture_unit_counter.erase(std::find(m_texture_unit_counter.begin(), m_texture_unit_counter.end(), m_diffuse_map->get_texture_id()));
	m_diffuse_maps.clear();

	int max_id = *std::max_element(m_texture_unit_counter.begin(), m_texture_unit_counter.end()) + 1;

	for (size_t i = 0; i < a_diffuse_maps_names.size(); i++) {
		m_texture_unit_counter.push_back(max_id + i);
		m_diffuse_maps.push_back(std::make_shared<Texture>(a_diffuse_maps_names[i], TextureType::DIFFUSE, max_id + i));
	}
}

// FIXME: Fragmentation in m_texture_unit_counter
void MaterialMap::set_specular_maps(std::vector<std::string> a_specular_maps_names) {
	for (const auto m_specular_map : m_specular_maps)
		m_texture_unit_counter.erase(std::find(m_texture_unit_counter.begin(), m_texture_unit_counter.end(), m_specular_map->get_texture_id()));
	m_specular_maps.clear();

	int max_id = *std::max_element(m_texture_unit_counter.begin(), m_texture_unit_counter.end()) + 1;

	for (size_t i = 0; i < a_specular_maps_names.size(); i++) {
		m_texture_unit_counter.push_back(max_id + i);
		m_specular_maps.push_back(std::make_shared<Texture>(a_specular_maps_names[i], TextureType::SPECULAR, max_id + i));
	}
}

// FIXME: Fragmentation in m_texture_unit_counter
void MaterialMap::set_emission_maps(std::vector<std::string> a_emission_maps_names) {
	for (const auto m_emission_map : m_emission_maps)
		m_texture_unit_counter.erase(std::find(m_texture_unit_counter.begin(), m_texture_unit_counter.end(), m_emission_map->get_texture_id()));
	m_emission_maps.clear();

	int max_id = *std::max_element(m_texture_unit_counter.begin(), m_texture_unit_counter.end()) + 1;

	for (size_t i = 0; i < a_emission_maps_names.size(); i++) {
		m_texture_unit_counter.push_back(max_id + i);
		m_emission_maps.push_back(std::make_shared<Texture>(a_emission_maps_names[i], TextureType::EMISSION, max_id + i));
	}
}

void MaterialMap::set_shininess(float a_shininess) {
	m_shininess = a_shininess;
}

std::vector<std::shared_ptr<Texture>> MaterialMap::get_diffuse_maps() const {
	return m_diffuse_maps;
}

std::vector<std::shared_ptr<Texture>> MaterialMap::get_specular_maps() const {
	return m_specular_maps;
}

std::vector<std::shared_ptr<Texture>> MaterialMap::get_emission_maps() const {
	return m_emission_maps;
}

float MaterialMap::get_shininess() const {
	return m_shininess;
}

BufferData::BufferData(BufferData::Type a_type)
	:buffer_type{ a_type } { }

Mesh::BufferObjects::BufferObjects() :m_VAO{ EMPTY_VBO }, VBO_pos{ EMPTY_VBO }, VBO_normal{ EMPTY_VBO }, VBO_texture_coords{ EMPTY_VBO }, EBO{ EMPTY_VBO } {  }

Mesh::BufferObjects::~BufferObjects() {
	glDeleteVertexArrays(1, &m_VAO);
	glDeleteBuffers(1, &VBO_pos);
	glDeleteBuffers(1, &VBO_normal);
	glDeleteBuffers(1, &VBO_texture_coords);
	glDeleteBuffers(1, &EBO);
}

Mesh::Mesh() :m_data{ }, m_wireframe{ false }, m_VBOs{ std::make_shared<BufferObjects>() } { }

Mesh::Mesh(const BufferData& a_data, const MaterialMap& a_mat, bool a_wireframe)
	:m_data{ a_data }, m_material_map{ a_mat }, m_wireframe{ a_wireframe }, m_VBOs{ std::make_shared<BufferObjects>() }
{
	glGenVertexArrays(1, &m_VBOs->m_VAO);

	if (!m_data.positions.empty()) {
		set_pos();
	}

	if (!m_data.normals.empty()) {
		set_normals();
	}

	if (!m_data.texture_coords.empty()) {
		set_texture_coords();
	}

	if (m_data.buffer_type == BufferData::Type::ELEMENT && m_data.indicies_sum) {
		set_elements();
	}
}
void Mesh::gen_VAO() {
	if (m_VBOs->m_VAO == EMPTY_VBO)
		glGenVertexArrays(1, &m_VBOs->m_VAO);
}

void Mesh::set_type(BufferData::Type a_type) {
	m_data.buffer_type = a_type;
}

void Mesh::set_pos(const std::vector<glm::vec3>& a_positions) {
	if (!a_positions.empty())
		m_data.positions = a_positions;

	m_data.vert_sum = m_data.positions.size();

	glBindVertexArray(m_VBOs->m_VAO);

	if (m_VBOs->VBO_pos == EMPTY_VBO) 
		glGenBuffers(1, &m_VBOs->VBO_pos);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOs->VBO_pos);
	glBufferData(GL_ARRAY_BUFFER, m_data.positions.size() * sizeof(m_data.positions[0]), m_data.positions.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(ATTRIB_POS_LOCATION, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(ATTRIB_POS_LOCATION);
}


void Mesh::set_normals(const std::vector<glm::vec3>& a_normals) {
	if (!a_normals.empty())
		m_data.normals = a_normals;

	glBindVertexArray(m_VBOs->m_VAO);

	if (m_VBOs->VBO_normal == EMPTY_VBO) 
		glGenBuffers(1, &m_VBOs->VBO_normal);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOs->VBO_normal);
	glBufferData(GL_ARRAY_BUFFER, m_data.normals.size() * sizeof(m_data.normals[0]), m_data.normals.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(ATTRIB_NORMAL_LOCATION, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(ATTRIB_NORMAL_LOCATION);
}

void Mesh::set_texture_coords(const std::vector<glm::vec2>& a_texture_coords) {
	if (!a_texture_coords.empty())
		m_data.texture_coords = a_texture_coords;

	glBindVertexArray(m_VBOs->m_VAO);

	if (m_VBOs->VBO_texture_coords == EMPTY_VBO)
		glGenBuffers(1, &m_VBOs->VBO_texture_coords);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOs->VBO_texture_coords);
	glBufferData(GL_ARRAY_BUFFER, m_data.texture_coords.size() * sizeof(m_data.texture_coords[0]), m_data.texture_coords.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(ATTRIB_TEX_LOCATION, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(ATTRIB_TEX_LOCATION);
}

void Mesh::set_elements(const std::vector<unsigned int>& a_elem_indicies) {
	if (m_data.buffer_type != BufferData::Type::ELEMENT)
		return;

	glBindVertexArray(m_VBOs->m_VAO);

	if (!a_elem_indicies.empty()) {
		m_data.indicies = a_elem_indicies;
		m_data.indicies_sum = a_elem_indicies.size();
	}

	if (m_VBOs->EBO == EMPTY_VBO)
		glGenBuffers(1, &m_VBOs->EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_VBOs->EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_data.indicies.size() * sizeof(m_data.indicies[0]), m_data.indicies.data(), GL_STATIC_DRAW);
}

void Mesh::set_material_map(const MaterialMap& a_material_map) {
	m_material_map = a_material_map;
}

void Mesh::draw(ShaderProgram& a_shader) {
	a_shader.use();

	glBindVertexArray(m_VBOs->m_VAO);

	if (m_wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	switch(m_data.buffer_type) {
		case BufferData::Type::VERTEX:
			glDrawArrays(GL_TRIANGLES, 0, m_data.vert_sum);
			break;
		case BufferData::Type::ELEMENT:
			glDrawElements(GL_TRIANGLES, m_data.indicies_sum, GL_UNSIGNED_INT, 0);
			break;
		default:
			ERROR("Unhandled draw type for buffer object type.");
			throw Error_code::bad_match;
	}

	glBindVertexArray(0);
}

MaterialMap& Mesh::get_material_map() {
	return m_material_map;
}
