#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <format>

#include "chill_renderer/window.hpp"
#include "chill_renderer/assert.hpp"

namespace chill_renderer {
static void glfw_error_callback(int error, const char* description) {
	ERROR(std::format("[GLFW_ERROR_CALLBACK] GLFW Error {}: {}", error, description), Error_action::throwing);
}

InputHandler::InputHandler(GLFWwindow* a_window) : m_window{ a_window } {
	// Ensure that glfwSetWindowUserPointer has been called before creating InputHandler.
	auto size_callback = [](GLFWwindow* w, int width, int height) {
		Window* p_win = static_cast<Window*>((Window*)glfwGetWindowUserPointer(w));
		p_win->framebuffer_size_callback(width, height);
		};

	glfwSetFramebufferSizeCallback(a_window, size_callback);
	glfwSetErrorCallback(glfw_error_callback);
}

void InputHandler::process_input() const {
	if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(m_window, GLFW_TRUE);
}

ImGuiHandler::ImGuiHandler(GLFWwindow* a_window) :m_window{ a_window } {
	// Init Imgui
	IMGUI_CHECKVERSION();

	ImGui::CreateContext();

	m_io = &ImGui::GetIO();
	m_io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	m_io->ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	ImGui::StyleColorsDark();
	set_imgui_dracula_style();

	ImGui_ImplOpenGL3_Init("#version 330 core");

	ImGui_ImplGlfw_InitForOpenGL(a_window, true);
}

Window::Window(int a_width, int a_height, const std::string& a_title, CursorMode a_mode)
	:m_width{ a_width }, m_height{ a_height }, m_title{ a_title }, m_mouse_pos_x{ a_width / 2.f }, m_mouse_pos_y{ a_height / 2.f }, m_cur_mode{ a_mode }
{
	// Init Window
	if (!glfwInit())
		ERROR("[WINDOW::WINDOW] Couldn't initialise glfw.", Error_action::throwing);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	m_window = glfwCreateWindow(m_width, m_height, m_title.c_str(), nullptr, nullptr);
	if (m_window == nullptr) {
		ERROR("[WINDOW::WINDOW] Couldn't create a window.", Error_action::throwing);
	}

	glfwSetWindowUserPointer(m_window, this);
	glfwMakeContextCurrent(m_window);

	set_cursor_mode(a_mode);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		ERROR("[WINDOW:WINDOW] Couldn't load glad function pointers.", Error_action::throwing);
	}
	glViewport(0, 0, m_width, m_height);

	// After having window correctly initialised, setup callbacks and imgui
	m_camera = std::make_unique<Camera>(m_window);
	m_input_handle = std::make_unique<InputHandler>(m_window);
	m_imgui_handle = std::make_unique<ImGuiHandler>(m_window);
}

Window::~Window() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(m_window);
	glfwTerminate();
}

void Window::mouse_callback(double x_pos, double y_pos) noexcept {
	float x_offset = x_pos - get_mouse_x();
	float y_offset = get_mouse_y() - y_pos;

	set_mouse_x(x_pos);
	set_mouse_y(y_pos);

	if (m_cur_mode == CursorMode::FOCUSED)
		m_camera->process_mouse_movement(x_offset, y_offset);
}

void Window::framebuffer_size_callback(int width, int height) noexcept {
	glViewport(0, 0, width, height);
	set_width(width);
	set_height(height);
}

bool Window::closed() {
	return glfwWindowShouldClose(m_window) == GLFW_TRUE;
}

void Window::title_lshift(int n) {
	int siz = m_title.size();
	n %= siz;

	if (n == 0) return;

	m_title = m_title.substr(n, siz) + m_title.substr(0, n);
	title_change();
}

void Window::title_rshift(int n) {
	int siz = m_title.size();
	n %= siz;

	if (n == 0) return;

	m_title = m_title.substr(siz - n, siz) + m_title.substr(0, siz - n);
	title_change();
}

void Window::title_change() {
	glfwSetWindowTitle(m_window, m_title.c_str());
}

void Window::set_width(float width) noexcept {
	m_width = width;
}

