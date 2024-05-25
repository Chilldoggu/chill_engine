#pragma once

#include <glm/glm.hpp>
#include "glm/fwd.hpp"

#include <map>
#include <string>

#include "window.hpp"
#include "camera.hpp"
#include "shaders.hpp"

class Window;

class Player {
public:
	Camera m_head;

	Player(glm::vec3 a_head_pos = glm::vec3(0.0f, 0.0f, 0.0f));
	auto get_pos() const -> glm::vec3;

private:
	glm::vec3 m_pos;
};


class Game {
public:
	Window m_win;
	Player m_player;

	Game(int win_width = 1280, int win_height = 720, std::string win_title = "OpenGL", MODES win_mode = MODES::FPS, glm::vec3 player_head_pos = glm::vec3(0.0f, 0.0f, 0.0f));
	~Game();
	auto new_shader(std::string a_name, Shader_program& a_shader) -> void;
	auto new_shader(std::string a_name, std::initializer_list<Shader_src> a_shaders_src) -> void;

	auto get_shader(std::string name) -> Shader_program&;
	auto do_test() -> void;

private:
	std::map<std::string, Shader_program*> m_shaders;
};
