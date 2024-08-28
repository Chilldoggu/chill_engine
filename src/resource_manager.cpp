#include "resource_manager.hpp"

bool operator==(const TextureKey& a_key1, const TextureKey& a_key2) {
	return (a_key1.flip_image == a_key2.flip_image && a_key1.name == a_key2.name);
}

bool operator!=(const TextureKey& a_key1, const TextureKey& a_key2) {
	return (a_key1.flip_image != a_key2.flip_image || a_key1.name != a_key2.name);
}

bool operator<(const TextureKey& a_key1, const TextureKey& a_key2) {
	return (a_key1.name < a_key2.name);
}

bool operator>(const TextureKey& a_key1, const TextureKey& a_key2) {
	return (a_key1.name > a_key2.name);
}

bool operator<=(const TextureKey& a_key1, const TextureKey& a_key2) {
	return (a_key1.name <= a_key2.name);
}

bool operator>=(const TextureKey& a_key1, const TextureKey& a_key2) {
	return (a_key1.name >= a_key2.name);
}

void ResourceManager::new_shader(const std::string& a_name, ShaderProgram& a_shader) {
	m_shaders[a_name] = std::make_unique<ShaderProgram>(a_shader);
	m_shaders[a_name]->set_name(a_name);
}

void ResourceManager::new_shader(const std::string& a_name, std::initializer_list<ShaderSrc> a_shaders_src) {
	m_shaders[a_name] = std::make_unique<ShaderProgram>(a_shaders_src);
	m_shaders[a_name]->set_name(a_name);
}

ShaderProgram& ResourceManager::get_shader(const std::string& a_name) {
	return *m_shaders[a_name];
}

Texture ResourceManager::load_texture(std::wstring a_dir, TextureType a_type, bool a_flip_image, int a_unit_id) {
	auto key = TextureKey(a_dir, a_flip_image);
	auto it = m_textures_cached.find(key);
	if (it == m_textures_cached.end())
		m_textures_cached[key] = std::make_unique<Texture>(a_dir, a_type, a_flip_image, a_unit_id);

	Texture cached_texture = *m_textures_cached[key];

	if (cached_texture.get_type() != a_type)
		cached_texture.set_type(a_type); 

	if (cached_texture.get_unit_id() != a_unit_id)
		cached_texture.set_unit_id(a_unit_id);

	return cached_texture;
}

Texture ResourceManager::create_texture(int a_width, int a_height, TextureType a_type) {
	return Texture(a_width, a_height, a_type); 
}

// 'True' - there are references to this texture elsewhere.
// 'False' - there are no references to this texture.
bool ResourceManager::chk_texture_ref_count(unsigned a_texture_id) {
	auto it = m_texture_ref_count.find(a_texture_id); 
	if (it == m_texture_ref_count.end())
		return false;
	
	int ref_count = (*it).second;
	if (ref_count <= 0) {
		return false;
	} else if (ref_count == 1) {
		// Check if the last referenced texture is in cached textures, if so delete it.
		for (auto& texture_cached : m_textures_cached) {
			if (texture_cached.second->get_id() == a_texture_id) {
				// Erasing should call Texture's destructor and decrement reference counter.
				m_textures_cached.erase(texture_cached.first);
				return false;
			}
		}
	}
	return true;
}

void ResourceManager::inc_texture_ref_count(unsigned a_texture_id) {
	auto it = m_texture_ref_count.find(a_texture_id);
	if (it == m_texture_ref_count.end())
		m_texture_ref_count[a_texture_id] = 1;
	m_texture_ref_count[a_texture_id]++; 
}

void ResourceManager::dec_texture_ref_count(unsigned a_texture_id) {
	auto it = m_texture_ref_count.find(a_texture_id);
	if (it == m_texture_ref_count.end())
		return;
	m_texture_ref_count[a_texture_id]--;
}
