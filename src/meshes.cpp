#include <algorithm>
#include <memory>
#include <string>
#include <utility>

#include <filesystem>
#include <format>
#include <tuple>

#include "meshes.hpp"
#include "assert.hpp"
#include "shaders.hpp"
#include "buffers.hpp"
#include "assert.hpp"
#include "file_manager.hpp"
 
MaterialMap::MaterialMap(std::initializer_list<std::tuple<std::wstring, TextureType, bool>> a_texture_maps, float a_shininess) 
	:m_shininess{ a_shininess }
{
	for (const auto& a_texture_map : a_texture_maps) {
		std::wstring texture_dir = std::get<0>(a_texture_map);
		TextureType texture_type = std::get<1>(a_texture_map);
		bool texture_flip        = std::get<2>(a_texture_map);

		switch (texture_type) {
			case TextureType::DIFFUSE:
				m_diffuse_maps.push_back(Texture(texture_dir, texture_type, texture_flip, m_cur_diffuse_unit_id++));
				break;
			case TextureType::SPECULAR:
				m_specular_maps.push_back(Texture(texture_dir, texture_type, texture_flip, m_cur_specular_unit_id++));
				break;
			case TextureType::EMISSION:
				m_emission_maps.push_back(Texture(texture_dir, texture_type, texture_flip, m_cur_emission_unit_id++));
				break;
			case TextureType::NONE:
				ERROR(std::format("[MATERIALMAP::MATERIALMAP] Bad texture type for {}", wstos(texture_dir)), Error_action::throwing);
		} 
	}
	check_unit_id_limits();
}

void MaterialMap::set_textures(std::vector<Texture> a_textures) {
	m_diffuse_maps.clear();
	m_specular_maps.clear();
	m_emission_maps.clear();

	int max_diffuse_unit_id  = DIFFUSE_UNIT_ID;
	int max_specular_unit_id = SPECULAR_UNIT_ID;
	int max_emission_unit_id = EMISSION_UNIT_ID;

	for (const auto& texture : a_textures) {
		int unit_id = texture.get_unit_id(); 

		switch (texture.get_type()) {
			case TextureType::DIFFUSE:
				m_diffuse_maps.push_back(texture);
				if (unit_id > max_diffuse_unit_id)
					max_diffuse_unit_id = unit_id;
				break;
			case TextureType::SPECULAR:
				m_specular_maps.push_back(texture);
				if (unit_id > max_specular_unit_id)
					max_specular_unit_id = unit_id;
				break;
			case TextureType::EMISSION:
				m_emission_maps.push_back(texture);
				if (unit_id > max_emission_unit_id)
					max_emission_unit_id = unit_id;
				break;
			case TextureType::NONE:
				break;
		}
	}
 
	m_cur_diffuse_unit_id  = max_diffuse_unit_id;
	m_cur_specular_unit_id = max_specular_unit_id;
	m_cur_emission_unit_id = max_emission_unit_id;

	check_unit_id_limits();
}

// FIXME: Fragmentation in m_texture_unit_counter
void MaterialMap::set_diffuse_maps(std::vector<std::tuple<std::wstring, bool>> a_diffuse_maps_names) {
	m_diffuse_maps.clear();
	m_cur_diffuse_unit_id = DIFFUSE_UNIT_ID; 

	for (auto& diffuse_map : a_diffuse_maps_names) {
		m_diffuse_maps.push_back(Texture(std::get<0>(diffuse_map), TextureType::DIFFUSE, std::get<1>(diffuse_map), m_cur_diffuse_unit_id++));
	} 
	check_unit_id_limits();
}

// FIXME: Fragmentation in m_texture_unit_counter
void MaterialMap::set_specular_maps(std::vector<std::tuple<std::wstring, bool>> a_specular_maps_names) {
	m_specular_maps.clear();
	m_cur_specular_unit_id = SPECULAR_UNIT_ID; 

	for (auto& specular_map : a_specular_maps_names) {
		m_specular_maps.push_back(Texture(std::get<0>(specular_map), TextureType::SPECULAR,std::get<1>(specular_map), m_cur_specular_unit_id++));
	} 
	check_unit_id_limits();
}

// FIXME: Fragmentation in m_texture_unit_counter
void MaterialMap::set_emission_maps(std::vector<std::tuple<std::wstring, bool>> a_emission_maps_names) {
	m_emission_maps.clear();
	m_cur_emission_unit_id = EMISSION_UNIT_ID; 

	for (auto& emission_map : a_emission_maps_names) {
		m_emission_maps.push_back(Texture(std::get<0>(emission_map), TextureType::EMISSION, std::get<1>(emission_map), m_cur_emission_unit_id++));
	} 
	check_unit_id_limits();
}

void MaterialMap::set_shininess(float a_shininess) {
	m_shininess = a_shininess;
}

std::vector<Texture> MaterialMap::get_diffuse_maps() const {
	return m_diffuse_maps;
}

std::vector<Texture> MaterialMap::get_specular_maps() const {
	return m_specular_maps;
}

std::vector<Texture> MaterialMap::get_emission_maps() const {
	return m_emission_maps;
}

float MaterialMap::get_shininess() const {
	return m_shininess;
}

void MaterialMap::check_unit_id_limits() const {
	auto dif_siz  = DIFFUSE_UNIT_ID - m_cur_diffuse_unit_id;
	auto spec_siz = SPECULAR_UNIT_ID - m_cur_specular_unit_id;
	auto em_siz   = EMISSION_UNIT_ID - m_cur_emission_unit_id;

	if (dif_siz >= MAX_SAMPLER_SIZ || spec_siz >= MAX_SAMPLER_SIZ || em_siz >= MAX_SAMPLER_SIZ) {
		ERROR(std::format("[MATERIALMAP::CHECK_UNIT_ID_LIMITS] Unit ids exceeded limit of sampler size {}. DIFFUSE: {}, SPECULAR: {}, EMISSION: {}\n", MAX_SAMPLER_SIZ, dif_siz, spec_siz, em_siz), Error_action::throwing);
	}
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

void Mesh::set_wireframe(bool a_option) {
	m_wireframe = a_option;
}

void Mesh::set_visibility(bool a_option) {
	m_visibility = a_option;
}


void Mesh::draw() {
	// Optimization: Shader is being used from Model object draw method to
	// reduce redundant glEnable()/glDisable() calls to OpenGL on every single mesh.
	// a_shader.use();

	if (m_visibility) {
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
				ERROR("[MESH::DRAW] Unhandled draw type for buffer object type.", Error_action::throwing);
		}

		glBindVertexArray(0); 
	}
}

unsigned int Mesh::get_VAO() {
	return m_VBOs->m_VAO;
}

MaterialMap& Mesh::get_material_map() {
	return m_material_map;
}
 
bool Mesh::get_wireframe() const {
	return m_wireframe;
}

bool Mesh::get_visibility() const {
	return m_visibility;
}
