#include <format>
#include <iostream>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "app.hpp"

static void glfw_error_callback(int error, const char* description) {
	std::cerr << std::format("GLFW Error {}: {}", error, description) << std::endl;
}

App::App(int win_width, int win_height, std::string win_title, CursorMode win_mode, glm::vec3 player_head_pos) 
	:m_win{ new Window(win_width, win_height, win_title, win_mode) }, m_cam{ new Camera(player_head_pos) }
{
	// Initialise callbacks
	// Use Set/Get UserPointer for scalabale solution for per window callbacks to member functions
	glfwSetWindowUserPointer(m_win->get_obj(), this);
	auto frambebuffer_size_callback = [](GLFWwindow* w, int width, int height) {
		static_cast<App*>(glfwGetWindowUserPointer(w))->framebuffer_size_callback(width, height);
	};
	auto cursor_pos_callback = [](GLFWwindow* w, double x_pos, double y_pos) {
		static_cast<App*>(glfwGetWindowUserPointer(w))->mouse_callback(x_pos, y_pos);
	};
	auto scroll_callback = [](GLFWwindow* w, double x_offset, double y_offset) {
		static_cast<App*>(glfwGetWindowUserPointer(w))->m_cam->process_mouse_scroll(y_offset);
	};

	glfwSetFramebufferSizeCallback(m_win->get_obj(), frambebuffer_size_callback);
	glfwSetCursorPosCallback(m_win->get_obj(), cursor_pos_callback);
	glfwSetScrollCallback(m_win->get_obj(), scroll_callback);
	glfwSetErrorCallback(glfw_error_callback);

	// Can't do it in window constructor because this method of callback setting modifies
	// GLFWwindow* window pointer that's being used for imgui initialisation.
	ImGui_ImplGlfw_InitForOpenGL(m_win->get_obj(), true);
}

App::~App() {
	for (auto& key : m_shaders) {
		ShaderProgram* shader = key.second;
		delete shader;
	}
}

void App::new_shader(const std::string& a_name, ShaderProgram& a_shader) {
	m_shaders[a_name] = new ShaderProgram(a_shader);
	m_shaders[a_name]->set_name(a_name);
}

void App::new_shader(const std::string& a_name, std::initializer_list<ShaderSrc> a_shaders_src) {
	m_shaders[a_name] = new ShaderProgram(a_shaders_src);
	m_shaders[a_name]->set_name(a_name);
}

void App::framebuffer_size_callback(int width, int height) {
	glViewport(0, 0,  width, height);
	m_win->set_width(width);
	m_win->set_height(height);
}

void App::mouse_callback(double x_pos, double y_pos) {
	if (m_win->get_mouse_focus_status()) {
		m_win->set_mouse_x(x_pos);
		m_win->set_mouse_y(y_pos);
		m_win->toggle_mouse_focus();
	}

	float x_offset = x_pos - m_win->get_mouse_x();
	float y_offset = m_win->get_mouse_y() - y_pos;

	m_win->set_mouse_x(x_pos);
	m_win->set_mouse_y(y_pos);

	if (m_win->get_cursor_mode() != CursorMode::IDLE)
		m_cam->process_mouse_movement(x_offset, y_offset);
}

Window& App::get_win() {
	return *m_win;
}

Camera& App::get_cam() {
	return *m_cam;
}

ShaderProgram& App::get_shader(const std::string& name) {
	return *(m_shaders[name]);
}
