#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "imgui/imgui.h"

#include <string>
#include <memory>

#include "chill_renderer/camera.hpp"

namespace chill_renderer {
void ToggleButton(const char* str_id, bool* v);

enum class CursorMode {
	NORMAL,
	FOCUSED,
};

class InputHandler {
public:
	InputHandler() = default;
	InputHandler(GLFWwindow* a_window);
	auto process_input() const -> void;

private:
	GLFWwindow* m_window = nullptr;
};

class ImGuiHandler {
public:
	ImGuiHandler() = default;
	ImGuiHandler(GLFWwindow* a_window);
	auto set_imgui_dracula_style() -> void;
	auto set_imgui_darkness_style() -> void;

	auto get_io() const -> ImGuiIO*;
private:
	GLFWwindow* m_window = nullptr;
	ImGuiIO* m_io = nullptr;
};

class Window {
public:
	Window(int a_width, int a_height, const std::string& a_title, CursorMode a_mode);
	~Window();

	auto mouse_callback(double x_pos, double y_pos) noexcept -> void;
	auto framebuffer_size_callback(int width, int height) noexcept -> void;

	auto closed() -> bool;
	auto title_change() -> void;
	auto title_lshift(int n) -> void;
	auto title_rshift(int n) -> void;
	auto calculate_delta() noexcept -> float;
	
	auto set_width(float width) noexcept -> void;
	auto set_height(float height) noexcept -> void;
	auto set_mouse_x(float x_pos) noexcept -> void;
	auto set_mouse_y(float y_pos) noexcept -> void;
	auto set_cursor_mode(CursorMode a_mode) noexcept -> void;

	auto get_obj() const noexcept -> GLFWwindow*;
	auto get_delta() const noexcept -> float;
	auto get_title() const noexcept -> std::string;
	auto get_width() const noexcept -> int;
	auto get_height() const noexcept -> int;
	auto get_mouse_x() const noexcept -> float;
	auto get_mouse_y() const noexcept -> float;
	auto get_cursor_mode() const noexcept -> CursorMode;
	auto get_aspect_ratio() const noexcept -> float;

	auto get_camera() const -> Camera&;
	auto get_input_handle() const -> InputHandler&;
	auto get_imgui_handle() const -> ImGuiHandler&;

private:
	float m_delta_time = 0.0f;
	float m_last_frame = 0.0f;
	float m_current_frame = 0.0f;

	GLFWwindow* m_window = nullptr;
	int m_width = 0;
	int m_height = 0;
	float m_mouse_pos_x = 0.f;
	float m_mouse_pos_y = 0.f;
	CursorMode m_cur_mode = CursorMode::NORMAL;
	std::string m_title = "OpenGL";
	std::unique_ptr<Camera> m_camera = nullptr;
	std::unique_ptr<InputHandler> m_input_handle = nullptr;
	std::unique_ptr<ImGuiHandler> m_imgui_handle = nullptr;
}; 
}