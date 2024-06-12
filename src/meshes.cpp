#include <algorithm>
#include <iostream>
#include <memory>
#include <utility>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <filesystem>
#include <format>

#include "assert.hpp"
#include "meshes.hpp"
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

Texture::Texture() :m_name{ "" }, m_type{ TextureType::NONE }, m_texture_id{ 0 }, m_texture_unit{ 0 }, m_deletable{ false } { }

Texture::Texture(std::string a_name, TextureType a_type, int a_texture_unit) {
	// generate_texture deletes older texture if m_deletable is true
	// before generating a new one. Because we are in a constructor 
	// we avoid with line below.
	m_deletable = false;
	generate_texture(a_name, a_type, a_texture_unit);
}

Texture::Texture(const Texture& a_texture) 
	:m_name{ a_texture.m_name }, m_type{ a_texture.m_type }, m_texture_id{ a_texture.m_texture_id },
	 m_texture_unit{ a_texture.m_texture_unit }, m_deletable{ false } { }

Texture::Texture(Texture&& a_texture) {
	m_name         = std::move(a_texture.m_name);
	m_type         = a_texture.m_type;
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
    m_texture_id   = a_texture.m_texture_id;
    m_texture_unit = a_texture.m_texture_unit;

    m_deletable = false;
	
	return *this;
}