void Window::set_height(float height) noexcept {
	m_height = height;
}

void Window::set_mouse_x(float x_pos) noexcept {
	m_mouse_pos_x = x_pos;
}

void Window::set_mouse_y(float y_pos) noexcept {
	m_mouse_pos_y = y_pos;
}

void Window::set_cursor_mode(CursorMode a_mode) noexcept {
	m_cur_mode = a_mode;
	switch (a_mode) {
	case CursorMode::NORMAL:  glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL); break;
	case CursorMode::FOCUSED: glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); break;
	}
}

float Window::calculate_delta() noexcept {
	m_current_frame = glfwGetTime();
	m_delta_time = m_current_frame - m_last_frame;
	m_last_frame = m_current_frame;
	return m_delta_time;
}

int Window::get_width() const noexcept {
	return m_width;
}

int Window::get_height() const noexcept {
	return m_height;
}

float Window::get_mouse_x() const noexcept {
	return m_mouse_pos_x;
}

float Window::get_mouse_y() const noexcept {
	return m_mouse_pos_y;
}

CursorMode Window::get_cursor_mode() const noexcept {
	return m_cur_mode;
}

float Window::get_aspect_ratio() const noexcept {
	return (m_width == 0) ? 0.f : float(m_height) / m_width;
}

float Window::get_delta() const noexcept {
	return m_delta_time;
}

std::string Window::get_title() const noexcept {
	return m_title;
}

GLFWwindow* Window::get_obj() const noexcept {
	return m_window;
}

Camera& Window::get_camera() const {
	return *m_camera;
}

InputHandler& Window::get_input_handle() const {
	return *m_input_handle;
}

ImGuiHandler& Window::get_imgui_handle() const {
	return *m_imgui_handle;
}

ImGuiIO* ImGuiHandler::get_io() const {
	return m_io;
}

// Thank you: https://github.com/ocornut/imgui/issues/1537#issuecomment-355562097
void ToggleButton(const char* str_id, bool* v) {
	enum Colors : ImU32 {
		on_norm = IM_COL32(0xBD, 0x94, 0xFA, 0xFF),
		on_hover = IM_COL32(0xBD + 20, 0x94 + 20, 0xFA, 0xFF),
		off_norm = IM_COL32(0x4F, 0x4F, 0x4F, 0xFF),
		off_hover = IM_COL32(0x4F + 20, 0x4F + 20, 0x4F + 20, 0xFF),
	};
	ImVec2 p = ImGui::GetCursorScreenPos();
	ImDrawList* draw_list = ImGui::GetWindowDrawList();

	float height = ImGui::GetFrameHeight();
	float width = height * 1.55f;
	float radius = height * 0.50f;

	if (ImGui::InvisibleButton(str_id, ImVec2(width, height)))
		*v = !*v;
	ImU32 col_bg;
	if (ImGui::IsItemHovered())
		col_bg = *v ? Colors::on_hover : Colors::off_hover;
	else
		col_bg = *v ? Colors::on_norm : Colors::off_norm;

	draw_list->AddRectFilled(p, ImVec2(p.x + width, p.y + height), col_bg, height * 0.5f);
	draw_list->AddCircleFilled(ImVec2(*v ? (p.x + width - radius) : (p.x + radius), p.y + radius), radius - 1.5f, IM_COL32(255, 255, 255, 255));
}

