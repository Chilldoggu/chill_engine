#include <filesystem>
#include <iostream>

#include "chill_renderer/resource_manager.hpp"
#include "chill_renderer/file_manager.hpp"

namespace chill_renderer {
namespace fs = std::filesystem;

// TODO: Async file manager
std::wstring ResourceManager::dialog_import_model() {
	std::vector<std::pair<std::wstring, std::wstring>> filters{
		{L"Wavefront (*.obj)", L"*.obj"},
		{L"All Files (*.*)",   L"*.*"},
	};

	return basic_file_open(L"Import model", filters);
}

ShaderProgram ResourceManager::new_shader(const ShaderSrc& a_vertex_shader, const ShaderSrc& a_fragment_shader, const ShaderSrc& a_geometry_shader) {
	const std::wstring& vert_path = a_vertex_shader.get_path();
	const std::wstring& frag_path = a_fragment_shader.get_path();
	const std::wstring& geom_path = a_geometry_shader.get_path();
	// Check if shader is cached.
	auto it = std::find_if(m_shaders_cached.cbegin(), m_shaders_cached.cend(),
		[&vert_path, &frag_path, &geom_path](const auto& elem) {
			ShaderProgram* cached_shader = elem.second.get();
			if (cached_shader != nullptr) {
				const std::wstring& cached_vert_path = cached_shader->get_vert_shader().get_path();
				const std::wstring& cached_frag_path = cached_shader->get_frag_shader().get_path();
				const std::wstring& cached_geom_path = cached_shader->get_geom_shader().get_path();
				if (cached_vert_path == vert_path && cached_frag_path == frag_path && cached_geom_path == geom_path)
					return true;
			}
			return false;
		});

	// If shader is not cached then cache it.
	if (it == m_shaders_cached.end()) {
		ShaderProgram a_shader_program(a_vertex_shader, a_fragment_shader, a_geometry_shader);
		m_shaders_cached[a_shader_program.get_id()] = std::make_unique<ShaderProgram>(a_shader_program);
		return *m_shaders_cached[a_shader_program.get_id()];
	}

	// Return cached shader.
	return *it->second;
}

Model ResourceManager::load_model(const std::wstring& a_path, bool a_flip_UVs, bool a_gamma_corr) {
	// Check if model is cached.
	std::wstring path = guess_path(a_path).wstring();
	auto it = std::find_if(m_models_cached.begin(), m_models_cached.end(),
		[&path, &a_flip_UVs, &a_gamma_corr](const auto& elem) {
			Model* cached_model = elem.second.get();
			return cached_model != nullptr && 
				   elem.first == path && 
				   cached_model->is_flipped() == a_flip_UVs &&
				   cached_model->is_gamma_corr() == a_gamma_corr;
		});

	// If model is not cached then cache it.
	if (it == m_models_cached.end()) {
		m_models_cached[path] = std::make_unique<Model>(a_path, a_flip_UVs, a_gamma_corr);
	}

	return *m_models_cached[path];
}

Model ResourceManager::create_model(const std::vector<Mesh>& a_meshes) {
	return Model(a_meshes);
}

Texture ResourceManager::load_texture(const std::wstring& a_path, TextureType a_type, int a_unit_id, bool a_flip_image, bool a_gamma_corr) {
	// Check if texture is cached.
	std::wstring filename = fs::path(a_path).filename().wstring();
	auto it = std::find_if(m_textures_cached.begin(), m_textures_cached.end(),
		[&filename, &a_flip_image, &a_gamma_corr](const auto& elem) {
			Texture* cached_texture = elem.second.get();

			return cached_texture != nullptr &&
				   cached_texture->get_filename() == filename &&
				   cached_texture->is_flipped() == a_flip_image &&
				   cached_texture->is_gamma_corr() == a_gamma_corr;
		});

	// If texture is not cached then cache it.
	if (it == m_textures_cached.end()) {
		Texture new_texture(a_path, a_type, a_unit_id, a_flip_image, a_gamma_corr);
		m_textures_cached[new_texture.get_id()] = std::make_unique<Texture>(new_texture);
		return *m_textures_cached[new_texture.get_id()];
	}

	// Texture is cached. Modify possibly wrong attributes to match user parameters.
	Texture cached_texture = *it->second;

	if (cached_texture.get_type() != a_type)
		cached_texture.set_type(a_type);

	if (cached_texture.get_unit_id() != a_unit_id)
		cached_texture.set_unit_id(a_unit_id);

	return cached_texture;
}

Texture ResourceManager::load_cubemap(const std::vector<std::wstring>& a_paths, int a_unit_id, bool a_flip_images, bool a_gamma_corr) {
	// Check if texture is cached.
	std::vector<std::wstring> filenames{};
	for (const auto& path : a_paths) {
		filenames.push_back(fs::path(path).filename().wstring());
	}
	std::wstring dir = guess_path(a_paths[0]).parent_path().wstring();

	auto it = std::find_if(m_textures_cached.begin(), m_textures_cached.end(),
		[&filenames, &dir, &a_flip_images, &a_gamma_corr](const auto& elem) {
			Texture* cached_texture = elem.second.get();

			if (cached_texture != nullptr && 
				cached_texture->get_path() != dir && 
				a_flip_images != cached_texture->is_flipped() &&
				a_gamma_corr != cached_texture->is_gamma_corr()) 
			{
				auto cached_filenames = cached_texture->get_filenames(); 
				if (cached_filenames.size() != 6)
					return false;

				// The right sequence of texture paths matters.
				for (size_t i = 0; i < 6; ++i) {
					if (filenames[i] != cached_filenames[i])
						return false;
				}

				// All checks positive
				return true;
			}

			return false;
		});

	// If texture is not cached then cache it.
	if (it == m_textures_cached.end()) {
		Texture new_texture(a_paths, a_unit_id, a_flip_images, a_gamma_corr);
		m_textures_cached[new_texture.get_id()] = std::make_unique<Texture>(new_texture);
		return *m_textures_cached[new_texture.get_id()];
	}

	// Texture is cached. Modify possibly wrong attributes to match user parameters.
	Texture cached_texture = *it->second;

	if (cached_texture.get_unit_id() != a_unit_id)
		cached_texture.set_unit_id(a_unit_id);

	return cached_texture; 
}

Texture ResourceManager::create_texture(int a_width, int a_height, TextureType a_type) {
	return Texture(a_width, a_height, a_type);
}

RenderBuffer ResourceManager::create_render_buffer(int a_width, int a_height, RenderBufferType a_type) {
	return RenderBuffer(a_width, a_height, a_type);
}

void ResourceManager::inc_ref_count(ResourceType a_res_type, GLuint a_id) noexcept {
	auto& res_ref_counter = m_ref_counter[a_res_type];

	auto it = res_ref_counter.find(a_id);
	if (it == res_ref_counter.end())
		res_ref_counter[a_id] = 0;
	res_ref_counter[a_id]++;
}

void ResourceManager::dec_ref_count(ResourceType a_res_type, GLuint a_id) noexcept {
	auto& res_ref_counter = m_ref_counter[a_res_type];

	auto it = res_ref_counter.find(a_id);
	if (it == res_ref_counter.end())
		return;
	res_ref_counter[a_id]--;
}

// 'True' - there are references to this texture elsewhere.
// 'False' - there are no references to this texture.
bool ResourceManager::chk_ref_count(ResourceType a_res_type, GLuint a_id) {
	auto& res_ref_counter = m_ref_counter[a_res_type];

	auto it = res_ref_counter.find(a_id);
	if (it == res_ref_counter.end())
		return false;

	int ref_count = it->second;
	if (ref_count <= 0) {
		return false;
	}

	// Check if the last referenced resource is cached, if so delete it.
	if (ref_count == 1) {
		if (a_res_type == ResourceType::SHADER_PROGRAMS) {
			return true;
			auto it = m_shaders_cached.find(a_id);
			ShaderProgram* cached_shader = (it == m_shaders_cached.end()) ? nullptr : it->second.get();
			if (cached_shader != nullptr && cached_shader->get_id() == a_id) {
				// Resetting should call ShaderProgram's destructor and decrement reference counter.
				it->second.reset(nullptr);
				return false;
			}
		}

		else if (a_res_type == ResourceType::TEXTURES) {
			auto it = m_textures_cached.find(a_id);
			Texture* cached_texture = (it == m_textures_cached.end()) ? nullptr : it->second.get();
			if (cached_texture != nullptr && cached_texture->get_id() == a_id) {
				// Resetting should call Texture's destructor and decrement reference counter.
				it->second.reset(nullptr);
				return false;
			}
		}
		// TODO: Deleting cached model is a bit performance heavy. Can be optimized by counting Model references and
		// implementing some id generation system. As long as lag is not visible it's good enough.
		// Find if there's a cached model that has this Mesh. If so delete whole Model.
		else if (a_res_type == ResourceType::MESHES) {
			unsigned mesh_id = a_id;
			auto it = std::find_if(m_models_cached.begin(), m_models_cached.end(),
				[mesh_id](const auto& elem) {
					if (elem.second != nullptr) {
						std::vector<Mesh>& cached_meshes = elem.second.get()->get_meshes();
						auto it = std::find_if(cached_meshes.begin(), cached_meshes.end(), [mesh_id](const Mesh& a_mesh) {
							return a_mesh.get_VAO() == mesh_id;
							});
						return it != cached_meshes.end();
					}
					return false;
				});
			if (it != m_models_cached.end()) {
				it->second.reset(nullptr);
				return false;
			}
		}
	}

	return true;
}

void ResourceManager::debug(ResourceType a_type) {
	for (auto& [id, cnt] : m_ref_counter[a_type]) {
		std::cout << std::format("[ID:{}, \tCOUNT:{}]", id, cnt) << '\n';
	}
	std::cout << '\n';
} 
}