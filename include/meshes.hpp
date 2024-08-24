#pragma once

#include "glm/glm.hpp"
#include "glad/glad.h"

#include <memory>
#include <vector>
#include <string>
#include <filesystem>

#define EMPTY_VBO 0
#define ATTRIB_POS_LOCATION    0
#define ATTRIB_COLOR_LOCATION  1
#define ATTRIB_TEX_LOCATION    2
#define ATTRIB_NORMAL_LOCATION 3
 
#define MAX_SAMPLER_SIZ  16
#define DIFFUSE_UNIT_ID  0
#define SPECULAR_UNIT_ID 16
#define EMISSION_UNIT_ID 32

class ShaderProgram;

std::filesystem::path get_asset_path();
std::filesystem::path get_proj_path();

enum class TextureType {
	DIFFUSE,
	SPECULAR,
	EMISSION,

	COLOR,
	DEPTH,
	STENCIL,
	DEPTH_STENCIL,

    NONE,
};

std::ostream& operator<<(std::ostream& os, const TextureType& a_type);

class Texture {
public:
	// Load texture from filesystem
    Texture(std::wstring a_dir, TextureType a_type, int texture_unit);
	// Load FBO textures
    Texture(int a_width, int a_height, TextureType a_type);
    ~Texture();

	auto clear() -> void;
    auto load_texture(std::wstring a_name, TextureType a_type, int a_texture_unit, bool a_flip_UVs = true) -> void;
	auto gen_FBO_texture(int a_viewport_w, int a_viewport_h, TextureType a_type) -> void;
	auto set_texture_unit(int a_unit_id) -> void;
	auto set_texture_type(TextureType a_type) -> void;
    auto activate() const -> void; 

    auto get_dir() const -> std::wstring;
    auto get_type() const -> TextureType;
    auto get_texture_id() const -> unsigned int;
	auto get_texture_unit() const -> int;

private:
    int m_texture_unit = 0;
    TextureType m_type = TextureType::NONE;
    std::wstring m_dir = L"";
    unsigned int m_texture_id = 0;
};

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

	auto set_pos(const std::vector<glm::vec3>& a_pos = {}) -> void;
	auto set_normals(const std::vector<glm::vec3>& a_normals = {}) -> void;
	auto set_elements(const std::vector<unsigned int>& a_elem_indicies = {}) -> void;
	auto set_material_map(const MaterialMap& a_material_map) -> void;
	auto set_texture_coords(const std::vector<glm::vec2>& a_texture_coords = {}) -> void;
	auto set_wireframe(bool a_option) -> void;
	auto set_visibility(bool a_option) -> void;

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
