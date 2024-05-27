#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "glm/fwd.hpp"

#include <memory>
#include <string>
#include <vector>

#include "shaders.hpp"
#include "buffer_obj.hpp"

template<typename T>
unsigned int constexpr VBO_generate(std::vector<T> data);

class Texture;

enum class Axis {
    X,
    Y,
    Z
};

enum class TextureType {
    TEX_1D,
    TEX_2D,
    TEX_3D,
    NONE,
};

struct Angle {
    float roll;
    float pitch;
    float yaw;

    Angle(float a_roll = 0.0f, float a_pitch = 0.0f, float a_yaw = 0.0f);
};

struct Point3D {
    float x;
    float y;
    float z;

    explicit Point3D(float a_x = 0.0f, float a_y = 0.0f, float a_z = 0.0f);

    Point3D& operator+=(const Point3D& p);
    Point3D& operator-=(const Point3D& p);
    Point3D& operator*=(const Point3D& p);
    Point3D& operator/=(const Point3D& p);
};


struct Material {
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float shininess;

    Material();
    Material(glm::vec3 a_ambient, glm::vec3 a_diffuse, glm::vec3 a_specular, float a_shininess);
};

struct MaterialMap {
    std::unique_ptr<Texture> diffuse_map;
    std::unique_ptr<Texture> specular_map;
    std::unique_ptr<Texture> emission_map;
    float shininess;

    MaterialMap(std::string a_diffuse_map = "", std::string a_specular_map = "", std::string a_emission_map = "", float a_shininess = 32);
    MaterialMap(const MaterialMap& a_material_map);

	auto set_diffuse_map(std::string a_diffuse_map) -> void;
	auto set_specular_map(std::string a_specular_map) -> void;
	auto set_emission_map(std::string a_emission_map) -> void;
	auto set_shininess(float a_shininess) -> void;
};

struct VBO_FIGURES {
    int vert_sum = 0;
    unsigned int VERTS    = EMPTY_VBO;
    unsigned int TEXTURE  = EMPTY_VBO;
    unsigned int NORMALS  = EMPTY_VBO;
    unsigned int INDICIES = EMPTY_VBO;

    ~VBO_FIGURES();
};

template<typename T>
unsigned int constexpr VBO_generate(std::vector<T> data) {
    unsigned int VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(T), data.data(), GL_STATIC_DRAW);
    return VBO;
}

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
    auto activate() -> void;

    auto get_width() const -> int;
    auto get_height() const -> int;
    auto get_type() const -> TextureType;
    auto get_name() const -> std::string;
    auto get_texture_id() const -> unsigned int;

private:
    int m_width;
    int m_height;
    int m_texture_unit;
    bool m_deletable;
    TextureType m_type;
    std::string m_name;
    unsigned int m_texture_id;
};

class Shape {
public:
    Shape(Point3D a_center, float a_size, float a_degree_angle, BufferType a_data_type, std::vector<float> a_verts, std::vector<int> a_elem_indices = {}, bool a_wireframe = false);
    Shape(Point3D a_center, float a_size, float a_degree_angle, BufferType a_data_type, const VBO_FIGURES& a_VBOs, bool a_wireframe = false);

    auto set_pos(Point3D a_center) -> void;
    auto set_pos(std::vector<float> a_vec) -> void;
    auto set_color(const std::vector<float>& a_color, bool a_multi = false, std::vector<std::vector<int>> a_color_oreder = {}) -> void;
    auto set_normals(const std::vector<float>& a_normals) -> void;
    auto set_texture_buf(std::vector<float> a_texture_cords = {}, float a_ratio = 1.f) -> void;
    auto set_material_map(const MaterialMap& a_material_map) -> void;
    // auto init_texture_stack(std::initializer_list<std::string> a_texture_names) -> void;

    auto move(Point3D a_vec) -> void;
    auto move(std::vector<float> a_vec) -> void;
    auto rotate(float degree_angle, Axis axis = Axis::Z) -> void;
    auto resize(float a_size) -> void;
    auto resize(std::vector<float> a_size) -> void;
    auto toggle_material_map() -> void;
    auto reset() -> void;
    auto draw(Shader_program& a_shader) -> void;

    auto get_obj_data() const -> Buffer_data;
    auto get_size() const -> glm::vec3;
    auto get_pos() const -> Point3D;
    auto get_model() const -> glm::mat4;
    auto get_normal_mat() const -> glm::mat3;
    auto inside_viewport() const -> bool;
    auto print_verts() const -> void;
    auto print_texture_verts() const -> void;

private:
    bool m_material_map_used;
    float m_texture_ratio;
    Angle m_rad_angles;
    Point3D m_center;
    MaterialMap m_material_map;
    glm::vec3 m_size;
    glm::mat4 m_transform_scale;
    glm::mat4 m_transform_rotation;
    glm::mat4 m_transform_pos;
    glm::mat2 m_texture_scalar;
    std::unique_ptr<VAO> m_shape_obj;
};

class Triangle2D : public Shape {
public:
    Triangle2D(Point3D a_center, float a_size, float a_degree_angle, bool a_wireframe = false);
private:
};

class Rectangle2D : public Shape {
public:
    Rectangle2D(Point3D a_center, float a_size, float a_degree_angle, bool a_wireframe = false);
private:
};

class Cube : public Shape {
public:
    Cube(Point3D a_center, float a_size, float a_degree_angle, bool a_wireframe = false); // Create new local VBOs
    Cube(Point3D a_center, float a_size, float a_degree_angle, const VBO_FIGURES& a_VBOs, bool a_wireframe = false); // Reuse global VBOs
private:
};
