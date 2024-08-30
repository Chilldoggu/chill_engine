#pragma once

#include <map>
#include <string>

#include "chill_engine/model.hpp"
#include "chill_engine/buffers.hpp"
#include "chill_engine/shaders.hpp"


/*
Consists of:
	- TextureManager
	- ShaderManager
	- MeshManager (to avoid reloading vertex vectors)
*/
enum class ResourceType {
	TEXTURES,
	SHADER_SRCS,
	SHADER_PROGRAMS,
	RENDER_BUFFERS,
	FRAME_BUFFERS,
	MESHES,
};

class ResourceManager {
public: 
	static auto dialog_import_model() -> std::wstring;

	auto inc_ref_count(ResourceType a_res_type, unsigned a_id) -> void;
	auto dec_ref_count(ResourceType a_res_type, unsigned a_id) -> void;
	auto chk_ref_count(ResourceType a_res_type, unsigned a_id) -> bool; 

	auto new_shader(std::string a_name, ShaderSrc a_vertex_shader, ShaderSrc a_fragment_shader) -> ShaderProgram;

	auto load_model(std::wstring a_dir, bool a_flip_UVs = false) -> Model;
	auto create_model(std::vector<Mesh> a_meshes) -> Model;

	auto load_texture(std::wstring a_path, TextureType a_type, bool a_flip_image, int a_unit_id) -> Texture;
	auto create_texture(int a_width, int a_height, TextureType a_type) -> Texture;
	
	auto debug() -> void;

private: 
	std::map<unsigned,     std::unique_ptr<ShaderProgram>> m_shaders_cached; 
	std::map<unsigned,     std::unique_ptr<Texture>> m_textures_cached;
	std::map<std::wstring, std::unique_ptr<Model>> m_models_cached;

	std::map<ResourceType, std::map<unsigned, int>> m_ref_counter;
};