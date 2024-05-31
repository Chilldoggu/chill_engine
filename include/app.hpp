#pragma once

#include <glm/glm.hpp>
#include "glm/fwd.hpp"

#include <map>
#include <memory>
#include <string>

#include "window.hpp"
#include "shaders.hpp"
#include "camera.hpp"

class App {
public:
	App(int win_width = 1280, int win_height = 720, std::string win_title = "OpenGL", CursorMode win_mode = CursorMode::NORMAL, glm::vec3 player_head_pos = glm::vec3(0.0f, 0.0f, 0.0f));
	~App();
	auto new_shader(std::string a_name, Shader_program& a_shader) -> void;
	auto new_shader(std::string a_name, std::initializer_list<Shader_src> a_shaders_src) -> void;
	auto framebuffer_size_callback(int width, int height) -> void;
    auto mouse_callback(double xpos, double ypos) -> void;

	auto get_win() -> Window&;
	auto get_cam() -> Camera&;
	auto get_shader(std::string name) -> Shader_program&;

private:
	std::unique_ptr<Window> m_win;
	std::unique_ptr<Camera> m_cam;
	std::map<std::string, Shader_program*> m_shaders;
};