Texture& Texture::operator=(Texture&& a_texture) {
    m_name         = std::move(a_texture.m_name);
    m_type         = a_texture.m_type;
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

	int nrChannels{ };
	stbi_set_flip_vertically_on_load(true);
	int width{ };
	int height{ };
	unsigned char* data = stbi_load(p.c_str(), &width, &height, &nrChannels, 0);
	if (data) {
		std::string extension = p.extension().string();
		if (extension == ".jpeg" || extension == ".jpg") {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
		} else if (extension == ".png") {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
		} else { // If extension not handled just guess
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
		}
	} else {
		ERROR(std::format("Couldn't load texture data {}.", m_name).data());
		throw Error_code::init_error;
	}
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

std::string Texture::get_name() const {
	return m_name;
}

unsigned int Texture::get_texture_id() const {
	return m_texture_id;
}

int Texture::get_texture_unit() const {
	return m_texture_unit;
}

/* Material::Material() :ambient(0.0f), diffuse(0.0f), specular(0.0f), shininess(0.0f) { } */

/* Material::Material(glm::vec3 a_ambient, glm::vec3 a_diffuse, glm::vec3 a_specular, float a_shininess)
	:ambient(a_ambient), diffuse(a_diffuse), specular(a_specular), shininess(a_shininess) { } */

MaterialMap::MaterialMap(std::initializer_list<std::pair<std::string, TextureType>> a_texture_maps, float a_shininess) 
	:m_shininess{ a_shininess }, m_diffuse_maps{ }, m_specular_maps{ }, m_emission_maps{ }, m_texture_unit_counter{ } 
{
	int i{ 0 };
	for (const auto& a_texture_map : a_texture_maps) {
		m_texture_unit_counter.push_back(i);
		switch (a_texture_map.second) {
			case TextureType::DIFFUSE: {
				m_diffuse_maps.push_back(std::make_shared<Texture>(a_texture_map.first, a_texture_map.second, i++));
				break;
			}
			case TextureType::SPECULAR: {
				m_specular_maps.push_back(std::make_shared<Texture>(a_texture_map.first, a_texture_map.second, i++));
				break;
			}
			case TextureType::EMISSION: {
				m_emission_maps.push_back(std::make_shared<Texture>(a_texture_map.first, a_texture_map.second, i++));
				break;
			}
			case TextureType::NONE: {
				break;
			}
		}
	}
}

MaterialMap::MaterialMap(const MaterialMap& a_material_map) {
	m_shininess = a_material_map.m_shininess;
	for (const auto& diffuse_map : a_material_map.get_diffuse_maps())
		m_diffuse_maps.push_back(std::make_shared<Texture>(*diffuse_map));
	for (const auto& specular_map : a_material_map.get_specular_maps())
		m_specular_maps.push_back(std::make_shared<Texture>(*specular_map));
	for (const auto& emission_map : a_material_map.get_emission_maps())
		m_emission_maps.push_back(std::make_shared<Texture>(*emission_map));
	m_texture_unit_counter = a_material_map.m_texture_unit_counter;
}

MaterialMap& MaterialMap::operator=(const MaterialMap& a_material_map) {
	m_diffuse_maps.clear();
	m_specular_maps.clear();
	m_emission_maps.clear();

	for (const auto& diffuse_map : a_material_map.get_diffuse_maps())
		m_diffuse_maps.push_back(std::make_shared<Texture>(*diffuse_map));
	for (const auto& specular_map : a_material_map.get_specular_maps())
		m_specular_maps.push_back(std::make_shared<Texture>(*specular_map));
	for (const auto& emission_map : a_material_map.get_emission_maps())
		m_emission_maps.push_back(std::make_shared<Texture>(*emission_map));
	m_texture_unit_counter = a_material_map.m_texture_unit_counter;

	m_shininess = a_material_map.m_shininess;

	return *this;
}

// FIXME: Fragmentation in m_texture_unit_counter
void MaterialMap::set_diffuse_maps(std::vector<std::string> a_diffuse_maps_names) {
	for (const auto m_diffuse_map : m_diffuse_maps)
		m_texture_unit_counter.erase(std::find(m_texture_unit_counter.begin(), m_texture_unit_counter.end(), m_diffuse_map->get_texture_id()));
	m_diffuse_maps.clear();

	int max_id = *std::max_element(m_texture_unit_counter.begin(), m_texture_unit_counter.end()) + 1;

	for (int i = 0; i < a_diffuse_maps_names.size(); i++) {
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

	for (int i = 0; i < a_specular_maps_names.size(); i++) {
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

	for (int i = 0; i < a_emission_maps_names.size(); i++) {
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

VBO_COLLECTION::VBO_COLLECTION(std::vector<glm::vec3> a_pos, std::vector<glm::vec3> a_normals, std::vector<glm::vec2> a_textures, std::vector<glm::vec3> a_indicies) 
	:m_VBO_positions{ VBO_generate(a_pos) }, m_VBO_normals{ VBO_generate(a_normals) }, m_VBO_texture_coords{ VBO_generate(a_textures) },
	m_VBO_indicies{ VBO_generate(a_indicies) }, m_vert_sum{ a_pos.size() } { }

VBO_COLLECTION::~VBO_COLLECTION() {
	// glDeleteBuffers(1, &VERTS);
	// glDeleteBuffers(1, &TEXTURE);
	// glDeleteBuffers(1, &INDICIES);
	// glDeleteBuffers(1, &NORMALS);
}

VBO_CUBE::VBO_CUBE() :VBO_COLLECTION(CUBE_POSITIONS, CUBE_NORMAL_COORDS, CUBE_TEXTURE_COORDS) { }

int VBO_COLLECTION::get_vert_sum() const {
	return m_vert_sum;
}

unsigned int VBO_COLLECTION::VBO_normals() const {
	return m_VBO_normals;
}

unsigned int VBO_COLLECTION::VBO_indicies() const {
	return m_VBO_indicies;
}

unsigned int VBO_COLLECTION::VBO_positions() const {
	return m_VBO_positions;
}

unsigned int VBO_COLLECTION::VBO_texture_coords() const {
	return m_VBO_texture_coords;
}


Buffer_data::Buffer_data(BufferType a_type)
	:buffer_type{ a_type } { }

Buffer_data::Buffer_data(Buffer_data&& a_data) {
	normals = std::move(a_data.normals);
	positions = std::move(a_data.positions);
	texture_coords = std::move(a_data.texture_coords);
	indicies = std::move(a_data.indicies);
	buffer_type = a_data.buffer_type;
}

Buffer_data::Buffer_data(const Buffer_data& a_data) {
	normals = a_data.normals;
	positions = a_data.positions;
	texture_coords = a_data.texture_coords;
	indicies    = a_data.indicies;
	buffer_type = a_data.buffer_type;
}

Mesh::Mesh(const Buffer_data& a_data, bool a_wireframe)
	:m_data{ a_data }, m_wireframe{ a_wireframe }, m_VBO_pos{ EMPTY_VBO },
	 m_VBO_texture_coords{ EMPTY_VBO }, m_VBO_normal{ EMPTY_VBO }, m_EBO{ EMPTY_VBO }
{
	glGenVertexArrays(1, &m_VAO);

	if (!m_data.positions.empty()) {
		set_pos();
	}

	if (!m_data.normals.empty()) {
		set_normals();
	}

	if (!m_data.texture_coords.empty()) {
		set_texture_coords();
	}

	if (m_data.buffer_type == BufferType::ELEMENT && !m_data.indicies.empty()) {
		set_elements();
	}
}

Mesh::Mesh(BufferType a_buf_type, const VBO_COLLECTION& a_VBOs, bool a_wireframe)
	:m_data{ a_buf_type }, m_wireframe{ a_wireframe }, m_VBO_pos{ EMPTY_VBO },
	 m_VBO_texture_coords{ EMPTY_VBO }, m_VBO_normal{ EMPTY_VBO }, m_EBO{ EMPTY_VBO }
{
	glGenVertexArrays(1, &m_VAO);

	m_data.vert_sum = a_VBOs.get_vert_sum();

	reuse_pos(a_VBOs.VBO_positions());
	reuse_normals(a_VBOs.VBO_normals());
	reuse_texture_coords(a_VBOs.VBO_texture_coords());
	if (a_buf_type == BufferType::ELEMENT)
		reuse_elements(a_VBOs.VBO_indicies());
}

Mesh::Mesh(const Mesh& a_mesh) :Mesh(a_mesh.m_data, a_mesh.m_wireframe) { };

void Mesh::set_pos(const std::vector<glm::vec3>& a_positions) {
	if (!a_positions.empty())
		m_data.positions = a_positions;

	m_data.vert_sum = m_data.positions.size();

	glBindVertexArray(m_VAO);

	if (m_VBO_pos == EMPTY_VBO) 
		glGenBuffers(1, &m_VBO_pos);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO_pos);
	glBufferData(GL_ARRAY_BUFFER, m_data.positions.size() * sizeof(m_data.positions[0]), m_data.positions.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(ATTRIB_POS_LOCATION, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(ATTRIB_POS_LOCATION);
}


void Mesh::set_normals(const std::vector<glm::vec3>& a_normals) {
	if (!a_normals.empty())
		m_data.normals = a_normals;

	glBindVertexArray(m_VAO);

	if (m_VBO_normal == EMPTY_VBO) 
		glGenBuffers(1, &m_VBO_normal);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO_normal);
	glBufferData(GL_ARRAY_BUFFER, m_data.normals.size() * sizeof(m_data.normals[0]), m_data.normals.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(ATTRIB_NORMAL_LOCATION, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(ATTRIB_NORMAL_LOCATION);
}

void Mesh::set_texture_coords(const std::vector<glm::vec2>& a_texture_coords) {
	if (!a_texture_coords.empty())
		m_data.texture_coords = a_texture_coords;

	glBindVertexArray(m_VAO);

	if (m_VBO_texture_coords == EMPTY_VBO)
		glGenBuffers(1, &m_VBO_texture_coords);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO_texture_coords);
	glBufferData(GL_ARRAY_BUFFER, m_data.texture_coords.size() * sizeof(m_data.texture_coords[0]), m_data.texture_coords.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(ATTRIB_TEX_LOCATION, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(ATTRIB_TEX_LOCATION);
}

void Mesh::set_elements(const std::vector<unsigned int>& a_elem_indicies) {
	glBindVertexArray(m_VAO);

	if (!a_elem_indicies.empty())
		m_data.indicies = a_elem_indicies;

	if (m_EBO == EMPTY_VBO)
		glGenBuffers(1, &m_EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_data.indicies.size() * sizeof(m_data.indicies[0]), m_data.indicies.data(), GL_STATIC_DRAW);
}

void Mesh::reuse_pos(unsigned int a_VBO_pos) {
	if (a_VBO_pos == EMPTY_VBO)
		return;

	m_data.positions.clear();

	m_VBO_pos = EMPTY_VBO;

	glBindVertexArray(m_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, a_VBO_pos);

	glVertexAttribPointer(ATTRIB_POS_LOCATION, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(ATTRIB_POS_LOCATION);
}

void Mesh::reuse_normals(unsigned int a_VBO_normal) {
	if (a_VBO_normal == EMPTY_VBO)
		return;

	m_data.normals.clear();

	m_VBO_normal = EMPTY_VBO;

	glBindVertexArray(m_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, a_VBO_normal);

	glVertexAttribPointer(ATTRIB_NORMAL_LOCATION, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(ATTRIB_NORMAL_LOCATION);
}

void Mesh::reuse_texture_coords(unsigned int a_VBO_texture_coords) {
	if (a_VBO_texture_coords == EMPTY_VBO)
		return;

	m_data.texture_coords.clear();

	m_VBO_texture_coords = EMPTY_VBO;

	glBindVertexArray(m_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, a_VBO_texture_coords);

	glVertexAttribPointer(ATTRIB_TEX_LOCATION, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(ATTRIB_TEX_LOCATION);
}

void Mesh::reuse_elements(unsigned int a_EBO) {
	if (a_EBO == EMPTY_VBO)
		return;

	m_data.indicies.clear();

	m_EBO = EMPTY_VBO;

	glBindVertexArray(m_VAO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, a_EBO);
}

void Mesh::update_pos() {
	if (m_data.positions.empty()) return;

	glBindBuffer(GL_ARRAY_BUFFER, m_VBO_pos);
	glBufferData(GL_ARRAY_BUFFER, m_data.positions.size() * sizeof(m_data.positions[0]), m_data.positions.data(), GL_STATIC_DRAW);
}

void Mesh::update_texture_coords() {
	if (m_data.texture_coords.empty()) 
		return;

	glBindBuffer(GL_ARRAY_BUFFER, m_VBO_texture_coords);
	glBufferData(GL_ARRAY_BUFFER, m_data.texture_coords.size() * sizeof(m_data.texture_coords[0]), m_data.texture_coords.data(), GL_STATIC_DRAW);
}

void Mesh::draw_vertices() {
	glBindVertexArray(m_VAO);

	if (m_wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	switch(m_data.buffer_type) {
		case (BufferType::VERTEX) : {
			glDrawArrays(GL_TRIANGLES, 0, m_data.vert_sum);
			break;
		}
		case (BufferType::ELEMENT) : {
			glDrawElements(GL_TRIANGLES, m_data.indicies.size(), GL_UNSIGNED_INT, 0);
			break;
		}
		default : {
			ERROR("Unhandled draw type for buffer object type.");
			throw Error_code::bad_match;
		}
	}

	glBindVertexArray(0);
}

unsigned int Mesh::get_VBO_pos() const {
	return m_VBO_pos;
}

unsigned int Mesh::get_VBO_texture_coords() const {
	return m_VBO_texture_coords;
}

unsigned int Mesh::get_VBO_indicies() const {
	return m_EBO;
}

Mesh::~Mesh() {
	glDeleteVertexArrays(1, &m_VAO);
	glDeleteBuffers(1, &m_VBO_pos);
	glDeleteBuffers(1, &m_VBO_normal);
	glDeleteBuffers(1, &m_VBO_texture_coords);
	glDeleteBuffers(1, &m_EBO);
}

/* DEPRACATED

void Mesh::set_color(const std::vector<float>& a_color) {
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

void Mesh::reuse_color(unsigned int a_VBO_color) {
	if (a_VBO_color == EMPTY_VBO)
		return;

	m_data.colors.clear();

	m_VBO_color = EMPTY_VBO;

	glBindVertexArray(m_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, a_VBO_color);

	glVertexAttribPointer(ATTRIB_COLOR_LOCATION, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(ATTRIB_COLOR_LOCATION);
}

void Mesh::update_color() {
	if (m_data.colors.empty()) return;

	glBindBuffer(GL_ARRAY_BUFFER, m_VBO_color);
	glBufferData(GL_ARRAY_BUFFER, data.colors.size() * sizeof(float), data.colors.data(), GL_STATIC_DRAW);
}

*/
