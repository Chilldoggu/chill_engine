#pragma once

#include <assimp/scene.h>

#include <vector>

#include "chill_engine/meshes.hpp"
#include "chill_engine/shaders.hpp"

namespace chill_engine { 
inline constexpr int g_attrib_model_mat_arr_location = 4;
inline constexpr int g_attrib_normal_mat_arr_location = 8;

enum class Axis {
	X, Y, Z
};

class Model {
public:
	Model() = default;
	Model(const std::wstring& a_path, bool a_flip_UVs = false, bool a_gamma_corr = false);
	Model(const std::vector<Mesh>& a_meshes);

	auto load_model(const std::wstring& a_path, bool a_flip_UVs, bool a_gamma_corr) -> void;
	auto set_pos(const glm::vec3& a_pos) noexcept -> void;
	auto set_size(float a_size) noexcept -> void;
	auto set_size(const glm::vec3& a_size) noexcept -> void;
	auto set_rotation(const glm::vec3& a_rotation) noexcept -> void;
	auto set_meshes(const std::vector<Mesh>& a_meshes) noexcept -> void;
	auto move(const glm::vec3& a_vec) noexcept -> void;
	auto rotate(float a_angle, Axis a_axis = Axis::X) noexcept -> void;

	auto draw() -> void;
	auto draw(ShaderProgram& a_shader, const std::string& a_material_map_uniform_name) -> void;
	auto draw_outlined(float a_thickness, ShaderProgram& a_object_shader, ShaderProgram& a_outline_shader, const std::string& a_model_uniform_name, const std::string& a_material_map_uniform_name) -> void;
	auto clear() noexcept -> void;

	auto get_pos() const noexcept -> glm::vec3;
	auto get_size() const noexcept -> glm::vec3;
	auto get_rotation() const noexcept -> glm::vec3;
	auto get_dir() const noexcept -> std::wstring;
	auto get_path() const noexcept -> std::wstring;
	auto get_filename() const noexcept -> std::wstring;
	auto get_meshes() noexcept -> std::vector<Mesh>&;
	auto get_model_mat() const noexcept -> glm::mat4;
	auto get_normal_mat() const noexcept -> glm::mat3;
	auto get_normal_view_mat(const glm::mat4& a_view_mat) const noexcept -> glm::mat3;
	auto is_flipped() const noexcept -> bool;
	auto is_gamma_corr() const noexcept -> bool;

private:
	auto process_node(aiNode* a_node, const aiScene* a_scene) -> void;
	auto process_mesh(aiMesh* a_mesh, const aiScene* a_scene) -> Mesh;
	auto process_texture(std::vector<Texture>& a_textures, aiMaterial* a_mat, aiTextureType a_ai_texture_type) -> void;

	bool m_flipped_UVs = false;
	bool m_gamma_corr = false;
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

enum class ModelInstancedVecs {
	POSITIONS,
	ROTATIONS,
	SIZES,
};

class ModelInstanced {
public:
	using InstVecsMap = std::map<ModelInstancedVecs, std::vector<glm::vec3>>;

	ModelInstanced() = default;
	ModelInstanced(const Model& a_model);
	ModelInstanced(const ModelInstanced& a_obj);
	ModelInstanced(ModelInstanced&& a_obj) noexcept;
	~ModelInstanced();
	
	ModelInstanced& operator=(const ModelInstanced& a_obj);
	ModelInstanced& operator=(ModelInstanced&& a_obj) noexcept;

	auto set_model(const Model& a_model) -> void; 
	auto push_position(const glm::vec3& a_position) noexcept -> void;
	auto push_rotation(const glm::vec3& a_rotation) noexcept -> void;
	auto push_size(const glm::vec3& a_size) noexcept -> void; 
	auto push_positions(const std::vector<glm::vec3>& a_positions) noexcept -> void;
	auto push_rotations(const std::vector<glm::vec3>& a_rotations) noexcept -> void;
	auto push_sizes(const std::vector<glm::vec3>& a_sizes) noexcept -> void; 
	auto insert_buffer(std::size_t idx) -> bool;
	auto insert_position(std::size_t idx, const glm::vec3& a_position) -> bool;
	auto insert_rotation(std::size_t idx, const glm::vec3& a_rotation) -> bool;
	auto insert_size(std::size_t idx, const glm::vec3& a_size) -> bool; 

	auto populate_model_mat_buffer() -> void;
	auto populate_normal_mat_buffer() -> void;

	auto draw() -> void;
	auto draw(ShaderProgram& a_shader, const std::string& a_material_map_uniform_name) -> void;

	auto get_model_base() noexcept -> Model&;
	auto get_positions() noexcept -> std::vector<glm::vec3>&;
	auto get_rotations() noexcept -> std::vector<glm::vec3>&;
	auto get_size() noexcept -> std::vector<glm::vec3>&;

private:
	auto calculate_model_mat(std::size_t idx) noexcept -> glm::mat4;
	auto calculate_normal_mat(std::size_t idx) noexcept -> glm::mat3;
	auto calculate_model_mats() noexcept -> void;
	auto calculate_normal_mats() noexcept -> void;
	auto create_model_instanced_arr(const std::vector<glm::mat4>& a_model_mats) -> void;
	auto create_normal_instanced_arr(const std::vector<glm::mat3>& a_normal_mats) -> void;

	Model m_model_base{};
	int m_instances_siz{};
	GLuint m_model_mat_buf_id = EMPTY_VBO;
	GLuint m_normal_mat_buf_id = EMPTY_VBO;
	InstVecsMap m_instanced_vecs{
		{ ModelInstancedVecs::POSITIONS, std::vector<glm::vec3>{} },
		{ ModelInstancedVecs::ROTATIONS, std::vector<glm::vec3>{} },
		{ ModelInstancedVecs::SIZES, std::vector<glm::vec3>{} },
	}; 
	std::vector<glm::mat4> m_model_mats{};
	std::vector<glm::mat3> m_normal_mats{};
};
}