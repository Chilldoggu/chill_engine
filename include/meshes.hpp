#pragma once

#include "glm/glm.hpp"
#include "glad/glad.h"

#include <memory>
#include <vector>
#include <string>
#include <filesystem>

#include "buffers.hpp"

constexpr int MAX_SAMPLER_SIZ  = 16;
constexpr int DIFFUSE_UNIT_ID  = 0 * MAX_SAMPLER_SIZ;
constexpr int SPECULAR_UNIT_ID = 1 * MAX_SAMPLER_SIZ;
constexpr int EMISSION_UNIT_ID = 2 * MAX_SAMPLER_SIZ;

class MaterialMap {
public:
    MaterialMap(std::initializer_list<std::pair<std::wstring, TextureType>> a_texture_maps = {}, float a_shininess = 32.f);

	auto set_textures(std::vector<std::shared_ptr<Texture>> a_textures) -> void;
	auto set_diffuse_maps(std::vector<std::wstring> a_diffuse_map) -> void;
	auto set_specular_maps(std::vector<std::wstring> a_specular_map) -> void;
	auto set_emission_maps(std::vector<std::wstring> a_emission_map) -> void;
	auto set_shininess(float a_shininess) -> void;

	auto get_diffuse_maps() const -> std::vector<std::shared_ptr<Texture>>;
	auto get_specular_maps() const -> std::vector<std::shared_ptr<Texture>>;
	auto get_emission_maps() const -> std::vector<std::shared_ptr<Texture>>;
	auto get_shininess() const -> float;

private:
	auto check_unit_id_limits() const -> void;

    float m_shininess = 32.f;
	int m_cur_diffuse_unit_id{ DIFFUSE_UNIT_ID };
	int m_cur_specular_unit_id{ SPECULAR_UNIT_ID };
	int m_cur_emission_unit_id{ EMISSION_UNIT_ID };
	std::vector<std::shared_ptr<Texture>> m_diffuse_maps;
	std::vector<std::shared_ptr<Texture>> m_specular_maps;
	std::vector<std::shared_ptr<Texture>> m_emission_maps;
};

template<typename T>
unsigned int constexpr VBO_generate(std::vector<T> data) {
	if (data.empty())
		return EMPTY_VBO;
    unsigned int VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(T), data.data(), GL_STATIC_DRAW);
    return VBO;
}

struct BufferData {
	enum class Type {
		VERTEX,
		ELEMENT,
		NONE
	};

	int vert_sum = 0;
	int indicies_sum = 0;
	Type buffer_type;
	std::vector<glm::vec3> normals = {};
	std::vector<glm::vec3> positions = {};
	std::vector<glm::vec2> texture_coords = {};
	std::vector<unsigned int> indicies = {};

	BufferData(Type a_type = Type::NONE);
};

class Mesh {
private:
	struct BufferObjects {
		BufferObjects();
		~BufferObjects();

		unsigned int m_VAO;
		unsigned int VBO_pos;
		unsigned int VBO_texture_coords;
		unsigned int VBO_normal;
		unsigned int EBO;
	};

public:
	Mesh();
	Mesh(const BufferData& a_data, const MaterialMap& a_mat, bool a_wireframe = false);

	auto draw() -> void;
	auto gen_VAO() -> void;

	auto clear() -> void;
	auto set_pos(const std::vector<glm::vec3>& a_pos = {}) -> void;
	auto set_normals(const std::vector<glm::vec3>& a_normals = {}) -> void;
	auto set_elements(const std::vector<unsigned int>& a_elem_indicies = {}) -> void;
	auto set_material_map(const MaterialMap& a_material_map) -> void;
	auto set_texture_coords(const std::vector<glm::vec2>& a_texture_coords = {}) -> void;
	auto set_wireframe(bool a_option) -> void;
	auto set_visibility(bool a_option) -> void;

	auto get_VAO() -> unsigned int;
	auto get_material_map() -> MaterialMap&;
	auto get_wireframe() -> bool;
	auto get_visibility() -> bool;

private:
	auto set_type(BufferData::Type a_type) -> void;

	bool m_wireframe = false;
	bool m_visibility = true;
	BufferData m_data;
	MaterialMap m_material_map;
	std::shared_ptr<BufferObjects> m_VBOs;
};
