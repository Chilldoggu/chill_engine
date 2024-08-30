#pragma once
#include "glm/glm.hpp"

#include <memory>
#include <vector>
#include <string>
#include <filesystem>
#include <tuple>

#include "chill_engine/buffers.hpp"

constexpr int MAX_SAMPLER_SIZ  = 16;
constexpr int DIFFUSE_UNIT_ID  = 0 * MAX_SAMPLER_SIZ;
constexpr int SPECULAR_UNIT_ID = 1 * MAX_SAMPLER_SIZ;
constexpr int EMISSION_UNIT_ID = 2 * MAX_SAMPLER_SIZ;

class MaterialMap {
public:
    MaterialMap(std::initializer_list<std::tuple<std::wstring, TextureType, bool>> a_texture_maps = {}, float a_shininess = 32.f);

	auto set_textures(std::vector<Texture>& a_textures) -> void;
	auto set_diffuse_maps(std::vector<std::tuple<std::wstring, bool>> a_diffuse_map) -> void;
	auto set_specular_maps(std::vector<std::tuple<std::wstring, bool>> a_specular_map) -> void;
	auto set_emission_maps(std::vector<std::tuple<std::wstring, bool>> a_emission_map) -> void;
	auto set_shininess(float a_shininess) -> void;

	auto get_diffuse_maps() const -> std::vector<Texture>;
	auto get_specular_maps() const -> std::vector<Texture>;
	auto get_emission_maps() const -> std::vector<Texture>;
	auto get_shininess() const -> float;

private:
	auto check_unit_id_limits() const -> void;

    float m_shininess = 32.f;
	int m_cur_diffuse_unit_id{ DIFFUSE_UNIT_ID };
	int m_cur_specular_unit_id{ SPECULAR_UNIT_ID };
	int m_cur_emission_unit_id{ EMISSION_UNIT_ID };
	std::vector<Texture> m_diffuse_maps;
	std::vector<Texture> m_specular_maps;
	std::vector<Texture> m_emission_maps;
};

enum class BufferDataType {
	VERTEX,
	ELEMENT,
	NONE
};

struct BufferData { 
	std::vector<glm::vec3> normals = {};
	std::vector<glm::vec3> positions = {};
	std::vector<glm::vec2> UVs = {};
	std::vector<unsigned int> indicies = {};
};

class Mesh {
public:
	Mesh() = default;
	Mesh(const BufferData& a_data, const MaterialMap& a_mat, bool a_wireframe = false);

	auto draw() -> void;

	auto set_positions(const std::vector<glm::vec3>& a_pos) -> void;
	auto set_UVs(const std::vector<glm::vec2>& a_UVs) -> void;
	auto set_normals(const std::vector<glm::vec3>& a_normals) -> void;
	auto set_indicies(const std::vector<unsigned int>& a_elem_indicies) -> void; 
	auto set_material_map(const MaterialMap& a_material_map) -> void;

	auto set_wireframe(bool a_option) -> void;
	auto set_visibility(bool a_option) -> void;

	auto get_VAO() const -> unsigned;
	auto get_wireframe() const -> bool;
	auto get_visibility() const -> bool;
	auto get_material_map() -> MaterialMap&;

private:
	bool m_wireframe = false;
	bool m_visibility = true;
	int m_verticies_sum = 0;
	int m_indicies_sum = 0;
	BufferObjects m_VBOs;
	MaterialMap m_material_map;
	BufferDataType m_type = BufferDataType::NONE;
};
