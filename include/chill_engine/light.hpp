#pragma once

#include <glm/glm.hpp>

namespace chill_engine {
enum class LightType {
	DIRECTIONAL,
	POINT,
	SPOTLIGHT,
};

class Light {
public:
	Light(const glm::vec4& a_pos = glm::vec4(0.0, 0.0, 0.0, 1.0));

	auto set_color(const glm::vec3& a_color) -> void;
	auto set_pos_dir(const glm::vec4& a_pos_dir) -> void; // 4th element decides whether light is positional or directional
	auto set_ambient_intens(const glm::vec3& a_intens) -> void;
	auto set_diffuse_intens(const glm::vec3& a_intens) -> void;
	auto set_specular_intens(const glm::vec3& a_intens) -> void;

	auto get_color() const -> glm::vec3;
	auto get_pos_dir() const -> glm::vec4;
	auto get_ambient() const -> glm::vec3;
	auto get_diffuse() const -> glm::vec3;
	auto get_specular() const -> glm::vec3;

protected:
	// either pos or dir depending on w == 1.0 or w == 0.0
	glm::vec4 m_pos_dir = glm::vec4(0.0, 0.0, 0.0, 1.0);
	glm::vec3 m_color = glm::vec3(1.0);
	glm::vec3 m_ambient_intens = glm::vec3(0.3);
	glm::vec3 m_diffuse_intens = glm::vec3(0.8);
	glm::vec3 m_specular_intens = glm::vec3(1.0);
};

class DirLight : public Light {
public:
	DirLight(const glm::vec3& a_direction);

	auto set_dir(const glm::vec3& a_dir) -> void;

	auto get_dir() const -> glm::vec3;
};

class PointLight : public Light {
public:
	PointLight(float a_max_distance, const glm::vec3& a_pos = glm::vec3(0.0));

	auto set_pos(const glm::vec3& a_pos) -> void;
	auto set_max_dist(float a_max_distance) -> void;
	auto set_const_att(float a_const) -> void;
	auto set_linear_att(float a_linear) -> void;
	auto set_quadratic_att(float a_quadratic) -> void;

	auto get_pos() const -> glm::vec3;
	auto get_max_dist() const -> float;
	auto get_linear() const -> float;
	auto get_quadratic() const -> float;
	auto get_constant() const -> float;

private:
	float m_linear = 0.f;
	float m_quadratic = 0.f;
	float m_constant = 1.f;
	float m_max_distance = 50.f;

	// Approximation of light attenuation based on max distance.
	// Works OK for distances <1, 2000>.
	auto gen_att_quadratic(float a_max_distance) -> float;
	auto gen_att_linear(float a_max_distance) -> float;
};

class SpotLight : public PointLight {
public:
	SpotLight(float a_inner_cutoff_deg, float a_outer_cutoff_deg, const glm::vec3& a_spot_dir, float a_max_distance, const glm::vec3& pos = glm::vec3(0.0));

	auto set_inner_cutoff(float a_cutoff_deg) -> void;
	auto set_outer_cutoff(float a_cutoff_deg) -> void;
	auto set_spot_dir(const glm::vec3& a_spot_dir) -> void;

	auto get_inner_cutoff_deg() const -> float;
	auto get_outer_cutoff_deg() const -> float;
	auto get_inner_cutoff() const -> float;
	auto get_outer_cutoff() const -> float;
	auto get_spot_dir() const -> glm::vec3;

private:
	float m_inner_cutoff_deg = 0.f;
	float m_outer_cutoff_deg = 0.f;
	float m_inner_cutoff = 0.f; // cos(theta)
	float m_outer_cutoff = 0.f; // cos(theta)
	glm::vec3 m_spot_dir = glm::vec3(0.0);
}; 
}