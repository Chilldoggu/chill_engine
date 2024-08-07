#pragma once

#include "imgui/imgui.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <string>

enum class CursorMode {
	NORMAL,
	FIRST_PERSON,
	IDLE,
};

class Window {
public:
	Window(int a_width, int a_height, const std::string& a_title, CursorMode a_mode = CursorMode::NORMAL);
	Window();
	~Window();

	auto closed() -> bool;
	auto set_width(float width) -> void;
	auto set_height(float height) -> void;
	auto set_mouse_x(float x_pos) -> void;
	auto set_mouse_y(float y_pos) -> void;
	auto title_change() -> void;
	auto title_lshift(int n) -> void;
	auto title_rshift(int n) -> void;
	auto process_input() -> void;
	auto calculate_delta() -> float;
	auto toggle_mouse_focus() -> void;
	auto change_cursor_mode(CursorMode a_mode) -> void;

	auto get_io() const -> ImGuiIO*;
	auto get_obj() const -> GLFWwindow*;
	auto get_delta() const -> float;
	auto get_title() const -> std::string;
	auto get_width() const -> float;
	auto get_height() const -> float;
	auto get_mouse_x() const -> float;
	auto get_mouse_y() const -> float;
	auto get_cursor_mode() const -> CursorMode;
	auto get_mouse_focus_status() const -> bool;

private:
	float m_delta_time    = 0.0f;
	float m_last_frame    = 0.0f;
	float m_current_frame = 0.0f;

	int m_width;
	int m_height;
	bool m_mouse_focus = false;
	float m_mouse_pos_x;
	float m_mouse_pos_y;
	CursorMode m_cur_mode;
	ImGuiIO* m_io;
	std::string m_title;
	GLFWwindow* m_window;
};
