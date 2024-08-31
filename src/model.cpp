#include <assimp/material.h>
#include <assimp/postprocess.h>

#include <format>
#include <memory>
#include <algorithm>

#include "chill_engine/model.hpp"
#include "chill_engine/meshes.hpp"
#include "chill_engine/file_manager.hpp"
#include "chill_engine/application.hpp"

namespace chill_engine {
namespace fs = std::filesystem;

extern fs::path guess_path(std::wstring a_path);

Model::Model(std::wstring a_path, bool a_flip_UVs) :m_flipped_UVs{ a_flip_UVs } {
	load_model(a_path, a_flip_UVs);
}

void Model::load_model(std::wstring& a_path, bool a_flip_UVs) {
	clear();

	fs::path p = guess_path(a_path);
	if (p == fs::path())
		ERROR(std::format("[MODEL::MODEL] Bad model path: {}", wstos(a_path)), Error_action::throwing);

	m_flipped_UVs = a_flip_UVs;
	m_path = fs::canonical(p).wstring();
	m_dir = fs::canonical(p.parent_path()).wstring();
	m_filename = p.filename().wstring();

	Assimp::Importer importer;
	int flags = aiProcess_Triangulate | aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_GenSmoothNormals;
	if (a_flip_UVs) {
		flags |= aiProcess_FlipUVs;
	}
	const aiScene* scene = importer.ReadFile(wstos(m_path), flags);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		ERROR(std::format("[MODEL::LOAD_MODEL] ASSIMP::{}", importer.GetErrorString()), Error_action::logging);
		return;
	}

	// Recursive method
	// process_node(scene->mRootNode, scene);

	// Iterative method
	for (unsigned int meshIndex = 0; meshIndex < scene->mNumMeshes; meshIndex++) {
		aiMesh* currentMesh = scene->mMeshes[meshIndex];
		m_meshes.push_back(process_mesh(currentMesh, scene));
	}
}

Model::Model(std::vector<Mesh> a_meshes) {
	set_meshes(a_meshes);
}

void Model::clear() {
	m_pos = glm::vec3(0.0f);
	m_size = glm::vec3(1.0f);
	m_transform_scale = 1.0f;
	m_transform_rotation = 1.0f;
	m_transform_pos = 1.0f;
	m_meshes.clear();
	m_path = L"";
	m_dir = L"";
	m_filename = L"";
	m_flipped_UVs = false;
}

// Nodes are laid out in tree like fashion. Each aiNode::mMeshes is an array of indicies into
// aiScene::mMeshes which contains Meshes we process.
void Model::process_node(aiNode* a_node, const aiScene* a_scene) {
	for (int i = 0; i < a_node->mNumMeshes; i++) {
		aiMesh* mesh = a_scene->mMeshes[a_node->mMeshes[i]];
		m_meshes.push_back(process_mesh(mesh, a_scene));
	}

	for (int i = 0; i < a_node->mNumChildren; i++) {
		process_node(a_node->mChildren[i], a_scene);
	}
}

