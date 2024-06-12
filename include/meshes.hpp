#pragma once

#include <cstddef>
#include <glm/glm.hpp>
#include "glm/fwd.hpp"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <memory>
#include <vector>
#include <string>

#define EMPTY_VBO 0
#define MAX_SAMPLER_SIZ 16
#define ATTRIB_POS_LOCATION    0
#define ATTRIB_COLOR_LOCATION  1
#define ATTRIB_TEX_LOCATION    2
#define ATTRIB_NORMAL_LOCATION 3

enum class TextureType {
	DIFFUSE,
	SPECULAR,
	EMISSION,
    NONE,
};

class Texture {
public:
    Texture(std::string a_name, TextureType a_type, int texture_unit);
    Texture(const Texture& a_texture);
    Texture(Texture&& a_texture);
    Texture();
    ~Texture();

    Texture& operator=(const Texture& a_texture);
    Texture& operator=(Texture&& a_texture);

    auto generate_texture(std::string a_name, TextureType a_type, int a_texture_unit) -> void;
    auto activate() const -> void;

    auto get_type() const -> TextureType;
    auto get_name() const -> std::string;
    auto get_texture_id() const -> unsigned int;
	auto get_texture_unit() const -> int;

private:
    int m_texture_unit;
    bool m_deletable;
    TextureType m_type;
    std::string m_name;
    unsigned int m_texture_id;
};

/* struct Material {
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float shininess;

    Material();
    Material(glm::vec3 a_ambient, glm::vec3 a_diffuse, glm::vec3 a_specular, float a_shininess);
}; */

// Copy assignment means sharing pointer to a texture that CAN be DELETED but
// proves to be effficient when modyfying the original MaterialMap object.
class MaterialMap {
public:
    MaterialMap(std::initializer_list<std::pair<std::string, TextureType>> a_texture_maps = {}, float a_shininess = 32.f);
    MaterialMap(const MaterialMap& a_material_map);

	auto operator=(const MaterialMap& a_material_map) -> MaterialMap&;

	auto set_diffuse_maps(std::vector<std::string> a_diffuse_map = {}) -> void;
	auto set_specular_maps(std::vector<std::string> a_specular_map = {}) -> void;
	auto set_emission_maps(std::vector<std::string> a_emission_map = {}) -> void;
	auto set_shininess(float a_shininess) -> void;

	auto get_diffuse_maps() const -> std::vector<std::shared_ptr<Texture>>;
	auto get_specular_maps() const -> std::vector<std::shared_ptr<Texture>>;
	auto get_emission_maps() const -> std::vector<std::shared_ptr<Texture>>;
	auto get_shininess() const -> float;

private:
	std::vector<std::shared_ptr<Texture>> m_diffuse_maps;
	std::vector<std::shared_ptr<Texture>> m_specular_maps;
	std::vector<std::shared_ptr<Texture>> m_emission_maps;
    float m_shininess;
	std::vector<int> m_texture_unit_counter;
};

class VBO_COLLECTION {
public:
	VBO_COLLECTION(std::vector<glm::vec3> a_pos = {}, std::vector<glm::vec3> a_normals = {}, std::vector<glm::vec2> a_textures = {}, std::vector<glm::vec3> a_indicies = {});
    ~VBO_COLLECTION();

	auto get_vert_sum() const -> int;
	auto VBO_normals() const -> unsigned int;
	auto VBO_indicies() const -> unsigned int;
	auto VBO_positions() const -> unsigned int;
	auto VBO_texture_coords() const -> unsigned int;

private:
    unsigned int m_VBO_normals;
    unsigned int m_VBO_indicies;
    unsigned int m_VBO_positions;
    unsigned int m_VBO_texture_coords;
	size_t m_vert_sum = 0;
};

class VBO_CUBE : public VBO_COLLECTION {
public:
	VBO_CUBE();
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


enum class BufferType {
	VERTEX,
	ELEMENT,
	NONE
};

struct Buffer_data {
	int vert_sum = 0;
	BufferType buffer_type;
	std::vector<glm::vec3> normals = {};
	std::vector<glm::vec3> positions = {};
	std::vector<glm::vec2> texture_coords = {};
	std::vector<unsigned int> indicies = {};

	explicit Buffer_data(BufferType a_type);
	Buffer_data(Buffer_data&& a_data);
	Buffer_data(const Buffer_data& a_data);
	Buffer_data() = delete;
};

class Mesh {
public:
	Buffer_data m_data;

	Mesh(const Buffer_data& a_data, bool a_wireframe = false);
	Mesh(BufferType a_buf_type, const VBO_COLLECTION& a_VBOs, bool a_wireframe = false);
	Mesh(const Mesh& a_mesh);
	~Mesh();

	auto set_pos(const std::vector<glm::vec3>& a_pos = {}) -> void;
	auto set_normals(const std::vector<glm::vec3>& a_normals = {}) -> void;
	auto set_texture_coords(const std::vector<glm::vec2>& a_texture_coords = {}) -> void;
	auto set_elements(const std::vector<unsigned int>& a_elem_indicies = {}) -> void;

	auto reuse_pos(unsigned int a_VBO_vert) -> void;
	auto reuse_color(unsigned int a_VBO_color) -> void;
	auto reuse_normals(unsigned int a_VBO_normal) -> void;
	auto reuse_texture_coords(unsigned int a_VBO_texture_coords) -> void;
	auto reuse_elements(unsigned int a_EBO) -> void;

	auto update_pos() -> void;
	auto update_color() -> void;
	auto update_texture_coords() -> void;
	auto draw_vertices() -> void;

	auto get_VBO_pos() const -> unsigned int;
	auto get_VBO_color() const -> unsigned int;
	auto get_VBO_texture_coords() const -> unsigned int;
	auto get_VBO_indicies() const -> unsigned int;

private:
	unsigned int m_VAO;
	unsigned int m_VBO_pos;
	unsigned int m_VBO_texture_coords;
	unsigned int m_VBO_normal;
	unsigned int m_EBO;
	bool m_wireframe;
};
