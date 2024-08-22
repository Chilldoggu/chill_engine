#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <vector>
#include <filesystem>

#include "meshes.hpp"
#include "shaders.hpp"


enum class Axis {
	X, Y, Z
};

class Model {
public:
	Model();
	Model(std::wstring a_dir, bool a_flip_UVs = false);
	Model(std::vector<Mesh> a_meshes);

	auto set_pos(glm::vec3 a_pos) -> void;
	auto set_size(float a_size) -> void;
	auto set_size(glm::vec3 a_size) -> void;
	auto set_rotation(glm::vec3 a_rotation) -> void;
	auto move(glm::vec3 a_vec) -> void;
	auto rotate(float a_angle, Axis a_axis = Axis::X) -> void;
	auto draw(ShaderProgram &a_shader, std::string a_material_map_uniform_name = "") -> void;
	auto draw_outlined(float a_thickness, ShaderProgram &a_object_shader, ShaderProgram& a_outline_shader, std::string a_model_uniform_name, std::string a_material_map_uniform_name) -> void;
	auto clear_model() -> void; 
	auto load_model(std::filesystem::path a_model_path = L"", bool a_flip_UVs = false) -> bool;

	auto get_pos() const -> glm::vec3;
	auto get_size() const -> glm::vec3;
	auto get_rotation() const -> glm::vec3; 
	auto get_dir() const -> std::wstring;
	auto get_name() const -> std::wstring;
	auto get_meshes() -> std::vector<Mesh>&; // WARNING: returning reference. Be cautious.
	auto get_model_mat() const -> glm::mat4;
	auto get_normal_mat() const -> glm::mat3;

private:
	auto process_node(aiNode *a_node, const aiScene *a_scene) -> void;
	auto process_mesh(aiMesh *a_mesh, const aiScene *a_scene) -> Mesh;
	auto process_texture(std::vector<std::shared_ptr<Texture>>& a_textures, aiMaterial* a_mat, aiTextureType a_ai_texture_type) -> void;

	glm::vec3 m_pos      = glm::vec3(0.0f);
	glm::vec3 m_size     = glm::vec3(1.0f);
	glm::vec3 m_rotation = glm::vec3(0.0f);
	glm::mat4 m_transform_pos      = 1.0f;
	glm::mat4 m_transform_scale    = 1.0f;
	glm::mat4 m_transform_rotation = 1.0f;
	std::vector<Mesh> m_meshes;
	std::vector<std::shared_ptr<Texture>> m_textures_loaded;
	std::filesystem::path m_dir = L"";
	std::filesystem::path m_name = L"";
};
