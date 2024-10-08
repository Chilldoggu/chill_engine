#pragma once

#include <memory>
#include <string>

#include "chill_renderer/window.hpp"
#include "chill_renderer/resource_manager.hpp"

namespace chill_renderer {
class Application {
public:
	Application(const Application&) = delete;
	Application& operator=(const Application&) = delete;

	static auto init(int win_width = 1280, int win_height = 720, const std::string& win_title = "OpenGL_App", CursorMode win_mode = CursorMode::NORMAL) -> Application&;
	static auto get_instance() -> Application&;
	static auto shutdown() -> void;

	auto get_win() noexcept -> Window&;
	auto get_rmanager() noexcept -> ResourceManager&;
private:
	Application(int win_width, int win_height, const std::string& win_title, CursorMode win_mode);
	~Application();

private:
	std::unique_ptr<Window> m_win = nullptr;
	std::unique_ptr<ResourceManager> m_rmanager = nullptr;
}; 
}