#include <assimp/Importer.hpp>
#include <assimp/material.h>
#include <assimp/postprocess.h>

#include <format>

#include "chill_renderer/model.hpp"
#include "chill_renderer/file_manager.hpp"
#include "chill_renderer/application.hpp"

namespace chill_renderer {
namespace fs = std::filesystem;

extern fs::path guess_path(const std::wstring& a_path);

Model::Model(const std::wstring& a_path, bool a_flip_UVs, bool a_gamma_corr) {
	load_model(a_path, a_flip_UVs, a_gamma_corr);
}

void Model::load_model(const std::wstring & a_path, bool a_flip_UVs, bool a_gamma_corr) {
	clear();

	fs::path p = guess_path(a_path);
	if (p == fs::path())
		ERROR(std::format("[MODEL::MODEL] Bad model path: {}", wstos(a_path)), Error_action::throwing);

	m_flipped_UVs = a_flip_UVs;
	m_gamma_corr = a_gamma_corr;
	m_path = p.wstring();
	m_dir = p.parent_path().wstring();
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

Model::Model(const std::vector<Mesh>& a_meshes) {
	set_meshes(a_meshes);
}

void Model::clear() noexcept {
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
	m_gamma_corr = false;
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
	std::vector<Texture2D> textures;
	auto texture_siz = a_scene->mMaterials[a_mesh->mMaterialIndex]->GetTextureCount(aiTextureType::aiTextureType_DIFFUSE);
	texture_siz += a_scene->mMaterials[a_mesh->mMaterialIndex]->GetTextureCount(aiTextureType::aiTextureType_SPECULAR);
	texture_siz += a_scene->mMaterials[a_mesh->mMaterialIndex]->GetTextureCount(aiTextureType::aiTextureType_EMISSIVE);
	textures.reserve(texture_siz);

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

void Model::process_texture(std::vector<Texture2D>& a_textures, aiMaterial* a_mat, aiTextureType a_ai_texture_type) {
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
		Texture2D tex_obj = rman.load_texture(texture_type, texture_path, false, m_gamma_corr);
		tex_obj.set_unit_id(unit_id + i);
		a_textures.push_back(tex_obj);
	}
}

void Model::set_pos(const glm::vec3& a_pos) noexcept {
	m_pos = a_pos;
	m_transform_pos = glm::mat4(1.0f);
	m_transform_pos = glm::translate(m_transform_pos, a_pos);
}

void Model::set_rotation(const glm::vec3& a_rotation) noexcept {
	m_rotation = a_rotation;
	m_transform_rotation = glm::mat4(1.0f);
	m_transform_rotation = glm::rotate(m_transform_rotation, glm::radians(a_rotation[0]), glm::vec3(1.0, 0.0, 0.0));
	m_transform_rotation = glm::rotate(m_transform_rotation, glm::radians(a_rotation[1]), glm::vec3(0.0, 1.0, 0.0));
	m_transform_rotation = glm::rotate(m_transform_rotation, glm::radians(a_rotation[2]), glm::vec3(0.0, 0.0, 1.0));
}

void Model::set_meshes(const std::vector<Mesh>& a_meshes) noexcept {
	m_meshes = a_meshes;
}

void Model::set_outline(bool a_option) noexcept {
	m_outline.enabled = a_option;
}

void Model::set_outline_thickness(float a_thickness) noexcept {
	m_outline.thickness = a_thickness;
}
 
void Model::set_outline_color(glm::vec3 a_color) noexcept {
	m_outline.color = a_color;
}

void Model::set_size(float a_size) noexcept {
	m_size = glm::vec3(a_size);
	m_transform_scale = glm::mat4(1.0f);
	m_transform_scale = glm::scale(m_transform_scale, glm::vec3(a_size));
}

void Model::set_size(const glm::vec3& a_size) noexcept {
	m_size = a_size;
	m_transform_scale = glm::mat4(1.0f);
	m_transform_scale = glm::scale(m_transform_scale, a_size);
}

void Model::move(const glm::vec3& a_vec) noexcept {
	m_pos += a_vec;
	m_transform_pos = glm::translate(m_transform_pos, a_vec);
}

void Model::rotate(float a_angle, Axis a_axis) noexcept {
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

void Model::draw_outline(ShaderProgram& a_object_shader, ShaderProgram& a_outline_shader, const std::string& a_model_uniform_name, const std::string& a_material_map_uniform_name) {
	// Save shader options
	bool obj_depth = a_object_shader.is_state(ShaderState::DEPTH_TEST);
	bool obj_stencil = a_object_shader.is_state(ShaderState::STENCIL_TEST);
	bool out_depth = a_outline_shader.is_state(ShaderState::DEPTH_TEST);
	bool out_stencil = a_outline_shader.is_state(ShaderState::STENCIL_TEST);

	// Render object and write 1's to stencil buffer.
	a_object_shader.set_state(ShaderState::STENCIL_TEST, true);
	glStencilMask(0xFF);
	glStencilFunc(GL_ALWAYS, 1, 0xFF); // Stencil test always passes
	glStencilOp(GL_KEEP, GL_REPLACE, GL_REPLACE);

	a_object_shader.use();
	draw(a_object_shader, a_material_map_uniform_name);

	// Render object's outline by rendering bigger object and checking wheter
	// it's fragments stencil values are equal to 0, if yes then they are part
	// of outline.

	a_outline_shader.set_state(ShaderState::STENCIL_TEST, true);
	a_outline_shader.set_state(ShaderState::DEPTH_TEST, true);
	glStencilMask(0x00);
	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);

	auto tmp_scale_mat = m_transform_scale;
	auto tmp_size = m_size;
	set_size(get_size() * m_outline.thickness);
	a_outline_shader[a_model_uniform_name] = get_model_mat();
	a_outline_shader.use();
	draw();

	// Restore
	m_transform_scale = tmp_scale_mat;
	m_size = tmp_size;
	glStencilFunc(GL_ALWAYS, 0, 0xFF); // Stencil test always passes
	glStencilMask(0xFF);
	glClear(GL_STENCIL_BUFFER_BIT);

	a_object_shader.set_state(ShaderState::DEPTH_TEST, obj_depth);
	a_object_shader.set_state(ShaderState::STENCIL_TEST, obj_stencil);
	a_outline_shader.set_state(ShaderState::DEPTH_TEST, out_depth);
	a_outline_shader.set_state(ShaderState::STENCIL_TEST, out_stencil);
}

// Draw with material maps.
void Model::draw(ShaderProgram& a_shader, const std::string& a_material_map_uniform_name) {
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

glm::vec3 Model::get_pos() const noexcept {
	return m_pos;
}

glm::vec3 Model::get_size() const noexcept {
	return m_size;
}

glm::vec3 Model::get_rotation() const noexcept {
	return m_rotation;
}

std::wstring Model::get_path() const noexcept {
	return m_path;
}

std::wstring Model::get_dir() const noexcept {
	return m_dir;
}

std::wstring Model::get_filename() const noexcept {
	return m_filename;
}

std::vector<Mesh>& Model::get_meshes() noexcept {
	return m_meshes;
}

glm::mat4 Model::get_model_mat() const noexcept {
	return m_transform_pos * m_transform_rotation * m_transform_scale;
}

glm::mat3 Model::get_normal_mat() const noexcept {
	// Get normal matix from M matrix
	return glm::transpose(glm::inverse(glm::mat3(get_model_mat())));
}

glm::mat3 Model::get_normal_view_mat(const glm::mat4& a_view_mat) const noexcept {
	return glm::mat3(glm::transpose(glm::inverse(get_model_mat() * a_view_mat)));
}

float Model::get_outline_thickness() const noexcept {
	return m_outline.thickness;
}

glm::vec3 Model::get_outline_color() const noexcept {
	return m_outline.color;
}

bool Model::is_outlined() const noexcept {
	return m_outline.enabled;
}

bool Model::is_flipped() const noexcept {
	return m_flipped_UVs;
}

bool Model::is_gamma_corr() const noexcept {
	return m_gamma_corr;
}

ModelInstanced::ModelInstanced(const Model& a_model) 
	:m_model_base{ a_model } 
{ }

ModelInstanced::ModelInstanced(const ModelInstanced& a_obj) {
	Application::get_instance().get_rmanager().inc_ref_count(ResourceType::INSTANCED_ARRAYS, a_obj.m_model_mat_buf_id);
	Application::get_instance().get_rmanager().inc_ref_count(ResourceType::INSTANCED_ARRAYS, a_obj.m_normal_mat_buf_id);

	m_model_base = a_obj.m_model_base;
	m_instances_siz = a_obj.m_instances_siz;
	m_model_mat_buf_id = a_obj.m_model_mat_buf_id;
	m_normal_mat_buf_id = a_obj.m_normal_mat_buf_id;
	m_instanced_vecs = a_obj.m_instanced_vecs;
	m_model_mats = a_obj.m_model_mats;
	m_normal_mats = a_obj.m_normal_mats;
}

ModelInstanced::ModelInstanced(ModelInstanced&& a_obj) noexcept { 
	m_model_base = std::move(a_obj.m_model_base);
	m_instances_siz = a_obj.m_instances_siz;
	m_model_mat_buf_id = a_obj.m_model_mat_buf_id;
	m_normal_mat_buf_id = a_obj.m_normal_mat_buf_id;
	m_instanced_vecs = std::move(a_obj.m_instanced_vecs);
	m_model_mats = std::move(a_obj.m_model_mats);
	m_normal_mats = std::move(a_obj.m_normal_mats);

	a_obj.m_model_mat_buf_id = EMPTY_VBO;
	a_obj.m_normal_mat_buf_id = EMPTY_VBO;
}

ModelInstanced& ModelInstanced::operator=(const ModelInstanced& a_obj) {
	Application::get_instance().get_rmanager().inc_ref_count(ResourceType::INSTANCED_ARRAYS, a_obj.m_model_mat_buf_id);
	Application::get_instance().get_rmanager().inc_ref_count(ResourceType::INSTANCED_ARRAYS, a_obj.m_normal_mat_buf_id);

	m_model_base = a_obj.m_model_base;
	m_instances_siz = a_obj.m_instances_siz;
	m_model_mat_buf_id = a_obj.m_model_mat_buf_id;
	m_normal_mat_buf_id = a_obj.m_normal_mat_buf_id;
	m_instanced_vecs = a_obj.m_instanced_vecs;
	m_model_mats = a_obj.m_model_mats;
	m_normal_mats = a_obj.m_normal_mats;

	return *this;
}

ModelInstanced& ModelInstanced::operator=(ModelInstanced&& a_obj) noexcept {
	m_model_base = std::move(a_obj.m_model_base);
	m_instances_siz = a_obj.m_instances_siz;
	m_model_mat_buf_id = a_obj.m_model_mat_buf_id;
	m_normal_mat_buf_id = a_obj.m_normal_mat_buf_id;
	m_instanced_vecs = std::move(a_obj.m_instanced_vecs);
	m_model_mats = std::move(a_obj.m_model_mats);
	m_normal_mats = std::move(a_obj.m_normal_mats); 

	a_obj.m_model_mat_buf_id = EMPTY_VBO;
	a_obj.m_normal_mat_buf_id = EMPTY_VBO;
	return *this;
}

ModelInstanced::~ModelInstanced() {
	if (m_model_mat_buf_id != EMPTY_VBO) {
		Application::get_instance().get_rmanager().dec_ref_count(ResourceType::INSTANCED_ARRAYS, m_model_mat_buf_id);
		if (!Application::get_instance().get_rmanager().chk_ref_count(ResourceType::INSTANCED_ARRAYS, m_model_mat_buf_id)) {
			glDeleteTextures(1, &m_model_mat_buf_id);
		}
	}

	if (m_normal_mat_buf_id != EMPTY_VBO) {
		Application::get_instance().get_rmanager().dec_ref_count(ResourceType::INSTANCED_ARRAYS, m_normal_mat_buf_id);
		if (!Application::get_instance().get_rmanager().chk_ref_count(ResourceType::INSTANCED_ARRAYS, m_normal_mat_buf_id)) {
			glDeleteTextures(1, &m_normal_mat_buf_id);
		}
	}
}

void ModelInstanced::set_model(const Model& a_model) {
	m_model_base = a_model;
	create_model_instanced_arr(m_model_mats);
	create_normal_instanced_arr(m_normal_mats);
}

void ModelInstanced::push_position(const glm::vec3& a_position) noexcept {
	m_instanced_vecs[ModelInstancedVecs::POSITIONS].push_back(a_position);
}

void ModelInstanced::push_rotation(const glm::vec3& a_rotation) noexcept {
	m_instanced_vecs[ModelInstancedVecs::ROTATIONS].push_back(a_rotation);
} 

void ModelInstanced::push_size(const glm::vec3& a_size) noexcept {
	m_instanced_vecs[ModelInstancedVecs::SIZES].push_back(a_size);
}

void ModelInstanced::push_positions(const std::vector<glm::vec3>& a_positions) noexcept {
	m_instanced_vecs[ModelInstancedVecs::POSITIONS] = a_positions;
}

void ModelInstanced::push_rotations(const std::vector<glm::vec3>& a_rotations) noexcept {
	m_instanced_vecs[ModelInstancedVecs::ROTATIONS] = a_rotations;
}

void ModelInstanced::push_sizes(const std::vector<glm::vec3>& a_sizes) noexcept {
	m_instanced_vecs[ModelInstancedVecs::SIZES] = a_sizes;
}

bool ModelInstanced::insert_buffer(std::size_t idx) { 
	if (idx < m_instances_siz) {
		auto model_mat = calculate_model_mat(idx);
		auto normal_mat = calculate_normal_mat(idx);
		glBindBuffer(GL_ARRAY_BUFFER, m_model_mat_buf_id);
		glBufferSubData(GL_ARRAY_BUFFER, idx * sizeof(glm::mat4), sizeof(model_mat), glm::value_ptr(model_mat));

		glBindBuffer(GL_ARRAY_BUFFER, m_normal_mat_buf_id);
		glBufferSubData(GL_ARRAY_BUFFER, idx * sizeof(glm::mat4), sizeof(normal_mat), glm::value_ptr(normal_mat));
		return true;
	}
	return false;
}

bool ModelInstanced::insert_position(std::size_t idx, const glm::vec3& a_position) {
	auto& pos_vec = m_instanced_vecs[ModelInstancedVecs::POSITIONS]; 
	if (idx >= pos_vec.size())
		return false;
	pos_vec[idx] = a_position;
	return insert_buffer(idx);
}

bool ModelInstanced::insert_rotation(std::size_t idx, const glm::vec3& a_rotation) {
	auto& rot_vec = m_instanced_vecs[ModelInstancedVecs::ROTATIONS];
	if (idx >= rot_vec.size())
		return false;

	rot_vec[idx] = a_rotation; 
	return insert_buffer(idx);
}

bool ModelInstanced::insert_size(std::size_t idx, const glm::vec3& a_size) {
	auto& siz_vec = m_instanced_vecs[ModelInstancedVecs::SIZES];
	if (idx >= siz_vec.size())
		return false;

	siz_vec[idx] = a_size; 
	return insert_buffer(idx);
}

void ModelInstanced::populate_model_mat_buffer() {
	calculate_model_mats();
	create_model_instanced_arr(m_model_mats);
	m_instances_siz = m_model_mats.size();
}

void ModelInstanced::populate_normal_mat_buffer() {
	calculate_normal_mats();
	create_normal_instanced_arr(m_normal_mats);
}

void ModelInstanced::draw() {
	auto& meshes = m_model_base.get_meshes();
	for (auto& mesh : meshes) {
		mesh.draw_instances(m_instances_siz);
	}
}

void ModelInstanced::draw(ShaderProgram& a_shader, const std::string& a_material_map_uniform_name) {
	auto& meshes = m_model_base.get_meshes();
	for (auto& mesh : meshes) {
		if (a_material_map_uniform_name != "")
			a_shader.set_uniform(a_material_map_uniform_name, mesh.get_material_map());
		mesh.draw_instances(m_instances_siz);
	}
}

Model& ModelInstanced::get_model_base() noexcept {
	return m_model_base;
}

std::vector<glm::vec3>& ModelInstanced::get_positions() noexcept { 
	return m_instanced_vecs[ModelInstancedVecs::POSITIONS];
}

std::vector<glm::vec3>& ModelInstanced::get_rotations() noexcept { 
	return m_instanced_vecs[ModelInstancedVecs::ROTATIONS];
}

std::vector<glm::vec3>& ModelInstanced::get_size() noexcept {
	return m_instanced_vecs[ModelInstancedVecs::SIZES];
}

glm::mat4 ModelInstanced::calculate_model_mat(std::size_t idx) noexcept {
	auto& pos_vec = m_instanced_vecs[ModelInstancedVecs::POSITIONS];
	auto& rot_vec = m_instanced_vecs[ModelInstancedVecs::ROTATIONS];
	auto& siz_vec = m_instanced_vecs[ModelInstancedVecs::SIZES];

	glm::vec3 pos = (idx < pos_vec.size()) ? pos_vec[idx] : m_model_base.get_pos();
	glm::vec3 rot = (idx < rot_vec.size()) ? rot_vec[idx] : m_model_base.get_rotation();
	glm::vec3 siz = (idx < siz_vec.size()) ? siz_vec[idx] : m_model_base.get_size();

	glm::mat4 transform_pos = glm::translate(glm::mat4(1.0f), pos); 
	glm::mat4 transform_scale = glm::scale(glm::mat4(1.0f), siz); 
	glm::mat4 transform_rotation(1.0f);
	transform_rotation = glm::rotate(transform_rotation, glm::radians(rot[0]), glm::vec3(1.0, 0.0, 0.0));
	transform_rotation = glm::rotate(transform_rotation, glm::radians(rot[1]), glm::vec3(0.0, 1.0, 0.0));
	transform_rotation = glm::rotate(transform_rotation, glm::radians(rot[2]), glm::vec3(0.0, 0.0, 1.0));

	return transform_pos * transform_rotation * transform_scale;
}

glm::mat3 ModelInstanced::calculate_normal_mat(std::size_t idx) noexcept {
	return glm::transpose(glm::inverse(glm::mat3(m_model_mats[idx])));
}

void ModelInstanced::calculate_model_mats() noexcept {
	auto& pos_vec = m_instanced_vecs[ModelInstancedVecs::POSITIONS];
	auto& rot_vec = m_instanced_vecs[ModelInstancedVecs::ROTATIONS];
	auto& siz_vec = m_instanced_vecs[ModelInstancedVecs::SIZES];

	std::size_t max_siz = 0;
	for (const auto& size : { pos_vec.size(), rot_vec.size(), siz_vec.size()}) {
		max_siz = max_siz < size ? size : max_siz;
	}

	m_model_mats.clear();
	m_model_mats.reserve(max_siz); 
	for (std::size_t i = 0; i < max_siz; ++i) {
		m_model_mats.push_back(calculate_model_mat(i));
	}
	m_instances_siz = m_model_mats.size();
}

void ModelInstanced::calculate_normal_mats() noexcept {
	m_normal_mats.clear();
	m_normal_mats.reserve(m_model_mats.size());

	for (std::size_t i = 0; i < m_model_mats.size(); ++i) {
		m_normal_mats.push_back(calculate_normal_mat(i));
	}
}

// Modifies attribute pointers of original mesh.
auto ModelInstanced::create_model_instanced_arr(const std::vector<glm::mat4>& a_model_mats) -> void {
	m_instances_siz = a_model_mats.size();
	if (m_model_mat_buf_id != EMPTY_VBO)
		glDeleteBuffers(1, &m_model_mat_buf_id);

	glGenBuffers(1, &m_model_mat_buf_id);
	glBindBuffer(GL_ARRAY_BUFFER, m_model_mat_buf_id);
	glBufferData(GL_ARRAY_BUFFER, m_instances_siz * sizeof(glm::mat4), a_model_mats.data(), GL_STATIC_DRAW);

	auto& meshes = m_model_base.get_meshes();
	for (auto& mesh : meshes) {
		GLuint VAO = mesh.get_VAO(); 
		glBindVertexArray(VAO); 
		for (int i = 0; i < 4; ++i) {
			glVertexAttribPointer(g_attrib_model_mat_arr_location + i, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (void*)(i * sizeof(glm::vec4)));
			glEnableVertexAttribArray(g_attrib_model_mat_arr_location + i); 
			glVertexAttribDivisor(g_attrib_model_mat_arr_location + i, 1);
		}
	}
	glBindVertexArray(0);
}

auto ModelInstanced::create_normal_instanced_arr(const std::vector<glm::mat3>& a_normal_mats) -> void {
	if (m_normal_mat_buf_id != EMPTY_VBO)
		glDeleteBuffers(1, &m_normal_mat_buf_id);

	glGenBuffers(1, &m_normal_mat_buf_id);
	glBindBuffer(GL_ARRAY_BUFFER, m_normal_mat_buf_id);
	glBufferData(GL_ARRAY_BUFFER, a_normal_mats.size() * sizeof(glm::mat3), a_normal_mats.data(), GL_STATIC_DRAW);

	auto& meshes = m_model_base.get_meshes();
	for (auto& mesh : meshes) {
		GLuint VAO = mesh.get_VAO(); 
		glBindVertexArray(VAO); 
		for (int i = 0; i < 3; ++i) {
			glVertexAttribPointer(g_attrib_normal_mat_arr_location + i, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(glm::vec3), (void*)(i * sizeof(glm::vec3))); 
			glEnableVertexAttribArray(g_attrib_normal_mat_arr_location + i); 
			glVertexAttribDivisor(g_attrib_normal_mat_arr_location + i, 1);
		}
	}
	glBindVertexArray(0);
}
}