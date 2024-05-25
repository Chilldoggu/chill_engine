#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <functional>
#include <string>

enum class MODES {
    None,
    FPS,
};

class Window {
public:
    Window(int a_width, int a_height, std::string a_title, MODES a_mode = MODES::None);
    explicit Window();
    ~Window();

    auto process_input() -> void;
    auto closed() -> bool;
    auto title_lshift(int n) -> void;
    auto title_rshift(int n) -> void;
    auto title_change() -> void;
    auto draw(std::function<void(void)> draw_body, std::function<void(void)> custom_input) -> void;
    auto set_mouse_x(float x_pos) -> void;
    auto set_mouse_y(float y_pos) -> void;
    auto set_width(float width) -> void;
    auto set_height(float height) -> void;
    auto toggle_mouse_focus() -> void;
    auto calculate_delta() -> float;

    auto get_width() const -> float;
    auto get_height() const -> float;
    auto get_mouse_focus_status() const -> bool;
    auto get_mouse_x() const -> float;
    auto get_mouse_y() const -> float;
    auto get_delta() const -> float;
    auto get_title() const -> std::string;
    auto get_obj() const -> GLFWwindow*;

    friend auto mouse_callback(GLFWwindow* window, double xpos, double ypos) -> void;
    friend auto framebuffer_size_callback(GLFWwindow* window, int width, int height) -> void;

private:
    float m_delta_time;
    float m_last_frame;
    float m_current_frame;

    int m_width;
    int m_height;
    float m_mouse_pos_x;
    float m_mouse_pos_y;
    bool m_mouse_focus;
    std::string m_title;
    GLFWwindow* m_window = nullptr;
};
