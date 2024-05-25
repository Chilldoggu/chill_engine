#include "game.hpp"

Player::Player(glm::vec3 a_head_pos) :m_head{ a_head_pos }
{  }

glm::vec3 Player::get_pos() const {
	return m_head.get_position();
}

/* ================ */

Game::Game(int win_width, int win_height, std::string win_title, MODES win_mode, glm::vec3 player_head_pos) 
	:m_win{ win_width, win_height, win_title, win_mode }, m_player{ player_head_pos }
{  }

Game::~Game() {
	for (auto& i : m_shaders)
		delete i.second;
}

void Game::new_shader(std::string a_name, Shader_program& a_shader) {
	m_shaders[a_name] = new Shader_program(a_shader);
	m_shaders[a_name]->set_name(a_name);
}

void Game::new_shader(std::string a_name, std::initializer_list<Shader_src> a_shaders_src) {
	m_shaders[a_name] = new Shader_program(a_shaders_src);
	m_shaders[a_name]->set_name(a_name);
}

Shader_program& Game::get_shader(std::string name) {
	return *(m_shaders[name]);
}

void Game::do_test() {
}
