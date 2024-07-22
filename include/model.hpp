#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <vector>

#include "meshes.hpp"
#include "shaders.hpp"

enum class Axis {
	X, Y, Z
};

class Model {
public:
	Model(std::string a_name);

	auto set_pos(glm::vec3 a_pos) -> void;
	auto set_size(float a_size) -> void;
	auto set_size(glm::vec3 a_size) -> void;
	auto move(glm::vec3 a_vec) -> void;
	auto rotate(float a_angle, Axis a_axis = Axis::X) -> void;
	auto draw(ShaderProgram &a_shader, std::string a_material_map_uniform_name = "") -> void;

	auto get_pos() const -> glm::vec3;
	auto get_size() const -> glm::vec3;
    auto get_dir() const -> std::string;
    auto get_name() const -> std::string;
	auto get_model_mat() const -> glm::mat4;
	auto get_normal_mat() const -> glm::mat3;

private:
	auto load_model() -> void;
	auto process_node(aiNode *a_node, const aiScene *a_scene) -> void;
	auto process_mesh(aiMesh *a_mesh, const aiScene *a_scene) -> Mesh;
	auto process_texture(std::vector<std::shared_ptr<Texture>>& a_textures, aiMaterial* a_mat, aiTextureType a_ai_tex_type, int a_unit_id = 0) -> void;

	glm::vec3 m_pos;
	glm::vec3 m_size;
	glm::mat4 m_transform_scale;
	glm::mat4 m_transform_rotation;
	glm::mat4 m_transform_pos;
	std::vector<Mesh> m_meshes;
	std::vector<std::shared_ptr<Texture>> m_textures_loaded;
	std::filesystem::path m_dir;
	std::filesystem::path m_name;
};
