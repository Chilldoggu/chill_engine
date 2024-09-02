#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <vector>
#include <filesystem>

#include "chill_engine/meshes.hpp"
#include "chill_engine/shaders.hpp"

namespace chill_engine {
enum class Axis {
	X, Y, Z
};

class Model {
public:
	Model() = default;
	Model(const std::wstring& a_path, bool a_flip_UVs = false);
	Model(const std::vector<Mesh>& a_meshes);

	void load_model(const std::wstring& a_path, bool a_flip_UVs);
	auto set_pos(const glm::vec3& a_pos) -> void;
	auto set_size(float a_size) -> void;
	auto set_size(const glm::vec3& a_size) -> void;
	auto set_rotation(const glm::vec3& a_rotation) -> void;
	auto set_meshes(const std::vector<Mesh>& a_meshes) -> void;
	auto move(const glm::vec3& a_vec) -> void;
	auto rotate(float a_angle, Axis a_axis = Axis::X) -> void;

	auto draw() -> void;
	auto draw(ShaderProgram& a_shader, const std::string& a_material_map_uniform_name) -> void;
	auto draw_outlined(float a_thickness, ShaderProgram& a_object_shader, ShaderProgram& a_outline_shader, const std::string& a_model_uniform_name, const std::string& a_material_map_uniform_name) -> void;
	auto clear() -> void;

	auto get_pos() const -> glm::vec3;
	auto get_size() const -> glm::vec3;
	auto get_rotation() const -> glm::vec3;
	auto get_dir() const -> std::wstring;
	auto get_path() const -> std::wstring;
	auto get_filename() const -> std::wstring;
	auto get_meshes() -> std::vector<Mesh>&;
	auto get_model_mat() const -> glm::mat4;
	auto get_normal_mat() const -> glm::mat3;
	auto is_flipped() const -> bool;

private:
	auto process_node(aiNode* a_node, const aiScene* a_scene) -> void;
	auto process_mesh(aiMesh* a_mesh, const aiScene* a_scene) -> Mesh;
	auto process_texture(std::vector<Texture>& a_textures, aiMaterial* a_mat, aiTextureType a_ai_texture_type) -> void;

	bool m_flipped_UVs = false;
	glm::vec3 m_pos = glm::vec3(0.0f);
	glm::vec3 m_size = glm::vec3(1.0f);
	glm::vec3 m_rotation = glm::vec3(0.0f);
	glm::mat4 m_transform_pos = 1.0f;
	glm::mat4 m_transform_scale = 1.0f;
	glm::mat4 m_transform_rotation = 1.0f;
	std::vector<Mesh> m_meshes;
	std::wstring m_path = L"";
	std::wstring m_dir = L"";
	std::wstring m_filename = L"";
}; 
}