Mesh Model::process_mesh(aiMesh* a_mesh, const aiScene* a_scene) {
	BufferData data;
	MaterialMap mat;
	std::vector<Texture> textures;

	// Load VBO specific data
	if (a_mesh->HasPositions()) {
		auto has_UV_channels = a_mesh->GetNumUVChannels();
		auto has_normals = a_mesh->HasNormals();
		for (int i = 0; i < a_mesh->mNumVertices; i++) {
			// Load vertex positions.
			data.positions.push_back(glm::vec3(a_mesh->mVertices[i].x, a_mesh->mVertices[i].y, a_mesh->mVertices[i].z));

			// Load normals.
			if (has_normals) {
				data.normals.push_back(glm::vec3(a_mesh->mNormals[i].x, a_mesh->mNormals[i].y, a_mesh->mNormals[i].z));
			}
			else {
				data.normals.push_back(glm::vec3(0));
			}

			// Load UVs from first UV channel.
			// TODO: Manage situation when a_mesh->mNumUVComponents[n] is different than 2.
			if (has_UV_channels && a_mesh->mNumUVComponents[0] == 2) {
				data.UVs.push_back(glm::vec2(a_mesh->mTextureCoords[0][i].x, a_mesh->mTextureCoords[0][i].y));
			}
			else {
				data.UVs.push_back(glm::vec2(0));
			}
		}
	}

	if (a_mesh->HasFaces()) {
		// Iterate over mesh faces and save indicies if any.
		for (int i = 0; i < a_mesh->mNumFaces; i++) {
			aiFace face = a_mesh->mFaces[i];
			// If mNumIndices is not 3 then face is not a triangle, which shouldn't happend in this importer.
			assert(face.mNumIndices == 3);
			for (int j = 0; j < 3; j++) {
				data.indicies.push_back(a_mesh->mFaces[i].mIndices[j]);
			}
		}
	}

	// Load mesh textures. I'm assuming that result of ambient calculations is the same as for diffuse (usual behaviour),
	// and so I don't process aiTextureType_AMBIENT. Might add support for more aiTextureTypes in the future.
	if (a_scene->HasMaterials()) {
		aiMaterial* material = a_scene->mMaterials[a_mesh->mMaterialIndex];

		process_texture(textures, material, aiTextureType_DIFFUSE);
		process_texture(textures, material, aiTextureType_SPECULAR);
		process_texture(textures, material, aiTextureType_EMISSIVE);
	}
	mat.set_textures(textures);

	return Mesh(data, mat);
}

void Model::process_texture(std::vector<Texture>& a_textures, aiMaterial* a_mat, aiTextureType a_ai_texture_type) {
	aiString texture_name;
	TextureType texture_type{ TextureType::NONE };
	int unit_id{ 0 };

	switch (a_ai_texture_type) {
	case aiTextureType::aiTextureType_DIFFUSE:
		texture_type = TextureType::DIFFUSE;
		unit_id = g_diffuse_unit_id;
		break;
	case aiTextureType::aiTextureType_SPECULAR:
		texture_type = TextureType::SPECULAR;
		unit_id = g_specular_unit_id;
		break;
	case aiTextureType::aiTextureType_EMISSIVE:
		texture_type = TextureType::EMISSION;
		unit_id = g_emission_unit_id;
		break;
	}

	for (int i = 0; i < a_mat->GetTextureCount(a_ai_texture_type); i++) {
		// Get texture path
		a_mat->GetTexture(a_ai_texture_type, i, &texture_name);

		// Use rmanager to load texture (see Application class).
		ResourceManager& rman = Application::get_instance().get_rmanager();
		std::wstring texture_path = (fs::path(m_dir) / fs::path(texture_name.C_Str())).wstring();
		Texture texture_ptr = rman.load_texture(texture_path, texture_type, false, unit_id + i);
		a_textures.push_back(texture_ptr);
	}
}

void Model::set_pos(glm::vec3 a_pos) {
	m_pos = a_pos;
	m_transform_pos = glm::mat4(1.0f);
	m_transform_pos = glm::translate(m_transform_pos, a_pos);
}

void Model::set_rotation(glm::vec3 a_rotation) {
	m_rotation = a_rotation;
	m_transform_rotation = glm::mat4(1.0f);
	m_transform_rotation = glm::rotate(m_transform_rotation, glm::radians(a_rotation[0]), glm::vec3(1.0, 0.0, 0.0));
	m_transform_rotation = glm::rotate(m_transform_rotation, glm::radians(a_rotation[1]), glm::vec3(0.0, 1.0, 0.0));
	m_transform_rotation = glm::rotate(m_transform_rotation, glm::radians(a_rotation[2]), glm::vec3(0.0, 0.0, 1.0));
}