// Thank you: https://github.com/ocornut/imgui/issues/707#issuecomment-1372640066
void ImGuiHandler::set_imgui_dracula_style() {
	auto& colors = ImGui::GetStyle().Colors;
	colors[ImGuiCol_WindowBg] = ImVec4{ 0.1f, 0.1f, 0.13f, 1.0f };
	colors[ImGuiCol_MenuBarBg] = ImVec4{ 0.16f, 0.16f, 0.21f, 1.0f };

	// Border
	colors[ImGuiCol_Border] = ImVec4{ 0.44f, 0.37f, 0.61f, 0.29f };
	colors[ImGuiCol_BorderShadow] = ImVec4{ 0.0f, 0.0f, 0.0f, 0.24f };

	// Text
	colors[ImGuiCol_Text] = ImVec4{ 1.0f, 1.0f, 1.0f, 1.0f };
	colors[ImGuiCol_TextDisabled] = ImVec4{ 0.5f, 0.5f, 0.5f, 1.0f };

	// Headers
	colors[ImGuiCol_Header] = ImVec4{ 0.13f, 0.13f, 0.17, 1.0f };
	colors[ImGuiCol_HeaderHovered] = ImVec4{ 0.19f, 0.2f, 0.25f, 1.0f };
	colors[ImGuiCol_HeaderActive] = ImVec4{ 0.16f, 0.16f, 0.21f, 1.0f };

	// Buttons
	colors[ImGuiCol_Button] = ImVec4{ 0.13f, 0.13f, 0.17, 1.0f };
	colors[ImGuiCol_ButtonHovered] = ImVec4{ 0.19f, 0.2f, 0.25f, 1.0f };
	colors[ImGuiCol_ButtonActive] = ImVec4{ 0.16f, 0.16f, 0.21f, 1.0f };
	colors[ImGuiCol_CheckMark] = ImVec4{ 0.74f, 0.58f, 0.98f, 1.0f };

	// Popups
	colors[ImGuiCol_PopupBg] = ImVec4{ 0.1f, 0.1f, 0.13f, 0.92f };

	// Slider
	colors[ImGuiCol_SliderGrab] = ImVec4{ 0.44f, 0.37f, 0.61f, 0.54f };
	colors[ImGuiCol_SliderGrabActive] = ImVec4{ 0.74f, 0.58f, 0.98f, 0.54f };

	// Frame BG
	colors[ImGuiCol_FrameBg] = ImVec4{ 0.13f, 0.13, 0.17, 1.0f };
	colors[ImGuiCol_FrameBgHovered] = ImVec4{ 0.19f, 0.2f, 0.25f, 1.0f };
	colors[ImGuiCol_FrameBgActive] = ImVec4{ 0.16f, 0.16f, 0.21f, 1.0f };

	// Tabs
	colors[ImGuiCol_Tab] = ImVec4{ 0.16f, 0.16f, 0.21f, 1.0f };
	colors[ImGuiCol_TabHovered] = ImVec4{ 0.24, 0.24f, 0.32f, 1.0f };
	colors[ImGuiCol_TabActive] = ImVec4{ 0.2f, 0.22f, 0.27f, 1.0f };
	colors[ImGuiCol_TabUnfocused] = ImVec4{ 0.16f, 0.16f, 0.21f, 1.0f };
	colors[ImGuiCol_TabUnfocusedActive] = ImVec4{ 0.16f, 0.16f, 0.21f, 1.0f };

	// Title
	colors[ImGuiCol_TitleBg] = ImVec4{ 0.16f, 0.16f, 0.21f, 1.0f };
	colors[ImGuiCol_TitleBgActive] = ImVec4{ 0.16f, 0.16f, 0.21f, 1.0f };
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4{ 0.16f, 0.16f, 0.21f, 1.0f };

	// Scrollbar
	colors[ImGuiCol_ScrollbarBg] = ImVec4{ 0.1f, 0.1f, 0.13f, 1.0f };
	colors[ImGuiCol_ScrollbarGrab] = ImVec4{ 0.16f, 0.16f, 0.21f, 1.0f };
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4{ 0.19f, 0.2f, 0.25f, 1.0f };
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4{ 0.24f, 0.24f, 0.32f, 1.0f };

	// Seperator
	colors[ImGuiCol_Separator] = ImVec4{ 0.44f, 0.37f, 0.61f, 1.0f };
	colors[ImGuiCol_SeparatorHovered] = ImVec4{ 0.74f, 0.58f, 0.98f, 1.0f };
	colors[ImGuiCol_SeparatorActive] = ImVec4{ 0.84f, 0.58f, 1.0f, 1.0f };

	// Resize Grip
	colors[ImGuiCol_ResizeGrip] = ImVec4{ 0.44f, 0.37f, 0.61f, 0.29f };
	colors[ImGuiCol_ResizeGripHovered] = ImVec4{ 0.74f, 0.58f, 0.98f, 0.29f };
	colors[ImGuiCol_ResizeGripActive] = ImVec4{ 0.84f, 0.58f, 1.0f, 0.29f };

	// Docking
	// colors[ImGuiCol_DockingPreview] = ImVec4{ 0.44f, 0.37f, 0.61f, 1.0f };

	auto& style = ImGui::GetStyle();
	style.TabRounding = 4;
	style.ScrollbarRounding = 9;
	style.WindowRounding = 7;
	style.GrabRounding = 3;
	style.FrameRounding = 3;
	style.PopupRounding = 4;
	style.ChildRounding = 4;
}

