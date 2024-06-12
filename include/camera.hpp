#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

const float YAW         = -90.0f;
const float PITCH       = 0.0f;
const float NEAR_PLANE  = 0.1f;
const float FAR_PLANE   = 100.0f;
const float FOV         = 90.0f;
const float SPEED       = 4.0f;
const float SENSITIVITY = 0.1f;

enum class CameraMovement {
    FORWARD,
    BACKWARD,
    RIGHT,
    LEFT,
};

class Camera {
public:
    // constructor vector values
    Camera(const glm::vec3& position = glm::vec3(0.0f, 0.0f, 0.0f), const glm::vec3& up = glm::vec3(0.0f, 1.0f, 0.0f), 
           float yaw = YAW, float pitch = PITCH, float near_plane = NEAR_PLANE, float far_plane = FAR_PLANE);
    // construtor for scalar values
    Camera(float pos_x, float pos_y, float pos_z, float up_x, float up_y, float up_z,
           float yaw, float pitch, float near_plane = NEAR_PLANE, float far_plane = FAR_PLANE);

    auto set_fov(float a_fov) -> void;
    auto set_target(glm::vec3 a_target) -> void;
    auto set_position(glm::vec3 a_position) -> void;
    auto set_far_plane(float a_far) -> void;
    auto set_near_plane(float a_near) -> void;
    auto set_movement_speed(float a_speed) -> void;
    auto process_keyboard(CameraMovement direction, float delta_time) -> void;
    auto process_mouse_movement(float x_offset, float y_offset, GLboolean constrain_pitch = true) -> void;
    auto process_mouse_scroll(float y_offset) -> void;

    auto get_fov() const -> float;
    auto get_target() const -> glm::vec3;
    auto get_look_at() const -> glm::mat4;
    auto get_position() const -> glm::vec3;
	auto get_far_plane() const -> float;
	auto get_near_plane() const -> float;
	auto get_movement_speed() const -> float;
    auto get_projection_matrix(float width, float height) const -> glm::mat4;

private:
    // camera Attributes
    glm::vec3 m_up;
    glm::vec3 m_right;
    glm::vec3 m_front;
    glm::vec3 m_position;
    glm::vec3 m_world_up;
    // euler Angles
    float m_yaw;
    float m_pitch;
    // camera options
    float m_fov;
    float m_far_plane;
    float m_near_plane;
    float m_movement_speed;
    float m_mouse_sensitivity;

    // calculates the front vector from the Camera's (updated) Euler Angles
    void update_camera_vectors();
};
