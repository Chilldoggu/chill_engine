#pragma once
#include "glm/glm.hpp"

#include <vector>
#include <tuple>

#include "chill_engine/buffers.hpp"

namespace chill_engine {
constexpr int g_attrib_pos_location = 0;
constexpr int g_attrib_tex_location = 1;
constexpr int g_attrib_normal_location = 2;
constexpr int g_attrib_color_location = 3;

constexpr int g_max_sampler_siz  = 16;
constexpr int g_diffuse_unit_id  = 0 * g_max_sampler_siz;
constexpr int g_specular_unit_id = 1 * g_max_sampler_siz;
constexpr int g_emission_unit_id = 2 * g_max_sampler_siz;

class MaterialMap {
public:
	MaterialMap(const std::initializer_list<std::tuple<std::wstring, TextureType, bool>>& a_texture_maps = {});

	auto set_textures(const std::vector<Texture>& a_textures) -> void;
	auto set_diffuse_maps(const std::vector<std::tuple<std::wstring,bool>>& a_diffuse_maps_names) -> void;
	auto set_specular_maps(const std::vector<std::tuple<std::wstring,bool>>& a_specular_maps_names) -> void;
	auto set_emission_maps(const std::vector<std::tuple<std::wstring,bool>>& a_emission_maps_names) -> void;
	auto set_shininess(float a_shininess) -> void;

	auto get_diffuse_maps() const -> std::vector<Texture>;
	auto get_specular_maps() const -> std::vector<Texture>;
	auto get_emission_maps() const -> std::vector<Texture>;
	auto get_shininess() const -> float;

private:
	auto check_unit_id_limits() const -> void;

	float m_shininess = 32.f;
	int m_cur_diffuse_unit_id{ g_diffuse_unit_id };
	int m_cur_specular_unit_id{ g_specular_unit_id };
	int m_cur_emission_unit_id{ g_emission_unit_id };
	std::vector<Texture> m_diffuse_maps;
	std::vector<Texture> m_specular_maps;
	std::vector<Texture> m_emission_maps;
};

enum class BufferDataType {
	VERTEX,
	ELEMENT,
	NONE
};

enum class BufferDrawType {
	POINTS,
	LINES,
	LINE_STRIP,
	LINE_LOOP,
	TRIANGLES,
	TRIANGLE_STRIP,
	TRIANGLE_FAN,
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

	auto set_draw_mode(BufferDrawType a_option) -> void;
	auto set_wireframe(bool a_option) -> void;
	auto set_visibility(bool a_option) -> void;

	auto get_VAO() const -> GLuint;
	auto get_draw_mode() const -> BufferDrawType;
	auto get_wireframe() const -> bool;
	auto get_visibility() const -> bool;
	auto get_material_map() -> MaterialMap&;

private:
	bool m_wireframe = false;
	bool m_visibility = true;
	int m_verticies_sum{};
	int m_indicies_sum{};
	MaterialMap m_material_map{};
	BufferObjects m_VBOs{};
	BufferDataType m_type = BufferDataType::NONE;
	BufferDrawType m_draw_mode = BufferDrawType::TRIANGLES;
}; 
}