// Thank you: https://github.com/ocornut/imgui/issues/707#issuecomment-917151020
void ImGuiHandler::set_imgui_darkness_style() {
	ImVec4* colors = ImGui::GetStyle().Colors;
	colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
	colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
	colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.19f, 0.19f, 0.19f, 0.92f);
	colors[ImGuiCol_Border] = ImVec4(0.19f, 0.19f, 0.19f, 0.29f);
	colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.24f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.19f, 0.19f, 0.19f, 0.54f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
	colors[ImGuiCol_TitleBg] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.34f, 0.34f, 0.34f, 0.54f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40f, 0.40f, 0.40f, 0.54f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.56f, 0.56f, 0.56f, 0.54f);
	colors[ImGuiCol_CheckMark] = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
	colors[ImGuiCol_SliderGrab] = ImVec4(0.34f, 0.34f, 0.34f, 0.54f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.56f, 0.56f, 0.56f, 0.54f);
	colors[ImGuiCol_Button] = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.19f, 0.19f, 0.19f, 0.54f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
	colors[ImGuiCol_Header] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.00f, 0.00f, 0.00f, 0.36f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.20f, 0.22f, 0.23f, 0.33f);
	colors[ImGuiCol_Separator] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
	colors[ImGuiCol_SeparatorActive] = ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
	colors[ImGuiCol_ResizeGrip] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
	colors[ImGuiCol_Tab] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
	colors[ImGuiCol_TabHovered] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
	colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.20f, 0.20f, 0.36f);
	colors[ImGuiCol_TabUnfocused] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
	colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
	// colors[ImGuiCol_DockingPreview]         = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
	// colors[ImGuiCol_DockingEmptyBg]         = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotLines] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotHistogram] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_TableHeaderBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
	colors[ImGuiCol_TableBorderStrong] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
	colors[ImGuiCol_TableBorderLight] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
	colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
	colors[ImGuiCol_DragDropTarget] = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
	colors[ImGuiCol_NavHighlight] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 0.00f, 0.00f, 0.70f);
	colors[ImGuiCol_NavWindowingDimBg] = ImVec4(1.00f, 0.00f, 0.00f, 0.20f);
	colors[ImGuiCol_ModalWindowDimBg] = ImVec4(1.00f, 0.00f, 0.00f, 0.35f);

	ImGuiStyle& style = ImGui::GetStyle();
	style.WindowPadding = ImVec2(8.00f, 8.00f);
	style.FramePadding = ImVec2(5.00f, 2.00f);
	style.CellPadding = ImVec2(6.00f, 6.00f);
	style.ItemSpacing = ImVec2(6.00f, 6.00f);
	style.ItemInnerSpacing = ImVec2(6.00f, 6.00f);
	style.TouchExtraPadding = ImVec2(0.00f, 0.00f);
	style.IndentSpacing = 25;
	style.ScrollbarSize = 15;
	style.GrabMinSize = 10;
	style.WindowBorderSize = 1;
	style.ChildBorderSize = 1;
	style.PopupBorderSize = 1;
	style.FrameBorderSize = 1;
	style.TabBorderSize = 1;
	style.WindowRounding = 7;
	style.ChildRounding = 4;
	style.FrameRounding = 3;
	style.PopupRounding = 4;
	style.ScrollbarRounding = 9;
	style.GrabRounding = 3;
	style.LogSliderDeadzone = 4;
	style.TabRounding = 4;
} 
}