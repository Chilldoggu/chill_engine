#include "light.hpp"
#include <cmath>

Light::Light(glm::vec4 a_pos, glm::vec3 a_color, glm::vec3 a_ambient, glm::vec3 a_diffuse, glm::vec3 a_specular)
	:m_pos_dir{ a_pos }, m_color{ a_color }, m_ambient_intens{ a_ambient }, m_diffuse_intens { a_diffuse }, m_specular_intens{ a_specular } { }

void Light::set_color(glm::vec3 a_color) {
	m_color = a_color;
}

void Light::set_pos_dir(glm::vec4 a_pos_dir) {
	m_pos_dir = a_pos_dir;
}

void Light::set_ambient_intens(glm::vec3 a_intens) {
	m_ambient_intens = a_intens;
}

void Light::set_diffuse_intens(glm::vec3 a_intens) {
	m_diffuse_intens = a_intens;
}

void Light::set_specular_intens(glm::vec3 a_intens) {
	m_specular_intens = a_intens;
}

glm::vec3 Light::get_color() const {
	return m_color;
}

glm::vec4 Light::get_pos_dir() const {
	return m_pos_dir;
}

glm::vec3 Light::get_ambient() const {
	return m_ambient_intens;
}

glm::vec3 Light::get_diffuse() const {
	return m_diffuse_intens;
}

glm::vec3 Light::get_specular() const {
	return m_specular_intens;
}

DirectionalLight::DirectionalLight(glm::vec3 a_direction, glm::vec3 a_color, glm::vec3 a_ambient, glm::vec3 a_diffuse, glm::vec3 a_specular)
	:Light(glm::vec4(a_direction, 0.0f), a_color, a_ambient, a_diffuse, a_specular) { }

void DirectionalLight::set_dir(glm::vec3 a_dir) {
	m_pos_dir = glm::vec4(a_dir, 0.0f);
}

glm::vec3 DirectionalLight::get_dir() const {
	return glm::vec3(m_pos_dir);
}

// Light(LightType a_type, glm::vec4 a_pos = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), glm::vec3 a_color = glm::vec3(1.0f, 1.0f, 1.0f), );
PointLight::PointLight(float a_max_distance, glm::vec3 a_pos, glm::vec3 a_color, glm::vec3 a_ambient, glm::vec3 a_diffuse, glm::vec3 a_specular) 
	:Light(glm::vec4(a_pos, 1.0f), a_color, a_ambient, a_diffuse, a_specular), m_constant{ 1.0f }, m_linear{ gen_att_linear(a_max_distance) }, m_quadratic{ gen_att_quadratic(a_max_distance) } { }

void PointLight::set_pos(glm::vec3 a_pos) {
	m_pos_dir = glm::vec4(a_pos, 1.0f);
}

void PointLight::set_const_att(float a_const) {
	m_constant = a_const;
}

void PointLight::set_linear_att(float a_linear) {
	m_linear = a_linear;
}

void PointLight::set_quadratic_att(float a_quadratic) {
	m_quadratic = a_quadratic;
}

glm::vec3 PointLight::get_pos() const {
	return glm::vec3(m_pos_dir);
}

float PointLight::get_linear() const {
	return m_linear;
}

float PointLight::get_quadratic() const {
	return m_quadratic;
}

float PointLight::get_constant() const {
	return m_constant;
}

float PointLight::gen_att_quadratic(float a_max_distance) {
	return 88.6677f * std::pow(a_max_distance, -1.9772);
}

float PointLight::gen_att_linear(float a_max_distance) {
	return 4.6905f * std::pow(a_max_distance, -1.0097);
}

SpotLight::SpotLight(float a_cutoff_deg, glm::vec3 a_spot_dir, float a_max_distance, glm::vec3 pos, glm::vec3 a_color, glm::vec3 m_ambient, glm::vec3 m_diffuse, glm::vec3 m_specular)
	:PointLight(a_max_distance, pos, a_color, m_ambient, m_diffuse, m_specular), m_cutoff{ std::cos(glm::radians(a_cutoff_deg)) }, m_spot_dir{ a_spot_dir } { }

void SpotLight::set_cutoff(float a_cutoff_deg) {
	m_cutoff = std::cos(glm::radians(a_cutoff_deg));
}

void SpotLight::set_spot_dir(glm::vec3 a_spot_dir) {
	m_spot_dir = a_spot_dir;
}

float SpotLight::get_cutoff() const {
	return m_cutoff;
}

glm::vec3 SpotLight::get_spot_dir() const {
	return m_spot_dir;
}
