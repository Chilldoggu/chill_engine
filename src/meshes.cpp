#include "chill_engine/meshes.hpp"
#include "chill_engine/assert.hpp"
#include "chill_engine/file_manager.hpp"
#include "chill_engine/application.hpp"

namespace chill_engine { 
MaterialMap::MaterialMap(const std::initializer_list<std::tuple<std::wstring,TextureType,bool>>& a_texture_maps) {
	ResourceManager& rman = Application::get_instance().get_rmanager();

	for (const auto& a_texture_map : a_texture_maps) {
		std::wstring texture_path = std::get<0>(a_texture_map);
		TextureType texture_type = std::get<1>(a_texture_map);
		bool texture_flip = std::get<2>(a_texture_map);

		switch (texture_type) {
		case TextureType::DIFFUSE:
			m_diffuse_maps.push_back(rman.load_texture(texture_path, texture_type, texture_flip, m_cur_diffuse_unit_id++));
			break;
		case TextureType::SPECULAR:
			m_specular_maps.push_back(rman.load_texture(texture_path, texture_type, texture_flip, m_cur_specular_unit_id++));
			break;
		case TextureType::EMISSION:
			m_emission_maps.push_back(rman.load_texture(texture_path, texture_type, texture_flip, m_cur_emission_unit_id++));
			break;
		case TextureType::NONE:
			ERROR(std::format("[MATERIALMAP::MATERIALMAP] Bad texture type for {}", wstos(texture_path)), Error_action::throwing);
		}
	}
	check_unit_id_limits();
}

void MaterialMap::set_textures(const std::vector<Texture>& a_textures) { 
	m_diffuse_maps.clear();
	m_specular_maps.clear();
	m_emission_maps.clear();

	int max_diffuse_unit_id = g_diffuse_unit_id;
	int max_specular_unit_id = g_specular_unit_id;
	int max_emission_unit_id = g_emission_unit_id;

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

	m_cur_diffuse_unit_id = max_diffuse_unit_id;
	m_cur_specular_unit_id = max_specular_unit_id;
	m_cur_emission_unit_id = max_emission_unit_id;

	check_unit_id_limits();
}

void MaterialMap::set_diffuse_maps(const std::vector<std::tuple<std::wstring,bool>>& a_diffuse_maps_names) {
	ResourceManager& rman = Application::get_instance().get_rmanager();

	m_diffuse_maps.clear();
	m_cur_diffuse_unit_id = g_diffuse_unit_id;

	for (auto& diffuse_map : a_diffuse_maps_names) {
		m_diffuse_maps.push_back(rman.load_texture(std::get<0>(diffuse_map), TextureType::DIFFUSE, std::get<1>(diffuse_map), m_cur_diffuse_unit_id++));
	}
	check_unit_id_limits();
}

void MaterialMap::set_specular_maps(const std::vector<std::tuple<std::wstring,bool>>& a_specular_maps_names) {
	ResourceManager& rman = Application::get_instance().get_rmanager();

	m_specular_maps.clear();
	m_cur_specular_unit_id = g_specular_unit_id;

	for (auto& specular_map : a_specular_maps_names) {
		m_specular_maps.push_back(rman.load_texture(std::get<0>(specular_map), TextureType::SPECULAR, std::get<1>(specular_map), m_cur_specular_unit_id++));
	}
	check_unit_id_limits();
}

void MaterialMap::set_emission_maps(const std::vector<std::tuple<std::wstring,bool>>& a_emission_maps_names) {
	ResourceManager& rman = Application::get_instance().get_rmanager();

	m_emission_maps.clear();
	m_cur_emission_unit_id = g_emission_unit_id;

	for (auto& emission_map : a_emission_maps_names) {
		m_emission_maps.push_back(rman.load_texture(std::get<0>(emission_map), TextureType::EMISSION, std::get<1>(emission_map), m_cur_emission_unit_id++));
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
	auto dif_siz = g_diffuse_unit_id - m_cur_diffuse_unit_id;
	auto spec_siz = g_specular_unit_id - m_cur_specular_unit_id;
	auto em_siz = g_emission_unit_id - m_cur_emission_unit_id;

	if (dif_siz >= g_max_sampler_siz || spec_siz >= g_max_sampler_siz || em_siz >= g_max_sampler_siz) {
		ERROR(std::format("[MATERIALMAP::CHECK_UNIT_ID_LIMITS] Unit ids exceeded limit of sampler size {}. DIFFUSE: {}, SPECULAR: {}, EMISSION: {}\n", g_max_sampler_siz, dif_siz, spec_siz, em_siz), Error_action::throwing);
	}
}

Mesh::Mesh(const BufferData& a_data, const MaterialMap& a_mat, bool a_wireframe)
	:m_material_map{ a_mat }, m_wireframe{ a_wireframe }
{
	glGenVertexArrays(1, &m_VBOs.VAO);
	Application::get_instance().get_rmanager().inc_ref_count(ResourceType::MESHES, m_VBOs.VAO);

	m_type = (a_data.indicies.empty()) ? BufferDataType::VERTEX : BufferDataType::ELEMENT;

	set_positions(a_data.positions);
	set_UVs(a_data.UVs);
	set_normals(a_data.normals);
	set_indicies(a_data.indicies);
}

void Mesh::set_positions(const std::vector<glm::vec3>& a_positions) {
	m_verticies_sum = a_positions.size();

	glBindVertexArray(m_VBOs.VAO);

	if (m_VBOs.VBO_pos == EMPTY_VBO)
		glGenBuffers(1, &m_VBOs.VBO_pos);

	glBindBuffer(GL_ARRAY_BUFFER, m_VBOs.VBO_pos);
	glBufferData(GL_ARRAY_BUFFER, m_verticies_sum * sizeof(a_positions[0]), a_positions.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(g_attrib_pos_location, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(g_attrib_pos_location);
}

void Mesh::set_UVs(const std::vector<glm::vec2>& a_UVs) {
	glBindVertexArray(m_VBOs.VAO);

	if (m_VBOs.VBO_UVs == EMPTY_VBO)
		glGenBuffers(1, &m_VBOs.VBO_UVs);

	glBindBuffer(GL_ARRAY_BUFFER, m_VBOs.VBO_UVs);
	glBufferData(GL_ARRAY_BUFFER, a_UVs.size() * sizeof(a_UVs[0]), a_UVs.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(g_attrib_tex_location, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(g_attrib_tex_location);
}

void Mesh::set_normals(const std::vector<glm::vec3>& a_normals) {
	glBindVertexArray(m_VBOs.VAO);

	if (m_VBOs.VBO_normals == EMPTY_VBO)
		glGenBuffers(1, &m_VBOs.VBO_normals);

	glBindBuffer(GL_ARRAY_BUFFER, m_VBOs.VBO_normals);
	glBufferData(GL_ARRAY_BUFFER, a_normals.size() * sizeof(a_normals[0]), a_normals.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(g_attrib_normal_location, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(g_attrib_normal_location);
}

void Mesh::set_indicies(const std::vector<unsigned int>& a_indicies) {
	if (m_type != BufferDataType::ELEMENT)
		return;

	glBindVertexArray(m_VBOs.VAO);

	m_indicies_sum = a_indicies.size();

	if (m_VBOs.EBO == EMPTY_VBO)
		glGenBuffers(1, &m_VBOs.EBO);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_VBOs.EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, a_indicies.size() * sizeof(a_indicies[0]), a_indicies.data(), GL_STATIC_DRAW);
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
	if (m_visibility) {
		glBindVertexArray(m_VBOs.VAO);

		if (m_wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		else glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		switch (m_type) {
		case BufferDataType::VERTEX:  glDrawArrays(GL_TRIANGLES, 0, m_verticies_sum); break;
		case BufferDataType::ELEMENT: glDrawElements(GL_TRIANGLES, m_indicies_sum, GL_UNSIGNED_INT, 0); break;
		default:
			ERROR("[MESH::DRAW] Unhandled draw type for buffer object type.", Error_action::throwing);
		}

		glBindVertexArray(0);
	}
}

GLuint Mesh::get_VAO() const {
	return m_VBOs.VAO;
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
}