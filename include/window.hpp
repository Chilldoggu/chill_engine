#pragma once

#include "imgui/imgui.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <functional>
#include <string>

enum class CursorMode {
    NORMAL,
    FIRST_PERSON,
};

class Window {
public:
    Window(int a_width, int a_height, std::string a_title, CursorMode a_mode = CursorMode::NORMAL);
    explicit Window();
    ~Window();

    auto draw(std::function<void(void)> draw_body, std::function<void(void)> custom_input, std::function<void(void)> imgui_body = []{}) -> void;
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
	auto toggle_cursor_mode() ->void;

	auto get_io() const -> ImGuiIO*;
    auto get_obj() const -> GLFWwindow*;
    auto get_delta() const -> float;
    auto get_title() const -> std::string;
    auto get_width() const -> float;
    auto get_height() const -> float;
    auto get_mouse_x() const -> float;
    auto get_mouse_y() const -> float;
    auto get_mouse_focus_status() const -> bool;

private:
    float m_delta_time;
    float m_last_frame;
    float m_current_frame;

    int m_width;
    int m_height;
    bool m_mouse_focus;
    float m_mouse_pos_x;
    float m_mouse_pos_y;
	CursorMode m_cur_mode;
	ImGuiIO* m_io;
    std::string m_title;
    GLFWwindow* m_window = nullptr;
};
