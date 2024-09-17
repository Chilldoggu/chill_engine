#pragma once

#include <map>

#include "chill_engine/model.hpp"
#include "chill_engine/buffers.hpp"
#include "chill_engine/shaders.hpp"

namespace chill_engine {
enum class ResourceType {
	TEXTURES,
	SHADER_SRCS,
	SHADER_PROGRAMS,
	RENDER_BUFFERS,
	FRAME_BUFFERS,
	UNIFORM_BUFFERS,
	MESHES,
};

class ResourceManager {
public:
	static auto dialog_import_model() -> std::wstring;

	auto inc_ref_count(ResourceType a_res_type, GLuint a_id) -> void;
	auto dec_ref_count(ResourceType a_res_type, GLuint a_id) -> void;
	auto chk_ref_count(ResourceType a_res_type, GLuint a_id) -> bool;

	auto new_shader(const ShaderSrc& a_vertex_shader, const ShaderSrc& a_fragment_shader, const ShaderSrc& a_geometry_shader = ShaderSrc{}) -> ShaderProgram;

	auto load_model(const std::wstring& a_dir, bool a_flip_UVs = false) -> Model;
	auto create_model(const std::vector<Mesh>& a_meshes) -> Model;

	auto load_texture(const std::wstring& a_path, TextureType a_type, bool a_flip_image, int a_unit_id) -> Texture;
	auto load_cubemap(const std::vector<std::wstring>& a_paths, bool a_flip_images, int a_unit_id) -> Texture;
	auto create_texture(int a_width, int a_height, TextureType a_type) -> Texture;

	auto create_render_buffer(int a_width, int a_height, RenderBufferType a_type) -> RenderBuffer;

	auto debug() -> void;

private:
	std::map<GLuint, std::unique_ptr<ShaderProgram>> m_shaders_cached;
	std::map<GLuint, std::unique_ptr<Texture>> m_textures_cached;
	std::map<std::wstring, std::unique_ptr<Model>> m_models_cached;

	std::map<ResourceType, std::map<GLuint, int>> m_ref_counter;
}; 
}