#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "window.hpp"
#include "assert.hpp"

#include <GLFW/glfw3.h>

Window::Window(int a_width, int a_height, const std::string& a_title, CursorMode a_mode) 
	:m_width{ a_width }, m_height{ a_height }, m_title{ a_title }, m_mouse_pos_x{ a_width / 2.f }, m_mouse_pos_y{ a_height / 2.f }, m_cur_mode{ a_mode }
{
	if (!glfwInit()) {
		ERROR("[WINDOW::WINDOW] Couldn't initialise glfw.", Error_action::throwing);
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	m_window = glfwCreateWindow(m_width, m_height, m_title.c_str(), nullptr, nullptr);

	if (m_window == nullptr) {
		ERROR("[WINDOW::WINDOW] Couldn't create a window.", Error_action::throwing);
	}

	glfwMakeContextCurrent(m_window);

	switch (a_mode) {
		case CursorMode::NORMAL:
			glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			break;
		case CursorMode::IDLE:
			glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			break;
		case CursorMode::FIRST_PERSON:
			glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			break;
	}

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		ERROR("[WINDOW:WINDOW] Couldn't load glad function pointers.", Error_action::throwing);
	}

	glViewport(0, 0, m_width, m_height);

	// Init Imgui
	IMGUI_CHECKVERSION();

	ImGui::CreateContext();

	m_io = &ImGui::GetIO();
	m_io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    m_io->ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	
	ImGui::StyleColorsDark();
	// ImGui::StyleColorsLight();
	// ImGui::StyleColorsClassic();
	
	ImGui_ImplOpenGL3_Init("#version 330 core");
	// OpenGL backend will be initialised only after setting up appropriate callbacks in App class.
	// That's because I'm using UserPointer that modifies GLFWwindow*.
	// ImGui_ImplGlfw_InitForOpenGL(m_window, true);
}

Window::Window() :Window(800, 600, "Window") { }

Window::~Window() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(m_window);
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

auto Window::change_cursor_mode(CursorMode a_mode) -> void {
	switch (a_mode) {
		case CursorMode::NORMAL:
			m_cur_mode = CursorMode::NORMAL;
			glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			break;
		case CursorMode::FIRST_PERSON:
			m_cur_mode = CursorMode::FIRST_PERSON;
			glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			break;
		case CursorMode::IDLE:
			m_cur_mode = CursorMode::IDLE;
			glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			break;
	}
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

CursorMode Window::get_cursor_mode() const {
	return m_cur_mode;
}

float Window::get_delta() const {
	return m_delta_time;
}

std::string Window::get_title() const {
	return m_title;
}

ImGuiIO* Window::get_io() const {
	return m_io;
}

GLFWwindow* Window::get_obj() const {
	return m_window;
}
