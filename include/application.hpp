#pragma once

#include <glm/glm.hpp>
#include "glm/fwd.hpp"

#include <map>
#include <memory>
#include <string>

#include "window.hpp"
#include "shaders.hpp"
#include "resource_manager.hpp"

class Application {
public:
	Application(const Application&) = delete;
	Application& operator=(const Application&) = delete; 

	static auto init(int win_width = 1280, int win_height = 720, std::string win_title = "OpenGL_App", CursorMode win_mode = CursorMode::NORMAL) -> Application&;
	static auto get_instance() -> Application&;
	static auto shutdown() -> void; 

	auto get_win() -> Window&;
	auto get_rmanager() -> ResourceManager&;

private:
	Application(int win_width, int win_height, std::string win_title, CursorMode win_mode);
	~Application();

private:
	std::unique_ptr<Window> m_win = nullptr;
	std::unique_ptr<ResourceManager> m_rmanager = nullptr;
};
