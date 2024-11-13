#include <tuple>

#include "chill_renderer/shadows.hpp"
#include "chill_renderer/meshes.hpp"

namespace chill_renderer {
void ShadowMap::set_resolution(int a_width, int a_height) {
	m_resolution.width = a_width;
	m_resolution.height = a_height;

	if (m_fb.get_id() == EMPTY_VBO) {
		m_fb.set_resolution(a_width, a_height);
		m_fb.attach(AttachmentType::COLOR_2D, AttachmentBufferType::NONE);
		m_fb.attach(AttachmentType::DEPTH, AttachmentBufferType::TEXTURE);

		auto& tex_depth_map = std::get<uPtrTex>(m_fb.get_depth_attachment_buffer().get_attachment());
		// Make sure that areas outside shadow map are lit by default.
		tex_depth_map->set_wrap(TextureWrap::CLAMP_BORDER);
		tex_depth_map->set_border_color(glm::vec3(1.f, 1.f, 1.f));
		// Needed for use with sampler2DShadow type in frag shader.
		tex_depth_map->set_cmp_func(TextureCmpFunc::LEQUAL);

		if (!m_fb.check_status()) {
			ERROR("[SHADOWMAP::SET_RESOLUTION] Framebuffer is not complete!", Error_action::throwing);
		}
	}
	else if (m_fb.get_width() != a_width || m_fb.get_height() != a_height) {
		m_fb.set_resolution(a_width, a_height);
	}
}

void ShadowMap::set_view(glm::vec3 a_pos, glm::vec3 a_target) {
	m_frustum.light_view = glm::lookAt(a_pos, a_target, glm::vec3(0.f, 1.f, 0.f));
}

void ShadowMap::set_proj(ProjectionType a_proj_type, float a_near_plane, float a_far_plane) {
	if (a_proj_type == ProjectionType::NONE) {
		m_frustum.proj_type = a_proj_type;
		m_frustum.near = 0;
		m_frustum.far = 0;
		m_frustum.light_proj = glm::mat4(0.f);
		return;
	}

	m_frustum.proj_type = a_proj_type;
	m_frustum.near = a_near_plane;
	m_frustum.far = a_far_plane;

	if (a_proj_type == ProjectionType::PERSPECTIVE) {
		m_frustum.light_proj = glm::perspective(glm::radians(90.f), static_cast<float>(m_resolution.width) / m_resolution.height, a_near_plane, a_far_plane);
	}
	else if (a_proj_type == ProjectionType::ORTOGHRAPHIC) {
		m_frustum.light_proj = glm::ortho(-40.f, 40.f, -40.f, 40.f, m_frustum.near, m_frustum.far); 
	}
}

void ShadowMap::set_unit_id(int a_unit_id) {
	if (a_unit_id < g_shadow_sampler_id || a_unit_id >= g_shadow_sampler_id + g_shadow_sampler_siz)
		return;

	// Set texture unit in order not to replace any used material maps in frag shader.
	auto& tex_depth_map = std::get<uPtrTex>(m_fb.get_depth_attachment_buffer().get_attachment());
	tex_depth_map->set_unit_id(a_unit_id);
}

void ShadowMap::bind() {
	m_fb.bind();
}

void ShadowMap::unbind() {
	m_fb.unbind();
}

void ShadowMap::activate() {
	m_fb.get_depth_attachment_buffer().activate();
}

bool ShadowMap::check_status() {
	if (m_fb.get_id() == EMPTY_VBO) return false;
	else if (!m_fb.check_status())  return false;

	return true;
}

int ShadowMap::get_unit_id() noexcept {
	auto& tex = std::get<uPtrTex>(m_fb.get_depth_attachment_buffer().get_attachment());
	return tex->get_unit_id();
}

float ShadowMap::get_near() const noexcept {
	return m_frustum.near;
}

float ShadowMap::get_far() const noexcept {
	return m_frustum.far;
}

ProjectionType ShadowMap::get_proj_type() const noexcept {
	return m_frustum.proj_type;
}

glm::mat4 ShadowMap::get_view_mat() const noexcept {
	return m_frustum.light_view;
}

glm::mat4 ShadowMap::get_proj_mat() const noexcept {
	return m_frustum.light_proj;
}

int ShadowMap::get_width() const noexcept {
	return m_resolution.width;
}

int ShadowMap::get_height() const noexcept {
	return m_resolution.height;
}
}

