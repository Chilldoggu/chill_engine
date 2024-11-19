#pragma once

#include <glm/glm.hpp>

#include "chill_renderer/buffers.hpp"


namespace chill_renderer { 
enum class ProjectionType {
	ORTOGHRAPHIC,
	PERSPECTIVE,
	NONE,
};

// Doesn't change glViewport. Do it yourself.
class ShadowMap {
public:
	ShadowMap() = default;
	
	auto bind() -> void;
	auto unbind() -> void;
	auto activate() -> void;
	auto check_status() -> bool;

	auto set_resolution(int a_width, int a_height) -> void; 
	auto set_view(glm::vec3 a_pos, glm::vec3 a_target) -> void; 
	auto set_proj(ProjectionType a_proj_type, float a_near_plane, float a_far_plane) -> void; 
	auto set_unit_id(int a_unit_id) -> void;
	auto set_offset_window(int a_window_size, int a_filter_width, int a_filter_height) -> void;

	auto get_unit_id() noexcept -> int ;
	auto get_near() const noexcept -> float;
	auto get_far() const noexcept -> float;
	auto get_proj_type() const noexcept -> ProjectionType;
	auto get_view_mat() const noexcept -> glm::mat4;
	auto get_proj_mat() const noexcept -> glm::mat4;
	auto get_width() const noexcept -> int;
	auto get_height() const noexcept -> int;
	auto get_offset_window() noexcept -> Texture3D&;
	auto get_offset_window_size() noexcept -> int;

private:
	FrameBuffer m_fb{};
	std::unique_ptr<Texture3D> m_offset_window = nullptr;

	struct Frustum {
		float near{};
		float far{};
		ProjectionType proj_type{ ProjectionType::NONE };
		glm::mat4 light_view{};
		glm::mat4 light_proj{}; 
	} m_frustum;

	struct Resolution {
		int width{};
		int height{};
	} m_resolution;

	struct OffsetWindowResolution {
		int size{};
		int filter_width{};
		int filter_height{};
	} m_offwin_resolution;
};
}
