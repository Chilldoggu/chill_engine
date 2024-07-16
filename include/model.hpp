#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <vector>

#include "meshes.hpp"
#include "shaders.hpp"

class Model {
public:
	Model(std::string a_name);

	auto set_pos(glm::vec3 a_pos) -> void;
	auto set_size(float a_size) -> void;
	auto draw(ShaderProgram &a_shader, std::string a_material_map_uniform_name) -> void;

    auto get_dir() const -> std::string;
    auto get_name() const -> std::string;
	auto get_model_mat() const -> glm::mat4;
	auto get_normal_mat() const -> glm::mat3;

private:
	auto load_model() -> void;
	auto process_node(aiNode *a_node, const aiScene *a_scene) -> void;
	auto process_mesh(aiMesh *a_mesh, const aiScene *a_scene) -> Mesh;

	glm::mat4 m_transform_scale;
	glm::mat4 m_transform_rotation;
	glm::mat4 m_transform_pos;
	std::vector<Mesh> m_meshes;
	std::vector<std::shared_ptr<Texture>> m_textures_loaded;
	std::filesystem::path m_dir;
	std::filesystem::path m_name;
};
