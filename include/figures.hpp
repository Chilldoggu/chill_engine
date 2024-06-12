#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "glm/fwd.hpp"

#include <memory>
#include <vector>

#include "meshes.hpp"

template<typename T>
unsigned int constexpr VBO_generate(std::vector<T> data);

class ShaderProgram;

enum class Axis {
    X,
    Y,
    Z
};

struct Angle {
    float roll;
    float pitch;
    float yaw;

    Angle(float a_roll = 0.0f, float a_pitch = 0.0f, float a_yaw = 0.0f);
};


class Shape {
public:
    Shape(glm::vec3 a_center, float a_size, float a_degree_angle, BufferType a_data_type, std::vector<glm::vec3> a_positions, std::vector<unsigned int> a_elem_indices = {}, bool a_wireframe = false);
    Shape(glm::vec3 a_center, float a_size, float a_degree_angle, BufferType a_data_type, const VBO_COLLECTION& a_VBOs, bool a_wireframe = false);

    auto set_pos(glm::vec3 a_center) -> void;
    auto set_normals(const std::vector<glm::vec3>& a_normals) -> void;
    auto set_texture_coords(std::vector<glm::vec2> a_texture_coords = {}) -> void;
    auto set_material_map(const MaterialMap& a_material_map) -> void;

    auto move(glm::vec3 a_vec) -> void;
    auto rotate(float degree_angle, Axis axis = Axis::Z) -> void;
    auto resize(float a_size) -> void;
    auto resize(glm::vec3 a_size) -> void;
    auto toggle_material_map() -> void;
    auto reset() -> void;
    auto draw(ShaderProgram& a_shader) -> void;

	auto get_material_map() -> MaterialMap&;
    auto get_obj_data() const -> Buffer_data;
    auto get_size() const -> glm::vec3;
    auto get_pos() const -> glm::vec3;
    auto get_model() const -> glm::mat4;
    auto get_normal_mat() const -> glm::mat3;
    auto inside_viewport() const -> bool;
    auto print_verts() const -> void;
    auto print_texture_verts() const -> void;

private:
	Angle m_rad_angles;
	glm::vec3 m_center;
	MaterialMap m_material_map;
	glm::vec3 m_size;
	glm::mat4 m_transform_scale;
	glm::mat4 m_transform_rotation;
	glm::mat4 m_transform_pos;
	glm::mat2 m_texture_scalar;
	std::shared_ptr<Mesh> m_shape_obj;
	VBO_COLLECTION m_reused_VBOs;
};

class Triangle2D : public Shape {
public:
    Triangle2D(glm::vec3 a_center, float a_size, float a_degree_angle, bool a_wireframe = false);
private:
};

class Rectangle2D : public Shape {
public:
    Rectangle2D(glm::vec3 a_center, float a_size, float a_degree_angle, bool a_wireframe = false);
private:
};

class Cube : public Shape {
public:
    Cube(glm::vec3 a_center, float a_size, float a_degree_angle, bool a_wireframe = false); // Create new local VBOs
    Cube(glm::vec3 a_center, float a_size, float a_degree_angle, const VBO_COLLECTION& a_VBOs, bool a_wireframe = false); // Reuse global VBOs
private:
};
