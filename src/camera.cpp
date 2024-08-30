#include "chill_engine/camera.hpp"
#include "chill_engine/application.hpp"

Camera::Camera(GLFWwindow* a_window, const glm::vec3& a_position)
	:m_position{ a_position }, m_window{ a_window }
{
	auto mouse_callback = [](GLFWwindow* w, double x_pos, double y_pos) {
		Window* p_win = static_cast<Window*>((Window*)glfwGetWindowUserPointer(w));
		p_win->mouse_callback(x_pos, y_pos);
	}; 
	auto scroll_callback = [](GLFWwindow* w, double x_offset, double y_offset) {
		Window* p_win = static_cast<Window*>((Window*)glfwGetWindowUserPointer(w));
		p_win->get_camera().process_mouse_scroll(y_offset);
	};

	glfwSetCursorPosCallback(a_window, mouse_callback);
	glfwSetScrollCallback(a_window, scroll_callback);

	update_camera_vectors();
}

glm::mat4 Camera::get_projection_matrix(float width, float height) const {
	return glm::perspective(glm::radians(m_fov), width/height, m_near_plane, m_far_plane);
}

glm::vec3 Camera::get_position() const {
	return m_position;
}

float Camera::get_far_plane() const {
	return m_far_plane;
}

float Camera::get_near_plane() const {
	return m_near_plane;
}

glm::vec3 Camera::get_target() const {
	return m_front;
}

// returns the view matrix calculated using Euler Angles and the LookAt Matrix
glm::mat4 Camera::get_look_at() const {
	return glm::lookAt(m_position, m_position + m_front, m_up);
}

float Camera::get_movement_speed() const {
	return m_movement_speed;
}

void Camera::set_fov(float a_fov) {
	m_fov = a_fov;
}

void Camera::set_target(glm::vec3 a_target) {
	m_front = a_target;
}

void Camera::set_position(glm::vec3 a_position) {
	m_position = a_position;
}

void Camera::set_far_plane(float a_far) {
	m_far_plane = a_far;
}

void Camera::set_near_plane(float a_near) {
	m_near_plane = a_near;
}

auto Camera::set_movement_speed(float a_speed) -> void {
	if (a_speed >= 0) {
		m_movement_speed = a_speed;
	}
}

// processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
void Camera::process_keyboard(CameraMovement direction, float delta_time) {
	float velocity = m_movement_speed * delta_time;
	// float height = m_position.y;
	if (direction == CameraMovement::FORWARD)
		m_position += m_front * velocity;
	if (direction == CameraMovement::BACKWARD)
		m_position -= m_front * velocity;
	if (direction == CameraMovement::LEFT)
		m_position -= m_right * velocity;
	if (direction == CameraMovement::RIGHT)
		m_position += m_right * velocity;
}

// processes input received from a mouse input system. Expects the offset value in both the x and y direction.
void Camera::process_mouse_movement(float x_offset, float y_offset, bool constrain_pitch) {
	x_offset *= m_mouse_sensitivity;
	y_offset *= m_mouse_sensitivity;

	m_yaw   += x_offset;
	m_pitch += y_offset;

	// make sure that when pitch is out of bounds, screen doesn't get flipped
	if (constrain_pitch) {
		if (m_pitch >  89.0f) m_pitch =  89.0f;
		if (m_pitch < -89.0f) m_pitch = -89.0f;
	}

	// update Front, Right and Up Vectors using the updated Euler angles
	update_camera_vectors(); 
}

// processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
void Camera::process_mouse_scroll(float y_offset) {
	m_fov -= y_offset;
	if (m_fov <  1.0f) m_fov = 1.0f;
	if (m_fov > 90.0f) m_fov = 90.0f;
}

float Camera::get_fov() const {
	return m_fov;
}

void Camera::update_camera_vectors() {
	// calculate the new Front vector
	glm::vec3 front(0);
	front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
	front.y = sin(glm::radians(m_pitch));
	front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
	m_front = glm::normalize(front);
	// also re-calculate the Right and Up vector
	m_right = glm::normalize(glm::cross(m_front, m_world_up));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
	m_up    = glm::normalize(glm::cross(m_right, m_front));
}