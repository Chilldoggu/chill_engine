#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "imgui/imgui.h"

#include <string>
#include <memory>

#include "camera.hpp"

namespace chill_engine {
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

	auto mouse_callback(double x_pos, double y_pos) -> void;
	auto framebuffer_size_callback(int width, int height) -> void;

	auto closed() -> bool;
	auto set_width(float width) -> void;
	auto set_height(float height) -> void;
	auto set_mouse_x(float x_pos) -> void;
	auto set_mouse_y(float y_pos) -> void;
	auto set_cursor_mode(CursorMode a_mode) -> void;
	auto title_change() -> void;
	auto title_lshift(int n) -> void;
	auto title_rshift(int n) -> void;
	auto calculate_delta() -> float;

	auto get_obj() const -> GLFWwindow*;
	auto get_delta() const -> float;
	auto get_title() const -> std::string;
	auto get_width() const -> int;
	auto get_height() const -> int;
	auto get_mouse_x() const -> float;
	auto get_mouse_y() const -> float;
	auto get_cursor_mode() const -> CursorMode;
	auto get_aspect_ratio() const -> float;

	auto get_camera() const -> Camera&;
	auto get_input_handle() const -> InputHandler&;
	auto get_imgui_handle() const -> ImGuiHandler&;

private:
	float m_delta_time = 0.0f;
	float m_last_frame = 0.0f;
	float m_current_frame = 0.0f;

	GLFWwindow* m_window;
	int m_width;
	int m_height;
	float m_mouse_pos_x;
	float m_mouse_pos_y;
	CursorMode m_cur_mode;
	std::string m_title;
	std::unique_ptr<Camera> m_camera;
	std::unique_ptr<InputHandler> m_input_handle;
	std::unique_ptr<ImGuiHandler> m_imgui_handle;
}; 
}