#include <cmath>

#include "chill_engine/light.hpp"

namespace chill_engine {
Light::Light(const glm::vec4& a_pos) :m_pos_dir{ a_pos } {}

void Light::set_color(const glm::vec3& a_color) noexcept {
	m_color = a_color;
}

void Light::set_pos_dir(const glm::vec4& a_pos_dir) noexcept {
	m_pos_dir = a_pos_dir;
}

void Light::set_ambient_intens(const glm::vec3& a_intens) noexcept {
	m_ambient_intens = a_intens;
}

void Light::set_diffuse_intens(const glm::vec3& a_intens) noexcept {
	m_diffuse_intens = a_intens;
}

void Light::set_specular_intens(const glm::vec3& a_intens) noexcept {
	m_specular_intens = a_intens;
}

glm::vec3 Light::get_color() const noexcept {
	return m_color;
}

glm::vec4 Light::get_pos_dir() const noexcept {
	return m_pos_dir;
}

glm::vec3 Light::get_ambient() const noexcept {
	return m_ambient_intens;
}

glm::vec3 Light::get_diffuse() const noexcept {
	return m_diffuse_intens;
}

glm::vec3 Light::get_specular() const noexcept {
	return m_specular_intens;
}

DirLight::DirLight(const glm::vec3& a_direction) :Light(glm::vec4(a_direction, 0.0f)) { }

void DirLight::set_dir(const glm::vec3& a_dir) noexcept {
	m_pos_dir = glm::vec4(a_dir, 0.0f);
}

glm::vec3 DirLight::get_dir() const noexcept {
	return glm::vec3(m_pos_dir);
}

// Light(LightType a_type, glm::vec4 a_pos = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), glm::vec3 a_color = glm::vec3(1.0f, 1.0f, 1.0f), );
PointLight::PointLight(float a_max_distance, const glm::vec3& a_pos) :Light(glm::vec4(a_pos, 1.0f)) {
	m_max_distance = a_max_distance;
	m_linear = gen_att_linear(a_max_distance);
	m_quadratic = gen_att_quadratic(a_max_distance);
}

void PointLight::set_pos(const glm::vec3& a_pos) noexcept {
	m_pos_dir = glm::vec4(a_pos, 1.0f);
}

void PointLight::set_max_dist(float a_max_distance) noexcept {
	m_max_distance = a_max_distance;
	m_linear = gen_att_linear(a_max_distance);
	m_quadratic = gen_att_quadratic(a_max_distance);
}

void PointLight::set_const_att(float a_const) noexcept {
	m_constant = a_const;
}

void PointLight::set_linear_att(float a_linear) noexcept {
	m_linear = a_linear;
}

void PointLight::set_quadratic_att(float a_quadratic) noexcept {
	m_quadratic = a_quadratic;
}

glm::vec3 PointLight::get_pos() const noexcept {
	return glm::vec3(m_pos_dir);
}

float PointLight::get_max_dist() const noexcept {
	return m_max_distance;
}

float PointLight::get_linear() const noexcept {
	return m_linear;
}

float PointLight::get_quadratic() const noexcept {
	return m_quadratic;
}

float PointLight::get_constant() const noexcept {
	return m_constant;
}

float PointLight::gen_att_quadratic(float a_max_distance) noexcept {
	return 88.6677f * std::pow(a_max_distance, -1.9772);
}

float PointLight::gen_att_linear(float a_max_distance) noexcept {
	return 4.6905f * std::pow(a_max_distance, -1.0097);
}

SpotLight::SpotLight(float a_inner_cutoff_deg, float a_outer_cutoff_deg, const glm::vec3& a_spot_dir, float a_max_distance, const glm::vec3& pos)
	:PointLight(a_max_distance, pos), m_spot_dir{ a_spot_dir }
{
	m_inner_cutoff = std::cos(glm::radians(a_inner_cutoff_deg));
	m_outer_cutoff = std::cos(glm::radians(a_outer_cutoff_deg));
	m_inner_cutoff_deg = a_inner_cutoff_deg;
	m_outer_cutoff_deg = a_outer_cutoff_deg;
}

void SpotLight::set_inner_cutoff(float a_cutoff_deg) noexcept {
	m_inner_cutoff = std::cos(glm::radians(a_cutoff_deg));
	m_inner_cutoff_deg = a_cutoff_deg;
}

void SpotLight::set_outer_cutoff(float a_cutoff_deg) noexcept {
	m_outer_cutoff = std::cos(glm::radians(a_cutoff_deg));
	m_outer_cutoff_deg = a_cutoff_deg;
}

void SpotLight::set_spot_dir(const glm::vec3& a_spot_dir) noexcept {
	m_spot_dir = a_spot_dir;
}

float SpotLight::get_inner_cutoff_deg() const noexcept {
	return m_inner_cutoff_deg;
}

float SpotLight::get_outer_cutoff_deg() const noexcept {
	return m_outer_cutoff_deg;
}

float SpotLight::get_inner_cutoff() const noexcept {
	return m_inner_cutoff;
}

float SpotLight::get_outer_cutoff() const noexcept {
	return m_outer_cutoff;
}

glm::vec3 SpotLight::get_spot_dir() const noexcept {
	return m_spot_dir;
} 
}