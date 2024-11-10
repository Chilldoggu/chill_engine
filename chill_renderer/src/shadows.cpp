#include <tuple>

#include "chill_renderer/shadows.hpp"
#include "chill_renderer/meshes.hpp"

namespace chill_renderer {
auto ShadowMap::set_resolution(int a_width, int a_height) -> void {
	m_resolution.width = a_width;
	m_resolution.height = a_height;

	if (m_fb.get_id() == EMPTY_VBO) {
		m_fb.set_resolution(a_width, a_height);
		m_fb.attach(AttachmentType::COLOR_2D, AttachmentBufferType::NONE);
		m_fb.attach(AttachmentType::DEPTH, AttachmentBufferType::TEXTURE);

		auto& tex_depth_map = std::get<Texture>(m_fb.get_depth_attachment_buffer().get_attachment());
		// Make sure that areas outside shadow map are lit by default.
		tex_depth_map.set_wrap(TextureWrap::CLAMP_BORDER);
		tex_depth_map.set_border_color(glm::vec3(1.f, 1.f, 1.f));
		// Needed for use with sampler2DShadow type in frag shader.
		tex_depth_map.set_comp_func(TextureCompFunc::LEQUAL);

		if (!m_fb.check_status()) {
			ERROR("[SHADOWMAP::SET_RESOLUTION] Framebuffer is not complete!", Error_action::throwing);
		}
	}
	else if (m_fb.get_width() != a_width || m_fb.get_height() != a_height) {
		m_fb.set_resolution(a_width, a_height);
	}
}

auto ShadowMap::set_view(glm::vec3 a_pos, glm::vec3 a_target) -> void {
	m_frustum.light_view = glm::lookAt(a_pos, a_target, glm::vec3(0.f, 1.f, 0.f));
}

auto ShadowMap::set_proj(ProjectionType a_proj_type, float a_near_plane, float a_far_plane) -> void {
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

auto ShadowMap::set_unit_id(int a_unit_id) -> void {
	if (a_unit_id < g_shadow_sampler_id || a_unit_id >= g_shadow_sampler_id + g_shadow_sampler_siz)
		return;

	// Set texture unit in order not to replace any used material maps in frag shader.
	auto& tex_depth_map = std::get<Texture>(m_fb.get_depth_attachment_buffer().get_attachment());
	tex_depth_map.set_unit_id(a_unit_id);
}

auto ShadowMap::bind() -> void {
	m_fb.bind();
}

auto ShadowMap::unbind() -> void {
	m_fb.unbind();
}

auto ShadowMap::activate() -> void {
	m_fb.get_depth_attachment_buffer().activate();
}

auto ShadowMap::check_status() -> bool {
	if (m_fb.get_id() == EMPTY_VBO) return false;
	else if (!m_fb.check_status())  return false;

	return true;
}

auto ShadowMap::get_unit_id() noexcept -> int { 
	auto& tex = std::get<Texture>(m_fb.get_depth_attachment_buffer().get_attachment());
	return tex.get_unit_id();
}

auto ShadowMap::get_near() const noexcept -> float {
	return m_frustum.near;
}

auto ShadowMap::get_far() const noexcept -> float {
	return m_frustum.far;
}

auto ShadowMap::get_proj_type() const noexcept -> ProjectionType {
	return m_frustum.proj_type;
}

auto ShadowMap::get_view_mat() const noexcept -> glm::mat4 {
	return m_frustum.light_view;
}

auto ShadowMap::get_proj_mat() const noexcept -> glm::mat4 {
	return m_frustum.light_proj;
}

auto ShadowMap::get_width() const noexcept -> int {
	return m_resolution.width;
}

auto ShadowMap::get_height() const noexcept -> int {
	return m_resolution.height;
}
}

