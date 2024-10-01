#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

namespace chill_engine {
enum class CameraMovement {
	FORWARD,
	BACKWARD,
	RIGHT,
	LEFT,
};

class Camera {
public:
	// constructor vector values
	Camera(GLFWwindow* a_window, const glm::vec3& position = glm::vec3(0.0f, 0.0f, 0.0f));

	auto set_fov(float a_fov) noexcept -> void;
	auto set_up(const glm::vec3& a_up) noexcept -> void;
	auto set_right(const glm::vec3& a_right) noexcept -> void;
	auto set_target(const glm::vec3& a_target) noexcept -> void;
	auto set_world_up(const glm::vec3& a_world_up) noexcept -> void;
	auto set_position(const glm::vec3& a_position) noexcept -> void;
	auto set_far_plane(float a_far) noexcept -> void;
	auto set_near_plane(float a_near) noexcept -> void;
	auto set_movement_speed(float a_speed) noexcept -> void;
	auto process_keyboard(CameraMovement direction, float delta_time) noexcept -> void;
	auto process_mouse_movement(float x_offset, float y_offset, bool constrain_pitch = true) noexcept -> void;
	auto process_mouse_scroll(float y_offset) noexcept -> void;

	auto get_fov() const noexcept -> float;
	auto get_up() const noexcept -> glm::vec3;
	auto get_right() const noexcept -> glm::vec3;
	auto get_target() const noexcept -> glm::vec3;
	auto get_look_at() const -> glm::mat4;
	auto get_position() const noexcept -> glm::vec3;
	auto get_world_up() const noexcept -> glm::vec3;
	auto get_far_plane() const noexcept -> float;
	auto get_near_plane() const noexcept -> float;
	auto get_movement_speed() const noexcept -> float;
	auto get_projection_matrix(float width, float height) const -> glm::mat4;

private:
	// Window attached to
	GLFWwindow* m_window = nullptr;
	// camera Attributes
	glm::vec3 m_up = glm::vec3(0.f, 1.f, 0.f);
	glm::vec3 m_front = glm::vec3(0.f, 0.f, -1.f);
	glm::vec3 m_right = glm::vec3(1.f, 0.f, 0.f);
	glm::vec3 m_position = glm::vec3(0.f, 0.f, 0.f);
	glm::vec3 m_world_up = glm::vec3(0.f, 1.f, 0.f);
	// euler Angles
	float m_yaw = -90.f;
	float m_pitch = 0.f;
	float m_fov = 90.f;
	float m_far_plane = 100.f;
	float m_near_plane = 0.1f;
	float m_movement_speed = 4.f;
	float m_mouse_sensitivity = 0.1f;

	// calculates the front vector from the Camera's (updated) Euler Angles
	void update_camera_vectors() noexcept;
}; 
}