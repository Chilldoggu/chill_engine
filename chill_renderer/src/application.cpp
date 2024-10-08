#include "chill_renderer/application.hpp"

namespace chill_renderer {
static Application* s_instance = nullptr;

Application& Application::init(int win_width, int win_height, const std::string& win_title, CursorMode win_mode) {
	if (!s_instance)
		s_instance = new Application(win_width, win_height, win_title, win_mode);
	return *s_instance;
}

Application& Application::get_instance() {
	if (!s_instance)
		ERROR("[APPLICATION::GET_INSTANCE] Application haven't been inistialied.", Error_action::throwing);

	return *s_instance;
}

void Application::shutdown() {
	delete& get_instance();
}

Application::Application(int win_width, int win_height, const std::string& win_title, CursorMode win_mode)
	:m_win{ new Window(win_width, win_height, win_title, win_mode) }, m_rmanager{ new ResourceManager() } { }

Application::~Application() {
	delete& get_instance();
}

Window& Application::get_win() noexcept {
	return *m_win;
}

ResourceManager& Application::get_rmanager() noexcept {
	return *m_rmanager;
} 
}