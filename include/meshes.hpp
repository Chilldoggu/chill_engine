#pragma once

#include "glm/glm.hpp"
#include "glad/glad.h"

#include <memory>
#include <vector>
#include <string>
#include <filesystem>

#define EMPTY_VBO 0
#define MAX_SAMPLER_SIZ 16
#define ATTRIB_POS_LOCATION    0
#define ATTRIB_COLOR_LOCATION  1
#define ATTRIB_TEX_LOCATION    2
#define ATTRIB_NORMAL_LOCATION 3

class ShaderProgram;

std::filesystem::path get_asset_path();

enum class TextureType {
	DIFFUSE,
	SPECULAR,
	EMISSION,
    NONE,
};

class Texture {
public:
    Texture(std::string a_name, TextureType a_type, int texture_unit);
    Texture();
    ~Texture();

    auto generate_texture(std::string a_name, TextureType a_type, int a_texture_unit) -> void;
    auto activate() const -> void;

    auto get_type() const -> TextureType;
    auto get_dir() const -> std::string;
    auto get_texture_id() const -> unsigned int;
	auto get_texture_unit() const -> int;

private:
    int m_texture_unit;
    TextureType m_type;
    std::string m_dir;
    unsigned int m_texture_id;
};

class MaterialMap {
public:
    MaterialMap(std::initializer_list<std::pair<std::string, TextureType>> a_texture_maps = {}, float a_shininess = 32.f);

	auto set_textures(std::vector<std::shared_ptr<Texture>> a_textures) -> void;
	auto set_diffuse_maps(std::vector<std::string> a_diffuse_map) -> void;
	auto set_specular_maps(std::vector<std::string> a_specular_map) -> void;
	auto set_emission_maps(std::vector<std::string> a_emission_map) -> void;
	auto set_shininess(float a_shininess) -> void;

	auto get_diffuse_maps() const -> std::vector<std::shared_ptr<Texture>>;
	auto get_specular_maps() const -> std::vector<std::shared_ptr<Texture>>;
	auto get_emission_maps() const -> std::vector<std::shared_ptr<Texture>>;
	auto get_shininess() const -> float;

private:
    float m_shininess;
	std::vector<int> m_texture_unit_counter;
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

	auto draw(ShaderProgram& a_shader) -> void;
	auto gen_VAO() -> void;

	auto set_pos(const std::vector<glm::vec3>& a_pos = {}) -> void;
	auto set_type(BufferData::Type a_type) -> void;
	auto set_normals(const std::vector<glm::vec3>& a_normals = {}) -> void;
	auto set_elements(const std::vector<unsigned int>& a_elem_indicies = {}) -> void;
	auto set_material_map(const MaterialMap& a_material_map) -> void;
	auto set_texture_coords(const std::vector<glm::vec2>& a_texture_coords = {}) -> void;

	auto get_material_map() -> MaterialMap&;

private:
	bool m_wireframe;
	BufferData m_data;
	MaterialMap m_material_map;
	std::shared_ptr<BufferObjects> m_VBOs;
};
