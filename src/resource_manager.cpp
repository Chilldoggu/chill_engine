#include <filesystem>

#include "chill_engine/resource_manager.hpp"
#include "chill_engine/file_manager.hpp"

namespace chill_engine {
namespace fs = std::filesystem;

// TODO: Make it more general or whatever.
std::wstring ResourceManager::dialog_import_model() {
	std::vector<std::pair<std::wstring, std::wstring>> filters{
		{L"Wavefront (*.obj)", L"*.obj"},
		{L"All Files (*.*)",   L"*.*"},
	};

	return basic_file_open(L"Import model", filters);
}

ShaderProgram ResourceManager::new_shader(std::string a_name, const ShaderSrc& a_vertex_shader, const ShaderSrc& a_fragment_shader) {
	// Check if shader is cached.
	auto it = std::find_if(m_shaders_cached.begin(), m_shaders_cached.end(),
		[&a_name](auto& elem) {
			ShaderProgram* cached_shader = elem.second.get();
			return (cached_shader != nullptr && cached_shader->get_name() == a_name);
		});

	// If shader is not cached then cache it.
	if (it == m_shaders_cached.end()) {
		ShaderProgram a_shader_program(a_name, a_vertex_shader, a_fragment_shader);
		m_shaders_cached[a_shader_program.get_id()] = std::make_unique<ShaderProgram>(a_shader_program);
		return *m_shaders_cached[a_shader_program.get_id()];
	}

	// Return cached shader.
	return *(*it).second;
}

Model ResourceManager::load_model(const std::wstring& a_path, bool a_flip_UVs) {
	// Check if model is cached.
	std::wstring filename = fs::path(a_path).filename().wstring();
	auto it = std::find_if(m_models_cached.begin(), m_models_cached.end(),
		[&filename, a_flip_UVs](auto& elem) {
			Model* cached_model = elem.second.get();
			return (cached_model != nullptr && elem.first == filename && cached_model->is_flipped() == a_flip_UVs);
		});

	// If model is not cached then cache it.
	if (it == m_models_cached.end()) {
		Model new_model(a_path, a_flip_UVs);
		m_models_cached[filename] = std::make_unique<Model>(new_model);
		return *m_models_cached[filename];
	}

	return *m_models_cached[filename];
}

Model ResourceManager::create_model(const std::vector<Mesh>& a_meshes) {
	return Model(a_meshes);
}

Texture ResourceManager::load_texture(const std::wstring& a_path, TextureType a_type, bool a_flip_image, int a_unit_id) {
	// Check if texture is cached.
	std::wstring filename = fs::path(a_path).filename().wstring();
	auto it = std::find_if(m_textures_cached.begin(), m_textures_cached.end(),
		[&filename, a_flip_image](auto& elem) {
			Texture* cached_texture = elem.second.get();
			return (cached_texture != nullptr && cached_texture->get_filename() == filename && cached_texture->is_flipped() == a_flip_image);
		});

	// If texture is not cached then cache it.
	if (it == m_textures_cached.end()) {
		Texture new_texture(a_path, a_type, a_flip_image, a_unit_id);
		m_textures_cached[new_texture.get_id()] = std::make_unique<Texture>(new_texture);
		return *m_textures_cached[new_texture.get_id()];
	}

	// Texture is cached. Modify possibly wrong attributes to match user parameters.
	auto id = (*it).second->get_id();
	Texture cached_texture = *m_textures_cached[id];

	if (cached_texture.get_type() != a_type)
		cached_texture.set_type(a_type);

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

void ResourceManager::inc_ref_count(ResourceType a_res_type, GLuint a_id) {
	std::map<unsigned, int>& res_ref_counter = m_ref_counter[a_res_type];

	auto it = res_ref_counter.find(a_id);
	if (it == res_ref_counter.end())
		res_ref_counter[a_id] = 0;
	res_ref_counter[a_id]++;
}

void ResourceManager::dec_ref_count(ResourceType a_res_type, GLuint a_id) {
	std::map<unsigned, int>& res_ref_counter = m_ref_counter[a_res_type];

	auto it = res_ref_counter.find(a_id);
	if (it == res_ref_counter.end())
		return;
	res_ref_counter[a_id]--;
}

// 'True' - there are references to this texture elsewhere.
// 'False' - there are no references to this texture.
bool ResourceManager::chk_ref_count(ResourceType a_res_type, GLuint a_id) {
	std::map<unsigned, int>& res_ref_counter = m_ref_counter[a_res_type];

	auto it = res_ref_counter.find(a_id);
	if (it == res_ref_counter.end())
		return false;

	int ref_count = (*it).second;
	if (ref_count <= 0) {
		return false;
	}

	// Check if the last referenced resource is cached, if so delete it.
	if (ref_count == 1) {
		if (a_res_type == ResourceType::SHADER_PROGRAMS) {
			return true;
			auto it = m_shaders_cached.find(a_id);
			ShaderProgram* cached_shader = (it == m_shaders_cached.end()) ? nullptr : (*it).second.get();
			if (cached_shader != nullptr && cached_shader->get_id() == a_id) {
				// Resetting should call ShaderProgram's destructor and decrement reference counter.
				(*it).second.reset(nullptr);
				return false;
			}
		}

		else if (a_res_type == ResourceType::TEXTURES) {
			auto it = m_textures_cached.find(a_id);
			Texture* cached_texture = (it == m_textures_cached.end()) ? nullptr : (*it).second.get();
			if (cached_texture != nullptr && cached_texture->get_id() == a_id) {
				// Resetting should call Texture's destructor and decrement reference counter.
				(*it).second.reset(nullptr);
				return false;
			}
		}
		// TODO: Deleting cached model is a bit performance heavy. Can be optimized by counting Model references and
		// implementing some id generation system. As long as lag is not visible it's good enough.
		// Find if there's a cached model that has this Mesh. If so delete whole Model.
		else if (a_res_type == ResourceType::MESHES) {
			unsigned mesh_id = a_id;
			auto it = std::find_if(m_models_cached.begin(), m_models_cached.end(),
				[mesh_id](auto& elem) {
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
				(*it).second.reset(nullptr);
				return false;
			}
		}
	}

	return true;
}

void ResourceManager::debug() {
	//for (auto& x : m_ref_counter[ResourceType::TEXTURES]) {
	//	std::wstring ret_ws{ L"" };
	//	unsigned ret_id{x.first};
	//	int ret_cnt{x.second};
	//	for (auto& y : m_textures_cached) {
	//		if (y.second != nullptr && y.second->get_id() == ret_id) {
	//			ret_ws = y.second->get_filename();
	//			break;
	//		}
	//	}
	//	std::cout << std::format("[COUNT:{}, \tID:{}, \tFILE: {}]", ret_cnt, ret_id, wstos(ret_ws)) << '\n';
	//}
	//std::cout << '\n';

	for (auto& x : m_ref_counter[ResourceType::MESHES]) {
		unsigned ret_id{ x.first };
		int ret_cnt{ x.second };
		std::cout << std::format("[COUNT:{}, \tID:{}]", ret_cnt, ret_id) << '\n';
	}
	std::cout << '\n';
} 
}