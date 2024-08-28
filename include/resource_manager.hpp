#pragma once

#include <map>
#include <string>

#include "buffers.hpp"
#include "shaders.hpp"


/*
Consists of:
	- TextureManager
	- ShaderManager
	- MeshManager (to avoid reloading vertex vectors)
*/
struct TextureKey {
	std::wstring name;
	bool flip_image;
}; 
auto operator==(const TextureKey& a_key1, const TextureKey& a_key2) -> bool;
auto operator!=(const TextureKey& a_key1, const TextureKey& a_key2) -> bool;
auto operator<(const TextureKey& a_key1, const TextureKey& a_key2)  -> bool;
auto operator>(const TextureKey& a_key1, const TextureKey& a_key2)  -> bool;
auto operator<=(const TextureKey& a_key1, const TextureKey& a_key2) -> bool;
auto operator>=(const TextureKey& a_key1, const TextureKey& a_key2) -> bool;

class ResourceManager {
public: 
	auto new_shader(const std::string& a_name, ShaderProgram& a_shader) -> void;
	auto new_shader(const std::string& a_name, std::initializer_list<ShaderSrc> a_shaders_src) -> void;
	auto get_shader(const std::string& a_name) -> ShaderProgram&;

	auto load_texture(std::wstring a_dir, TextureType a_type, bool a_flip_image, int a_unit_id) -> Texture;
	auto create_texture(int a_width, int a_height, TextureType a_type) -> Texture;
	auto chk_texture_ref_count(unsigned a_texture_id) -> bool;
	auto inc_texture_ref_count(unsigned a_texture_id) -> void;
	auto dec_texture_ref_count(unsigned a_texture_id) -> void;

private: 

	std::map<std::string, std::unique_ptr<ShaderProgram>> m_shaders; 

	std::map<unsigned, int> m_texture_ref_count;
	std::map<TextureKey, std::unique_ptr<Texture>> m_textures_cached;
};