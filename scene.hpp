#pragma once

#include <chrono>
#include <random>

#include "chill_renderer/buffers.hpp"
#include "chill_renderer/model.hpp"
#include "chill_renderer/light.hpp"
#include "chill_renderer/window.hpp"
#include "chill_renderer/camera.hpp"
#include "chill_renderer/shaders.hpp"
#include "chill_renderer/presets.hpp"
#include "chill_renderer/application.hpp"

using namespace chill_renderer;

class Scene;
struct Skybox;
void imgui_cam(Camera& cam);
void imgui_dirlight(DirLight& dirlight_source);
void imgui_spotlight(SpotLight& spotlight_source);
void imgui_pointlight(PointLight& pointlight_source);
void imgui_model(Model& model); 
void process_input(Scene& a_scene);
void draw_gui(Scene& scene, Skybox& skybox1, Skybox& skybox2);
 
class Rand {
public: 
	using EngType = std::default_random_engine;
	using DistType = std::normal_distribution<>;
	using sys_clck = std::chrono::system_clock;

	Rand();
	auto roll_vec3(float min, float max) -> glm::vec3;
	auto roll_f(float min, float max) -> float;

	struct DistStruct {
		DistStruct(float a_min, float a_max);
		float min{};
		float max{};
		DistType distribution{};
	};

private: 
	DistType& find_dist(float min, float max);
	template<typename T>
	void seed(T&& s) { 
		m_engine.seed(s); 
	}

	EngType m_engine{};
	std::vector<DistStruct> m_distributions{};
};

template<typename T>
struct LitModel {
	T light;
	Model model;
};

struct Skybox {
	Texture cubemap{};
	Model cube{};
}; 

enum class CurShaderType { 
	POST_NONE, 
	MSAA_POST_NONE, 
	POST_KERNEL,  
	POST_INV, 
	POST_GRAY_AVG, 
	POST_GRAY_WGT, 
	POST_GAMMA,
	NORMAL_VIS,
};

struct CurShaderState {
	CurShaderState();
	glm::mat3 m_kernel{};
	glm::vec3 m_inv_color{ 1, 0.411, 0.705 };
	glm::vec3 m_normal_color{ 0, 1, 1 };
	glm::vec3 m_fog_color{ 1, 1, 1 };
	float m_fog_dens = 1.8f;
	float m_normal_mag = 0.2f;
	float m_kernel_offset = 1.f / 300.f;
	float m_gamma = 2.2f;
	int m_MSAA_samples = 4;
	bool m_blinn_phong = true;
};

class Scene {
public:
	Scene() = default;

	void set_window(Window* a_window);
	void set_camera(Camera* a_camera);
	void set_skybox(const Skybox& a_skybox);
	void set_cur_shader(CurShaderType a_type);
	void set_shaders(const std::map<std::string, ShaderProgram>& a_shaders);
	void set_pointlights(const std::vector<LitModel<PointLight>>& a_lights);
	void set_spotlights(const std::vector<LitModel<SpotLight>>& a_lights);
	void set_dirlights(const std::vector<LitModel<DirLight>>& a_lights);
	void set_generic_models(const std::vector<Model>& a_generic_models);
	void set_transparent_models(const std::vector<Model>& a_transparent_models);
	void set_reflective_models(const std::vector<Model>& a_reflective_models);
	void set_fb_reflection_cubemap(FrameBuffer&& a_fb_refl);
	void set_default_material(const std::wstring& a_diffuse_path);

	void push_shader(const std::string& a_name, const ShaderProgram& a_shader); 
	void push_dirlight(const LitModel<DirLight>& a_light);
	void push_pointlight(const LitModel<PointLight>& a_light);
	void push_spotlight(const LitModel<SpotLight>& a_light);
	void push_generic_model(const Model& a_model);
	void push_reflective_model(const Model& a_model);
	void push_transparent_model(const Model& a_model);
	void push_model_instanced(const ModelInstanced& a_mod_inst);
	void push_uniform_buffer(const UniformBuffer& a_ubo);
	void push_frame_buffer_post(FrameBuffer&& a_fb);

	void set_uniforms();

	void draw();
	void draw_lights();
	void draw_skybox();
	void draw_generic_models();
	void draw_instanced_models();
	void draw_reflective_models(const FrameBuffer& a_fb_last);
	void draw_transparent_models();
	void transform_models();
	void post_process();

	Window* get_window();
	Camera* get_camera();
	CurShaderType get_cur_shader() const;
	MaterialMap& get_default_material();

	FrameBuffer& get_post_fb();
	CurShaderState& get_shader_state();
	std::vector<Model>& get_generic_models();
	std::vector<Model>& get_reflective_models();
	std::vector<Model>& get_transparent_models();
	std::vector<LitModel<PointLight>>& get_pointlight_sources();
	std::vector<LitModel<SpotLight>>& get_spotlight_sources();
	std::vector<LitModel<DirLight>>& get_dirlight_sources();
	std::map<std::string, ShaderProgram>& get_shaders();

private:
	void sort_transparent_models();
	void set_reflective_cubemap(Model& a_refl_obj, FrameBuffer& a_fb_refl_cubemap);

	Window* m_window = nullptr;
	Camera* m_camera = nullptr;
	Model m_basic_plane = Application::get_instance().get_rmanager().create_model({ Mesh(presets::g_plane_data, MaterialMap()) });
	Skybox m_skybox{};
	CurShaderType m_cur_shader{ CurShaderType::MSAA_POST_NONE };
	CurShaderState m_shader_state{};
	MaterialMap m_default_material{};
	UniformBuffer m_ubo{};
	FrameBuffer m_fb_refl_cubemap{};
	FrameBuffer m_fb_post_process{};
	std::map<std::string, ShaderProgram> m_shaders{};
	std::vector<LitModel<PointLight>> m_pointlight_sources{};
	std::vector<LitModel<SpotLight>> m_spotlight_sources{};
	std::vector<LitModel<DirLight>> m_dirlight_sources{};
	std::vector<Model> m_generic_models{};
	std::vector<Model> m_transparent_models{};
	std::vector<Model> m_reflective_models{};
	std::vector<ModelInstanced> m_instanced_models{};
};
