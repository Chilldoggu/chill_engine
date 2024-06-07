#include "camera.hpp"

Camera::Camera(const glm::vec3& a_position, const glm::vec3& up, float a_yaw, float a_pitch, float a_near_plane, float a_far_plane)
	:m_front{ glm::vec3(0.0f, 0.0f, -1.0f) }, m_movement_speed{ SPEED }, m_mouse_sensitivity{ SENSITIVITY }, m_fov{ FOV }, m_near_plane{ a_near_plane }, m_far_plane{ a_far_plane },
	 m_position{ a_position }, m_world_up{ up }, m_yaw{ a_yaw }, m_pitch{ a_pitch }
{
	update_camera_vectors();
}

// constructor with scalar values
Camera::Camera(float a_pos_x, float a_pos_y, float a_pos_z, float a_up_x, float a_up_y, float a_up_z, float a_yaw, float a_pitch, float a_near_plane, float a_far_plane)
	:Camera(glm::vec3(a_pos_x, a_pos_y, a_pos_z), glm::vec3(a_up_x, a_up_y, a_up_z), a_yaw, a_pitch, a_near_plane, a_far_plane) { }

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
	// m_position = glm::vec3(m_position.x, height, m_position.z);
}

// processes input received from a mouse input system. Expects the offset value in both the x and y direction.
void Camera::process_mouse_movement(float x_offset, float y_offset, GLboolean constrain_pitch) {
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
	glm::vec3 front;
	front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
	front.y = sin(glm::radians(m_pitch));
	front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
	m_front = glm::normalize(front);
	// also re-calculate the Right and Up vector
	m_right = glm::normalize(glm::cross(m_front, m_world_up));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
	m_up    = glm::normalize(glm::cross(m_right, m_front));
}

/*****************
 *   GRAVEYARD   *
 *****************

glm::mat4 Camera::get_own_look_at(glm::vec3 position, glm::vec3 target, glm::vec3 world_up) const {
	glm::vec3 direction = glm::normalize(position - target);
	glm::vec3 right = glm::normalize(glm::cross(direction, world_up));
	glm::vec3 up = glm::cross(right, direction);

	glm::mat4 pos{ 1.0f };
	position *= -1.0f;
	pos = glm::translate(pos, glm::vec3(position.x, position.y, position.z));

	glm::mat4 rotation{ 1.0f };
	rotation[0][0] = right.x;
	rotation[1][0] = right.y;
	rotation[2][0] = right.z;
	rotation[0][1] = up.x;
	rotation[1][1] = up.y;
	rotation[2][1] = up.z;
	rotation[0][2] = direction.x;
	rotation[1][2] = direction.y;
	rotation[2][2] = direction.z;
	return rotation * pos;
}
*/
