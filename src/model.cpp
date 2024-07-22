#include <algorithm>
#include <assimp/material.h>
#include <assimp/postprocess.h>
#include <format>
#include <memory>

#include "model.hpp"
#include "meshes.hpp"

namespace fs = std::filesystem;

extern fs::path get_proj_path();

Model::Model(std::string a_dir) 
	:m_transform_pos{ 1.0f }, m_transform_scale{ 1.0f }, m_transform_rotation{ 1.0f }, m_pos{ 0.0 }, m_size{ 1.0 }
{
	m_dir = fs::path(a_dir).parent_path();
	m_name = fs::path(a_dir).filename();
	load_model();
}

void Model::set_pos(glm::vec3 a_pos) {
	m_pos = a_pos;
	m_transform_pos = glm::mat4(1.0f);
	m_transform_pos = glm::translate(m_transform_pos, a_pos);
}

void Model::set_size(float a_size) {
	m_size = glm::vec3(a_size);
	m_transform_scale = glm::mat4(1.0f);
	m_transform_scale = glm::scale(m_transform_scale, glm::vec3(a_size));
}

void Model::set_size(glm::vec3 a_size) {
	m_size = a_size;
	m_transform_scale = glm::mat4(1.0f);
	m_transform_scale = glm::scale(m_transform_scale, a_size);
}

void Model::move(glm::vec3 a_vec) {
	m_pos += a_vec;
	m_transform_pos = glm::translate(m_transform_pos, a_vec);
}

void Model::rotate(float a_angle, Axis a_axis) {
	switch (a_axis) {
		case Axis::X:
			m_transform_rotation = glm::rotate(m_transform_rotation, glm::radians(a_angle), glm::vec3(1.0, 0.0, 0.0));
			break;
		case Axis::Y:
			m_transform_rotation = glm::rotate(m_transform_rotation, glm::radians(a_angle), glm::vec3(0.0, 1.0, 0.0));
			break;
		case Axis::Z:
			m_transform_rotation = glm::rotate(m_transform_rotation, glm::radians(a_angle), glm::vec3(0.0, 0.0, 1.0));
			break;
	}
}

void Model::draw(ShaderProgram &a_shader, std::string a_material_map_uniform_name) {
	for (auto& mesh : m_meshes) {
		if (a_material_map_uniform_name != "")
			a_shader.set_uniform(a_material_map_uniform_name, mesh.get_material_map());
		mesh.draw(a_shader);
	}
}

glm::vec3 Model::get_pos() const {
	return m_pos;
}

glm::vec3 Model::get_size() const {
	return m_size;
}

std::string Model::get_dir() const {
	return m_dir.c_str();
}

std::string Model::get_name() const {
	return m_name.c_str();
}

glm::mat4 Model::get_model_mat() const {
	return m_transform_pos * m_transform_rotation * m_transform_scale;
}

glm::mat3 Model::get_normal_mat() const {
	// Get normal matix from M matrix
	return glm::transpose(glm::inverse(glm::mat3(get_model_mat())));
}

void Model::load_model() {
	Assimp::Importer importer;
	fs::path p = get_proj_path() / m_dir / m_name;
	const aiScene *scene = importer.ReadFile(p.c_str(), aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		ERROR(std::format("ASSIMP::{}", importer.GetErrorString()).c_str());
		return;
    }

    process_node(scene->mRootNode, scene);
}

void Model::process_node(aiNode *a_node, const aiScene *a_scene) {
	for (size_t i = 0; i < a_node->mNumMeshes; i++) {
		aiMesh *mesh = a_scene->mMeshes[a_node->mMeshes[i]];
		m_meshes.push_back(process_mesh(mesh, a_scene));
	}

	for (size_t i = 0; i < a_node->mNumChildren; i++) {
		process_node(a_node->mChildren[i], a_scene);
	}
}

void Model::process_texture(std::vector<std::shared_ptr<Texture>>& a_textures, aiMaterial* a_mat, aiTextureType a_ai_tex_type, int a_unit_id) {
	aiString texture_name;
	TextureType tex_type{ TextureType::NONE };
	switch (a_ai_tex_type) {
		case aiTextureType::aiTextureType_DIFFUSE:
			tex_type = TextureType::DIFFUSE;
			break;
		case aiTextureType::aiTextureType_SPECULAR:
			tex_type = TextureType::SPECULAR;
			break;
		case aiTextureType::aiTextureType_EMISSIVE:
			tex_type = TextureType::EMISSION;
			break;
	}
	auto lamb_texture_name_compare = [&texture_name, this](const std::shared_ptr<Texture>& texture){
		return texture->get_dir() == (this->m_dir / texture_name.C_Str()).c_str(); 
	};

	for (size_t i = 0; i < a_mat->GetTextureCount(a_ai_tex_type); i++) {
		a_mat->GetTexture(a_ai_tex_type, i, &texture_name);
		auto it = std::find_if(m_textures_loaded.begin(), m_textures_loaded.end(), lamb_texture_name_compare);
		if (it != m_textures_loaded.end()) {
			a_textures.push_back(*it);
		} else {
			// std::string texture_dir = (this->m_dir / formatted_texture_name).c_str();
			std::string texture_dir = (this->m_dir / texture_name.C_Str()).c_str();
			std::shared_ptr<Texture> texture_ptr = std::make_shared<Texture>(texture_dir, tex_type, a_unit_id++);
			a_textures.push_back(texture_ptr);
			m_textures_loaded.push_back(texture_ptr);
		}
	}
}

Mesh Model::process_mesh(aiMesh *a_mesh, const aiScene *a_scene) {
	BufferData data;
	std::vector<std::shared_ptr<Texture>> textures;
	MaterialMap mat;

	for (size_t i = 0; i < a_mesh->mNumVertices; i++) {
		data.positions.push_back(glm::vec3(a_mesh->mVertices[i].x, a_mesh->mVertices[i].y, a_mesh->mVertices[i].z));
		data.normals.push_back(glm::vec3(a_mesh->mNormals[i].x, a_mesh->mNormals[i].y, a_mesh->mNormals[i].z));
		if (a_mesh->mTextureCoords[0]) 
			data.texture_coords.push_back(glm::vec2(a_mesh->mTextureCoords[0][i].x, a_mesh->mTextureCoords[0][i].y));
		else 
			data.texture_coords.push_back(glm::vec2(0));
	}
	data.vert_sum = data.positions.size();

	for (size_t i = 0; i < a_mesh->mNumFaces; i++)
		for (size_t j = 0; j < a_mesh->mFaces[i].mNumIndices; j++)
			data.indicies.push_back(a_mesh->mFaces[i].mIndices[j]);
	data.indicies_sum = data.indicies.size();

	data.buffer_type = (data.indicies_sum) ? BufferData::Type::ELEMENT : BufferData::Type::VERTEX;
	
	if(a_mesh->mMaterialIndex >= 0) {
		aiMaterial *material = a_scene->mMaterials[a_mesh->mMaterialIndex];

		process_texture(textures, material, aiTextureType_DIFFUSE, 0);
		process_texture(textures, material, aiTextureType_SPECULAR, (int)material->GetTextureCount(aiTextureType_DIFFUSE));
	}
	mat.set_textures(textures);

	return Mesh(data, mat);
}