void Model::set_meshes(std::vector<Mesh>& a_meshes) {
	m_meshes = a_meshes;
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
		m_rotation[0] += a_angle;
		m_transform_rotation = glm::rotate(m_transform_rotation, glm::radians(a_angle), glm::vec3(1.0, 0.0, 0.0));
		break;
	case Axis::Y:
		m_rotation[1] += a_angle;
		m_transform_rotation = glm::rotate(m_transform_rotation, glm::radians(a_angle), glm::vec3(0.0, 1.0, 0.0));
		break;
	case Axis::Z:
		m_rotation[2] += a_angle;
		m_transform_rotation = glm::rotate(m_transform_rotation, glm::radians(a_angle), glm::vec3(0.0, 0.0, 1.0));
		break;
	}
}

void Model::draw_outlined(float a_thickness, ShaderProgram& a_object_shader, ShaderProgram& a_outline_shader, std::string a_model_uniform_name, std::string a_material_map_uniform_name) {
	// Save shader options
	bool obj_depth = a_object_shader.get_state("DEPTH_TEST");
	bool obj_stencil = a_object_shader.get_state("STENCIL_TEST");
	bool out_depth = a_outline_shader.get_state("DEPTH_TEST");
	bool out_stencil = a_outline_shader.get_state("STENCIL_TEST");

	// Render object and write 1's to stencil buffer.
	a_object_shader.set_stencil_testing(true);
	glStencilMask(0xFF);
	glStencilFunc(GL_ALWAYS, 1, 0xFF); // Stencil test always passes
	glStencilOp(GL_KEEP, GL_REPLACE, GL_REPLACE);

	a_object_shader.use();
	draw(a_object_shader, a_material_map_uniform_name);

	// Render object's outline by rendering bigger object and checking wheter
	// it's fragments stencil values are equal to 0, if yes then they are part
	// of outline.

	a_outline_shader.set_stencil_testing(true);
	a_outline_shader.set_depth_testing(false);
	glStencilMask(0x00);
	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);

	auto tmp_scale_mat = m_transform_scale;
	auto tmp_size = m_size;
	set_size(get_size() * a_thickness);
	a_outline_shader[a_model_uniform_name] = get_model_mat();
	a_outline_shader.use();
	draw();

	// Restore
	m_transform_scale = tmp_scale_mat;
	m_size = tmp_size;
	glStencilFunc(GL_ALWAYS, 0, 0xFF); // Stencil test always passes
	glStencilMask(0xFF);
	glClear(GL_STENCIL_BUFFER_BIT);

	a_object_shader.set_depth_testing(obj_depth);
	a_object_shader.set_stencil_testing(obj_stencil);
	a_outline_shader.set_depth_testing(out_depth);
	a_outline_shader.set_stencil_testing(out_stencil);
}

// Draw with material maps.
void Model::draw(ShaderProgram& a_shader, std::string a_material_map_uniform_name) {
	for (auto& mesh : m_meshes) {
		if (a_material_map_uniform_name != "")
			a_shader.set_uniform(a_material_map_uniform_name, mesh.get_material_map());
		mesh.draw();
	}
}

// Draw without material maps.
void Model::draw() {
	for (auto& mesh : m_meshes) {
		mesh.draw();
	}
}

glm::vec3 Model::get_pos() const {
	return m_pos;
}

glm::vec3 Model::get_size() const {
	return m_size;
}

glm::vec3 Model::get_rotation() const {
	return m_rotation;
}

std::wstring Model::get_path() const {
	return m_path;
}

std::wstring Model::get_dir() const {
	return m_dir;
}

std::wstring Model::get_filename() const {
	return m_filename;
}

std::vector<Mesh>& Model::get_meshes() {
	return m_meshes;
}

glm::mat4 Model::get_model_mat() const {
	return m_transform_pos * m_transform_rotation * m_transform_scale;
}

glm::mat3 Model::get_normal_mat() const {
	// Get normal matix from M matrix
	return glm::transpose(glm::inverse(glm::mat3(get_model_mat())));
}

bool Model::is_flipped() const {
	return m_flipped_UVs;
} 
}