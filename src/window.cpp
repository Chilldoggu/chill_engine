#include "window.hpp"
#include "camera.hpp"
#include "game.hpp"
#include "assert.hpp"
#include <GLFW/glfw3.h>

extern Game game;
// extern Camera Camera;
// extern Window Win;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0,  width, height);
	game.m_win.m_width = width;
}

void mouse_callback(GLFWwindow* window, double x_pos, double y_pos) {
	Window* win  = &game.m_win;

	if (!win->get_mouse_focus_status()) {
		win->set_mouse_x(x_pos);
		win->set_mouse_y(y_pos);
		win->toggle_mouse_focus();
	}
	float x_offset = x_pos - win->get_mouse_x();
	float y_offset = win->get_mouse_y() - y_pos;
	win->set_mouse_x(x_pos);
	win->set_mouse_y(y_pos);

	game.m_player.m_head.process_mouse_movement(x_offset, y_offset);
}

void scroll_callback(GLFWwindow* window, double x_offset, double y_offset) {
	game.m_player.m_head.process_mouse_scroll(y_offset);
}

Window::Window(int a_width, int a_height, std::string a_title, MODES a_mode) 
	:m_width{ a_width }, m_height{ a_height }, m_title{ a_title }, m_mouse_pos_x{ a_width / 2.f }, m_mouse_pos_y{ a_height / 2.f }, m_mouse_focus{ false },
	 m_delta_time{ 0.0f }, m_last_frame{ 0.0f }, m_current_frame{ 0.0f }
{
	if (!glfwInit()) {
		ERROR("Couldn't initialise glfw.");
		throw Error_code::init_error;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	m_window = glfwCreateWindow(m_width, m_height, m_title.c_str(), nullptr, nullptr);

	if (m_window == nullptr) {
		ERROR("Couldn't create a window.");
		throw Error_code::init_error;
	}

	glfwMakeContextCurrent(m_window);

	switch (a_mode) {
		case MODES::FPS: {
			glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			break;
		}
		case MODES::None: {
			break;
		}
	}

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		ERROR("Couldn't setup load glad function pointers.");
		throw Error_code::init_error;
	}

	glViewport(0, 0, m_width, m_height);

	glfwSetFramebufferSizeCallback(m_window, framebuffer_size_callback);
	glfwSetCursorPosCallback(m_window, mouse_callback);
	glfwSetScrollCallback(m_window, scroll_callback);
}

Window::Window() :Window(800, 600, "Window") { }

Window::~Window() {
	glfwTerminate();
}

void Window::process_input() {
	if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(m_window, GLFW_TRUE);
}

bool Window::closed() {
	return glfwWindowShouldClose(m_window) == GLFW_TRUE;
}

void Window::title_lshift(int n) {
	int siz = m_title.size();
	n %= siz;

	if (!n) return;

	m_title = m_title.substr(n, siz) + m_title.substr(0, n);
	title_change();
}

void Window::title_rshift(int n) {
	int siz = m_title.size();
	n %= siz;

	if (!n) return;

	m_title =  m_title.substr(siz-n, siz) + m_title.substr(0, siz-n);
	title_change();
}

void Window::title_change() {
	glfwSetWindowTitle(m_window, m_title.c_str());
}

void Window::draw(std::function<void(void)> draw_body, std::function<void(void)> custom_input) {
	while (!closed()) {
		process_input();
		custom_input();

		glClearColor(0.2, 0.2, 0.2, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		draw_body();

		glfwSwapBuffers(this->m_window);
		glfwPollEvents();
	}
}

void Window::set_mouse_x(float x_pos) {
	m_mouse_pos_x = x_pos;
}

void Window::set_mouse_y(float y_pos) {
	m_mouse_pos_y = y_pos;
}

void Window::set_width(float width) {
	m_width = width;
}

void Window::set_height(float height) {
	m_height = height;
}

void Window::toggle_mouse_focus() {
	m_mouse_focus = !m_mouse_focus;
}

float Window::calculate_delta() {
	m_current_frame = glfwGetTime();
	m_delta_time = m_current_frame - m_last_frame;
	m_last_frame = m_current_frame;
	return m_delta_time;
}

float Window::get_width() const {
	return m_width;
}

float Window::get_height() const {
	return m_height;
}

bool Window::get_mouse_focus_status() const {
	return m_mouse_focus;
}

float Window::get_mouse_x() const {
	return m_mouse_pos_x;
}
float Window::get_mouse_y() const {
	return m_mouse_pos_y;
}

float Window::get_delta() const {
	return m_delta_time;
}

std::string Window::get_title() const {
	return m_title;
}

GLFWwindow* Window::get_obj() const {
	return m_window;
}
