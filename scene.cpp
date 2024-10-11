#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp> // glm::length2
#include <imgui/imgui.h>
#include <imgui/backend/imgui_impl_glfw.h>
#include <imgui/backend/imgui_impl_opengl3.h>

#include <format>
#include <random>

#include "scene.hpp"
#include "chill_renderer/resource_manager.hpp"
#include "chill_renderer/file_manager.hpp"
#include "chill_renderer/application.hpp"

Rand::DistStruct::DistStruct(float a_min, float a_max) 
	:min{ a_min }, max{ a_max }
{
	float mean = (a_min + a_max) / 2;
	float sigma = (a_max - mean) / 3;
	distribution = DistType(mean, sigma);
}

Rand::Rand() {
	seed(sys_clck::now().time_since_epoch().count());
}

glm::vec3 Rand::roll_vec3(float min, float max) { 
	auto& dist = find_dist(min, max);
	glm::vec3 ret{};
	for (int i = 0; i < 3; ++i) {
		ret[i] = dist(m_engine);
	}
	return ret;
}

float Rand::roll_f(float min, float max) {
	auto& dist = find_dist(min, max);
	return dist(m_engine);
} 

Rand::DistType& Rand::find_dist(float min, float max) {
	auto it = std::find_if(m_distributions.begin(), m_distributions.end(), [&min, &max](const DistStruct& elem) {
		return (elem.min == min && elem.max == max);
		});
	if (it == m_distributions.end()) {
		m_distributions.emplace_back(min, max);
		return m_distributions.back().distribution;
	}
	return (*it).distribution;
}

CurShaderState::CurShaderState() {
	m_kernel[0][0] = 1.f; m_kernel[0][1] =  1.f; m_kernel[0][2] = 1.f;
	m_kernel[1][0] = 1.f; m_kernel[1][1] = -8.f; m_kernel[1][2] = 1.f;
	m_kernel[2][0] = 1.f; m_kernel[2][1] =  1.f; m_kernel[2][2] = 1.f;
}

void Scene::set_window(Window* a_window) {
	m_window = a_window;
}

void Scene::set_camera(Camera* a_camera) {
	m_camera = a_camera;
}

void Scene::set_skybox(const Skybox& a_skybox) {
	m_skybox = a_skybox;
}

void Scene::set_cur_shader(CurShaderType a_type) {
	m_cur_shader = a_type;
}

void Scene::set_shaders(const std::map<std::string, ShaderProgram>& a_shaders) {
	m_shaders = a_shaders;
}

void Scene::set_pointlights(const std::vector<LitModel<PointLight>>& a_lights) {
	m_pointlight_sources = a_lights;
}

void Scene::set_spotlights(const std::vector<LitModel<SpotLight>>& a_lights) {
	m_spotlight_sources = a_lights; 
}

void Scene::set_dirlights(const std::vector<LitModel<DirLight>>& a_lights) {
	m_dirlight_sources = a_lights; 
}

void Scene::set_generic_models(const std::vector<Model>& a_generic_models) {
	m_generic_models = a_generic_models;
}

void Scene::set_transparent_models(const std::vector<Model>& a_transparent_models) {
	m_transparent_models = a_transparent_models;
}

void Scene::set_reflective_models(const std::vector<Model>& a_reflective_models) {
	m_reflective_models = a_reflective_models;
}

void Scene::set_fb_reflection_cubemap(FrameBuffer&& a_fb_refl) {
	m_fb_refl_cubemap = std::move(a_fb_refl);
}

void Scene::set_default_material(const std::wstring& a_diffuse_path) {
	m_default_material.set_diffuse_maps({ {a_diffuse_path, false, false} });
}

void Scene::set_uniforms() { 
	glm::mat4 view_mat = m_camera->get_look_at();
	glm::mat4 projection_mat = m_camera->get_projection_matrix(m_window->get_width(), m_window->get_height());

	m_ubo["view"] = view_mat;
	m_ubo["projection"] = projection_mat;

	m_shaders["multi"]["view_pos"] = m_camera->get_position();
	m_shaders["multi"]["near_plane"] = m_camera->get_near_plane();
	m_shaders["multi"]["far_plane"] = m_camera->get_far_plane();
	m_shaders["multi"]["fog_dens"] = m_shader_state.m_fog_dens;
	m_shaders["multi"]["fog_color"] = m_shader_state.m_fog_color;
	m_shaders["multi"]["is_blinn_phong"] = m_shader_state.m_blinn_phong;
	m_shaders["refl"]["view_pos"] = m_camera->get_position();
	m_shaders["refr"]["view_pos"] = m_camera->get_position();
	m_shaders["dynamic_env"]["view_pos"] = m_camera->get_position();

	// Set light source uniforms 
	m_spotlight_sources[0].light.set_pos(m_camera->get_position());
	m_spotlight_sources[0].light.set_spot_dir(m_camera->get_target()); 
	m_shaders["multi"].set_uniform("dirlight_source", m_dirlight_sources[0].light);
	m_shaders["multi"].set_uniform("spotlight_source", m_spotlight_sources[0].light);
	for (size_t i = 0; i < m_pointlight_sources.size(); i++) {
		m_shaders["multi"].set_uniform(std::format("pointlight_sources[{}]", i), m_pointlight_sources[i].light);
	} 
}

void Scene::push_shader(const std::string& a_name, const ShaderProgram& a_shader) {
	m_shaders[a_name] = a_shader;
} 

void Scene::push_dirlight(const LitModel<DirLight>& a_light) {
	m_dirlight_sources.push_back(a_light);
}

void Scene::push_pointlight(const LitModel<PointLight>& a_light) {
	m_pointlight_sources.push_back(a_light);
}

void Scene::push_spotlight(const LitModel<SpotLight>& a_light) {
	m_spotlight_sources.push_back(a_light); 
}

void Scene::push_generic_model(const Model& a_model) {
	m_generic_models.push_back(a_model);
}

void Scene::push_reflective_model(const Model& a_model) {
	m_reflective_models.push_back(a_model);
}

void Scene::push_transparent_model(const Model& a_model) {
	m_transparent_models.push_back(a_model); 
}

void Scene::push_model_instanced(const ModelInstanced& a_mod_inst) {
	m_instanced_models.push_back(a_mod_inst);
}

void Scene::push_uniform_buffer(const UniformBuffer& a_ubo) {
	m_ubo = a_ubo;
}

void Scene::push_frame_buffer_post(FrameBuffer&& a_fb) {
	m_fb_post_process = std::move(a_fb);
}

void Scene::draw() {
	if (m_fb_post_process.get_id() != EMPTY_VBO)
		m_fb_post_process.bind();

	// Clear buffers
	glClearColor(0.05, 0.05, 0.05, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	set_uniforms(); 
	draw_lights();
	transform_models();
	draw_generic_models(); 
	draw_instanced_models();
	draw_reflective_models(m_fb_post_process);
	draw_skybox(); 
	draw_transparent_models();

	if (m_fb_post_process.get_id() != EMPTY_VBO)
		m_fb_post_process.unbind();
}

void Scene::draw_lights() {
	m_shaders["single"].use();

	auto cam_trg = m_camera->get_target();
	auto cam_pos = m_camera->get_position();

	m_spotlight_sources[0].model.set_pos(cam_pos);
	m_shaders["single"]["color"] = m_spotlight_sources[0].light.get_color();
	m_shaders["single"]["model"] = m_spotlight_sources[0].model.get_model_mat();
	m_spotlight_sources[0].model.draw();

	m_shaders["single"]["color"] = m_dirlight_sources[0].light.get_color();
	 m_shaders["single"]["model"] = m_dirlight_sources[0].model.get_model_mat();
	m_dirlight_sources[0].model.draw();

	m_pointlight_sources[0].model.set_pos(cam_pos);
	for (auto& lit_model : m_pointlight_sources) {
		m_shaders["single"]["color"] = lit_model.light.get_color();
		m_shaders["single"]["model"] = lit_model.model.get_model_mat();
		lit_model.model.draw(); 
	} 
} 

void Scene::draw_skybox() {
	glDepthFunc(GL_LEQUAL);
	m_shaders["skybox"].set_state(ShaderState::FACE_CULLING, false);
	m_shaders["skybox"].set_state(ShaderState::DEPTH_TEST, true);
	m_shaders["skybox"].use();
	m_skybox.cubemap.activate();
	m_skybox.cube.draw();
	glDepthFunc(GL_LESS); 
}

void Scene::draw_generic_models() {
	for (auto& gen_obj : m_generic_models) { 
		m_shaders["multi"].set_uniform("material", m_default_material);
		m_shaders["multi"]["model"] = gen_obj.get_model_mat();
		m_shaders["multi"]["normal_mat"] = gen_obj.get_normal_mat();

		if (gen_obj.is_outlined()) {
			m_shaders["single"]["color"] = gen_obj.get_outline_color();
			gen_obj.draw_outline(m_shaders["multi"], m_shaders["single"], "model", "material");
		}
		else {
			m_shaders["multi"].use();
			gen_obj.draw(m_shaders["multi"], "material"); 
		}
	} 

	if (m_cur_shader == CurShaderType::NORMAL_VIS) {
		m_shaders["normal_vis"].use();
		for (auto& gen_obj : m_generic_models) { 
			m_shaders["normal_vis"]["model"] = gen_obj.get_model_mat();
			m_shaders["normal_vis"]["normal_mat"] = gen_obj.get_normal_mat();
			m_shaders["normal_vis"]["normal_color"] = m_shader_state.m_normal_color;
			m_shaders["normal_vis"]["magnitude"] = m_shader_state.m_normal_mag;
			gen_obj.draw(); 
		} 
	}
}

void Scene::draw_instanced_models() {
	for (auto& inst_model : m_instanced_models) {
		m_shaders["multi_instanced"].use();
		m_shaders["multi_instanced"]["view_pos"] = m_camera->get_position();
		m_shaders["multi_instanced"].set_uniform("material", m_default_material);
		m_shaders["multi_instanced"].set_uniform("dirlight_source", m_dirlight_sources[0].light);
		//m_shaders["multi_instanced"]["time"] = glfwGetTime(); 
		inst_model.draw(m_shaders["multi_instanced"], "material"); 

	} 
}

void Scene::set_reflective_cubemap(Model& a_refl_obj, FrameBuffer& a_fb_refl_cubemap) { 
	// Save scene state
	Camera* m_camera_cp = m_camera;
	int window_width_cp = m_window->get_width();
	int window_height_cp = m_window->get_height();

	Camera refl_cam = *m_camera;
	refl_cam.set_position(a_refl_obj.get_pos());
	refl_cam.set_fov(90);
	m_window->set_width(a_fb_refl_cubemap.get_width());
	m_window->set_height(a_fb_refl_cubemap.get_height());
	m_camera = &refl_cam;

	glViewport(0, 0, a_fb_refl_cubemap.get_width(), a_fb_refl_cubemap.get_height());
	for (int i = 0; i < 6; ++i) {
		switch (GL_TEXTURE_CUBE_MAP_POSITIVE_X + i) {
		case GL_TEXTURE_CUBE_MAP_POSITIVE_X: 
			refl_cam.set_target(glm::vec3(1.0, 0.0, 0.0)); 
			refl_cam.set_up(glm::vec3(0, -1, 0));
			break;
		case GL_TEXTURE_CUBE_MAP_NEGATIVE_X: 
			refl_cam.set_target(glm::vec3(-1.0, 0.0, 0.0)); 
			refl_cam.set_up(glm::vec3(0, -1, 0));
			break;
		case GL_TEXTURE_CUBE_MAP_POSITIVE_Y: 
			refl_cam.set_target(glm::vec3(0.0, 1.0, 0.0)); 
			refl_cam.set_up(glm::vec3(0, 0, 1));
			break;
		case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y: 
			refl_cam.set_target(glm::vec3(0.0, -1.0, 0.0)); 
			refl_cam.set_up(glm::vec3(0, 0, -1));
			break;
		case GL_TEXTURE_CUBE_MAP_POSITIVE_Z: 
			refl_cam.set_target(glm::vec3(0.0, 0.0, 1.0)); 
			refl_cam.set_up(glm::vec3(0, -1, 0));
			break;
		case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z: 
			refl_cam.set_target(glm::vec3(0.0, 0.0, -1.0)); 
			refl_cam.set_up(glm::vec3(0, -1, 0));
			break;
		}

		a_fb_refl_cubemap.attach_cubemap_face(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i); 
		a_fb_refl_cubemap.bind(); 
		// Draw
		glClearColor(0.1, 0.1, 0.1, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
		set_uniforms(); 
		draw_lights();
		draw_generic_models(); 
		draw_instanced_models();
		draw_skybox(); 
		draw_transparent_models();
	}
	a_fb_refl_cubemap.unbind();

	// Restore scene state
	m_camera = m_camera_cp;
	m_window->set_width(window_width_cp);
	m_window->set_height(window_height_cp);
	glViewport(0, 0, window_width_cp, window_height_cp);
	set_uniforms(); 
}

void Scene::draw_reflective_models(const FrameBuffer& a_fb_last) {
	if (m_fb_refl_cubemap.get_id() == EMPTY_VBO) {
		FrameBuffer fb_refl_cubemap(2048, 2048);
		fb_refl_cubemap.attach(AttachmentType::COLOR_3D, AttachmentBufferType::TEXTURE);
		fb_refl_cubemap.attach(AttachmentType::DEPTH, AttachmentBufferType::RENDER_BUFFER);
		if (!fb_refl_cubemap.check_status()) {
			ERROR("[MAIN] FrameBuffer fb_dynamic_env is not complete!", Error_action::throwing); 
		}
		m_fb_refl_cubemap = std::move(fb_refl_cubemap);
	}

	m_shaders["dynamic_env"].use();
	for (auto& refl_obj : m_reflective_models) {
		if (glm::length(m_camera->get_position() - refl_obj.get_pos()) < 10) {
			set_reflective_cubemap(refl_obj, m_fb_refl_cubemap);
			m_fb_refl_cubemap.activate_color(); 
		}
		else {
			m_skybox.cubemap.activate(); 
		}

		m_shaders["dynamic_env"]["normal_mat"] = refl_obj.get_normal_mat(); 
		m_shaders["dynamic_env"]["model"] = refl_obj.get_model_mat(); 
		a_fb_last.bind();
		refl_obj.draw();
	}
	// Make sure last framebuffer is binded
	a_fb_last.bind();
}

// Sort from furthest to nearest only when camera moves
void Scene::sort_transparent_models() {
	static glm::vec3 prev_cam_pos(0, 0, 0);
	if (auto cam_pos = m_camera->get_position(); cam_pos != prev_cam_pos) {
		prev_cam_pos = cam_pos;
		m_pointlight_sources[0].light.set_pos(cam_pos);
		m_pointlight_sources[0].model.set_pos(cam_pos);
		std::sort(m_transparent_models.begin(), m_transparent_models.end(), [&cam_pos](const Model& trans_obj1, const Model& trans_obj2) {
			return glm::length2(trans_obj1.get_pos() - cam_pos) > glm::length2(trans_obj2.get_pos() - cam_pos);
			});
	} 
}

void Scene::draw_transparent_models() {
	sort_transparent_models();

	m_shaders["multi"].set_state(ShaderState::FACE_CULLING, false);
	m_shaders["multi"].use();
	for (auto& trans_obj : m_transparent_models) {
		m_shaders["multi"]["model"] = trans_obj.get_model_mat();
		m_shaders["multi"]["normal_mat"] = trans_obj.get_normal_mat();
		trans_obj.draw(m_shaders["multi"], "material");
	}
	m_shaders["multi"].set_state(ShaderState::FACE_CULLING, true); 
}

void Scene::transform_models() { 
	for (auto& inst_model : m_instanced_models) {
		auto positions = inst_model.get_positions();
		auto& rotations = inst_model.get_rotations();
		const float diff = AI_DEG_TO_RAD(0.1f);
		static float alfa = 0.f;
		const std::size_t N = positions.size();
		const float R = 200.f;
		const float PI2 = 6.283f;
		using EngType = std::default_random_engine;
		using DistType = std::normal_distribution<>;
		EngType eng;
		eng.seed();
		DistType dist(0.f, 1.0f); 
		for (std::size_t i = 0; i < positions.size(); ++i) {
			float deg = float(i) / N * PI2;
			positions[i].x += R * (cos(deg + alfa) - cos(deg + alfa - diff));
			positions[i].z += R * (sin(deg + alfa) - sin(deg + alfa - diff));

			rotations[i].x += dist(eng);
		}
		alfa += diff;
		if (alfa > PI2) {
			alfa = 0.f;
		}
		inst_model.push_positions(positions);
		inst_model.populate_model_mat_buffer();
		inst_model.populate_normal_mat_buffer();
	}
} 

Window* Scene::get_window() {
	return m_window;
}

Camera* Scene::get_camera() {
	return m_camera;
}

CurShaderType Scene::get_cur_shader() const {
	return m_cur_shader;
}

MaterialMap& Scene::get_default_material() {
	return m_default_material;
}

FrameBuffer& Scene::get_post_fb() {
	return m_fb_post_process;
}

CurShaderState& Scene::get_shader_state() {
	return m_shader_state;
}

std::vector<Model>& Scene::get_generic_models() {
	return m_generic_models;
}

std::vector<Model>& Scene::get_reflective_models() {
	return m_reflective_models;
}

std::vector<Model>& Scene::get_transparent_models() {
	return m_transparent_models;
}

std::vector<LitModel<PointLight>>& Scene::get_pointlight_sources() {
	return m_pointlight_sources;
}

std::vector<LitModel<SpotLight>>& Scene::get_spotlight_sources() {
	return m_spotlight_sources;
}

std::vector<LitModel<DirLight>>& Scene::get_dirlight_sources() {
	return m_dirlight_sources;
}

std::map<std::string, ShaderProgram>& Scene::get_shaders() {
	return m_shaders;
}

void process_input(Scene& a_scene) {
	Window& win = *a_scene.get_window();
	Camera& cam = *a_scene.get_camera();

	win.get_input_handle().process_input();
	{
		static bool m_tab_pressed = false;
		float delta = win.calculate_delta();

		if (glfwGetKey(win.get_obj(), GLFW_KEY_W))
			cam.process_keyboard(CameraMovement::FORWARD, delta);
		if (glfwGetKey(win.get_obj(), GLFW_KEY_S))
			cam.process_keyboard(CameraMovement::BACKWARD, delta);
		if (glfwGetKey(win.get_obj(), GLFW_KEY_D))
			cam.process_keyboard(CameraMovement::RIGHT, delta);
		if (glfwGetKey(win.get_obj(), GLFW_KEY_A))
			cam.process_keyboard(CameraMovement::LEFT, delta);
		if (glfwGetKey(win.get_obj(), GLFW_KEY_F1) == GLFW_PRESS)
			m_tab_pressed = true;
		if (m_tab_pressed && glfwGetKey(win.get_obj(), GLFW_KEY_F1) == GLFW_RELEASE) {
			m_tab_pressed = false;
			if (win.get_cursor_mode() == CursorMode::NORMAL)
				win.set_cursor_mode(CursorMode::FOCUSED);
			else
				win.set_cursor_mode(CursorMode::NORMAL);
		}
	} 
}

void Scene::post_process() {
	if (m_fb_post_process.get_id() == EMPTY_VBO)
		return;
 
	auto draw_with_shader = [](Model& screen_model, ShaderProgram& sh_p) {
			sh_p.set_state(ShaderState::FACE_CULLING, false);
			sh_p.set_state(ShaderState::DEPTH_TEST, false);
			sh_p.set_state(ShaderState::STENCIL_TEST, false);
			// sh_p.set_state(ShaderState::GAMMA_CORRECTION, true);
			sh_p["model"] = screen_model.get_model_mat();
			sh_p.use();
			screen_model.draw();
		};

	m_fb_post_process.unbind();
	glClearColor(0.1, 0.1, 0.1, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	std::string shader_name = "";
	switch (m_cur_shader) {
	case CurShaderType::POST_NONE: 
		shader_name = "post_none";
		break;
	case CurShaderType::MSAA_POST_NONE: 
		shader_name = "MSAA_post_none";
		m_shaders[shader_name]["sample_size"] = m_shader_state.m_MSAA_samples;
		break;
	case CurShaderType::POST_KERNEL: 
		shader_name = "post_kernel";
		m_shaders[shader_name]["kernel"] = m_shader_state.m_kernel;
		m_shaders[shader_name]["offset"] = m_shader_state.m_kernel_offset;
		break;
	case CurShaderType::POST_INV: 
		shader_name = "post_inv";
		m_shaders[shader_name]["inv_color"] = m_shader_state.m_inv_color;
		break;
	case CurShaderType::POST_GRAY_AVG: 
		shader_name = "post_gray_avg";
		break;
	case CurShaderType::POST_GRAY_WGT: 
		shader_name = "post_gray_wgt";
		break;
	case CurShaderType::NORMAL_VIS: 
		shader_name = "post_none";
		break;
	case CurShaderType::POST_GAMMA: 
		shader_name = "post_gamma";
		// m_shaders[shader_name]["gamma"] = m_shader_state.m_gamma;
		break;
	}

	m_fb_post_process.activate_color();
	draw_with_shader(m_basic_plane, m_shaders[shader_name]); 
}

void draw_gui(Scene& scene, Skybox& skybox1, Skybox& skybox2) {
	Window& win = *scene.get_window();
	Camera& cam = *scene.get_camera();
	std::vector<LitModel<DirLight>>& dirlight_sources = scene.get_dirlight_sources();
	std::vector<LitModel<SpotLight>>& spotlight_sources = scene.get_spotlight_sources();
	std::vector<LitModel<PointLight>>& pointlight_sources = scene.get_pointlight_sources();
	std::vector<Model>& generic_models = scene.get_generic_models();
	std::vector<Model>& transparent_models = scene.get_transparent_models();
	std::vector<Model>& reflective_models = scene.get_reflective_models();
	std::map<std::string, ShaderProgram>& shaders = scene.get_shaders();

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	{
		static ImGuiIO* im_io = win.get_imgui_handle().get_io();

		ImGui::ShowDemoWindow();

		ImGui::Begin("Config");
		if (ImGui::CollapsingHeader("Camera")) {
			imgui_cam(cam);
		}

		if (ImGui::CollapsingHeader("Lights")) {
			for (size_t i = 0; i < dirlight_sources.size(); i++) {
				if (ImGui::TreeNode(std::format("Directional light {}", i).c_str())) {
					imgui_dirlight(dirlight_sources[i].light);
					imgui_model(dirlight_sources[i].model);
					// if (ImGui::Button("Pop")) {
					// 	shaders["multi"].set_uniform(std::format("spotlight_sources[{}]", i), SpotLight(0, 0, glm::vec3(0), 0));
					// 	spotlight_sources.erase(spotlight_sources.begin() + i);
					// }
					ImGui::TreePop();
					ImGui::Spacing();
				}
			}

			for (size_t i = 0; i < spotlight_sources.size(); i++) {
				if (ImGui::TreeNode(std::format("Spotlight {}", i).c_str())) {
					imgui_spotlight(spotlight_sources[i].light);
					imgui_model(spotlight_sources[i].model);
					// if (ImGui::Button("Pop")) {
					// 	shaders["multi"].set_uniform(std::format("spotlight_sources[{}]", i), SpotLight(0, 0, glm::vec3(0), 0));
					// 	spotlight_sources.erase(spotlight_sources.begin() + i);
					// }
					ImGui::TreePop();
					ImGui::Spacing();
				}
			}

			for (size_t i = 0; i < pointlight_sources.size(); i++) {
				if (ImGui::TreeNode(std::format("Point light {}", i).c_str())) {
					imgui_pointlight(pointlight_sources[i].light);
					imgui_model(pointlight_sources[i].model);
					pointlight_sources[i].light.set_pos(pointlight_sources[i].model.get_pos());
					if (ImGui::Button("Pop")) {
						shaders["multi"].set_uniform(std::format("pointlight_sources[{}]", i), PointLight(0));
						pointlight_sources.erase(pointlight_sources.begin() + i);
					}
					ImGui::TreePop();
					ImGui::Spacing();
				}
			}

			if (ImGui::Button("Create point light")) {
				PointLight tmp_light(50, cam.get_position() + cam.get_target() * glm::vec3(2.5));
				pointlight_sources.emplace_back(tmp_light, pointlight_sources.at(0).model);
				pointlight_sources.back().model.set_pos(tmp_light.get_pos());
			}
		}

		if (ImGui::CollapsingHeader("Models")) {
			// if (ImGui::TreeNode("FrameBuffer post-processing")) {
			// 	imgui_model(basic_plane);
			// 	ImGui::TreePop();
			// 	ImGui::Spacing();
			// }
			if (ImGui::TreeNode("Generic")) {
				for (size_t i = 0; i < generic_models.size(); i++) {
					if (ImGui::TreeNode(std::format("[{}:{}]", i, wstos(generic_models[i].get_filename())).c_str())) {
						imgui_model(generic_models[i]);
						if (ImGui::Button("Pop")) {
							generic_models.erase(generic_models.begin() + i);
						}
						ImGui::TreePop();
						ImGui::Spacing();
					}
				}
				if (ImGui::Button("Create model")) {
					ImGui::OpenPopup("Invert UVs Popup");
				} 
				if (ImGui::BeginPopup("Invert UVs Popup")) {
					bool choice = false;
					bool invert_UVs = false;
					std::wstring path;
					ImGui::Text("Do you want to invert UVs?");
					if (ImGui::Button("Yes")) {
						choice = true;
						invert_UVs = true;
					}
					ImGui::SameLine();
					if (ImGui::Button("No")) {
						choice = true;
						invert_UVs = false;
					}
					if (choice) {
						path = ResourceManager::dialog_import_model();
						if (path != L"") {
							Model new_model = Application::get_instance().get_rmanager().load_model(path, invert_UVs);
							new_model.set_pos(cam.get_position() + cam.get_target() * glm::vec3(2.5));
							generic_models.push_back(new_model);
						}
					}
					ImGui::EndPopup();
					ImGui::CloseCurrentPopup();
				}
				if (ImGui::Button("Clear models")) {
					generic_models.clear();
				}
				ImGui::TreePop();
				ImGui::Spacing();
			}
			if (ImGui::TreeNode("Transparent")) {
				for (size_t i = 0; i < transparent_models.size(); i++) {
					if (ImGui::TreeNode(std::format("[{}:{}]", i, wstos(transparent_models[i].get_filename())).c_str())) {
						imgui_model(transparent_models[i]);
						if (ImGui::Button("Pop")) {
							transparent_models.erase(transparent_models.begin() + i);
						}
						ImGui::TreePop();
						ImGui::Spacing();
					}
				}
				if (ImGui::Button("Create model")) {
					ImGui::OpenPopup("Invert UVs Popup");
				} 
				if (ImGui::BeginPopup("Invert UVs Popup")) {
					bool choice = false;
					bool invert_UVs = false;
					std::wstring path;
					ImGui::Text("Do you want to invert UVs?");
					if (ImGui::Button("Yes")) {
						choice = true;
						invert_UVs = true;
					}
					ImGui::SameLine();
					if (ImGui::Button("No")) {
						choice = true;
						invert_UVs = false;
					}
					if (choice) {
						path = ResourceManager::dialog_import_model();
						if (path != L"") {
							Model new_model = Application::get_instance().get_rmanager().load_model(path, invert_UVs);
							new_model.set_pos(cam.get_position() + cam.get_target() * glm::vec3(2.5));
							transparent_models.push_back(new_model);
						}
					}
					ImGui::EndPopup();
					ImGui::CloseCurrentPopup();
				}
				if (ImGui::Button("Clear models")) {
					transparent_models.clear();
				}
				ImGui::TreePop();
				ImGui::Spacing();
			}
			if (ImGui::TreeNode("Reflective")) {
				for (size_t i = 0; i < reflective_models.size(); i++) {
					if (ImGui::TreeNode(std::format("[{}:{}]", i, wstos(reflective_models[i].get_filename())).c_str())) {
						imgui_model(reflective_models[i]);
						if (ImGui::Button("Pop")) {
							reflective_models.erase(reflective_models.begin() + i);
						}
						ImGui::TreePop();
						ImGui::Spacing();
					}
				}
				if (ImGui::Button("Create model")) {
					auto path = ResourceManager::dialog_import_model();
					if (path != L"") {
						Model new_model = Application::get_instance().get_rmanager().load_model(path, false);
						new_model.set_pos(cam.get_position() + cam.get_target() * glm::vec3(2.5));
						reflective_models.push_back(new_model);
					}
				} 
				if (ImGui::Button("Clear models")) {
					reflective_models.clear();
				}
				ImGui::TreePop();
				ImGui::Spacing();
			}
		} 
		if (ImGui::CollapsingHeader("Shaders")) { 
			auto& shader_state = scene.get_shader_state();

			int e{};
			switch (scene.get_cur_shader()) {
			case CurShaderType::POST_NONE:		e = 0; break;
			case CurShaderType::MSAA_POST_NONE: e = 1; break;
			case CurShaderType::POST_KERNEL:	e = 2; break;
			case CurShaderType::POST_INV:		e = 3; break;
			case CurShaderType::POST_GRAY_AVG:	e = 4; break;
			case CurShaderType::POST_GRAY_WGT:	e = 5; break;
			case CurShaderType::NORMAL_VIS:		e = 6; break;
			case CurShaderType::POST_GAMMA:		e = 7; break;
			};

			int samples = scene.get_post_fb().get_samples();
			float fog_dens = shader_state.m_fog_dens;
			float normal_mag = shader_state.m_normal_mag;
			float kernel_offset = shader_state.m_kernel_offset;
			float gamma = shader_state.m_gamma;
			glm::mat3 kernel = shader_state.m_kernel;
			glm::vec3 inverted_color = shader_state.m_inv_color;
			glm::vec3 normal_color = shader_state.m_normal_color;
			glm::vec3 fog_color = shader_state.m_fog_color;
			bool blinn_phong = shader_state.m_blinn_phong;
			int tmp_sample = samples;

			ImGui::RadioButton("None", &e, 0); 
			ImGui::RadioButton("MSAA", &e, 1);
			ImGui::RadioButton("Kernel", &e, 2);
			ImGui::RadioButton("Inverted color", &e, 3); 
			ImGui::RadioButton("Grayscale average", &e, 4);
			ImGui::RadioButton("Grayscale weighted", &e, 5);
			ImGui::RadioButton("Normal visualization", &e, 6);
			ImGui::RadioButton("Gamma correction", &e, 7);

			ImGui::SeparatorText("Configuration");
			if (ImGui::TreeNode("MSAA")) {
				float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
				ImGui::PushButtonRepeat(true);
				if (ImGui::ArrowButton("##left", ImGuiDir_Left)) { 
					if (--samples < 2) 
						samples = 2;
				}
				ImGui::SameLine(0.0f, spacing);
				if (ImGui::ArrowButton("##right", ImGuiDir_Right)) { 
					if (++samples > 16) {
						samples = 16;
					}
				}
				ImGui::PopButtonRepeat();
				ImGui::SameLine();
				ImGui::Text("%d", samples);
				ImGui::TreePop();
				ImGui::Spacing();
			}
			if (ImGui::TreeNode("Kernel")) {
				static int rad_ker = 2;
				static glm::vec3 kernel_row1{};
				static glm::vec3 kernel_row2{};
				static glm::vec3 kernel_row3{};
				ImGui::SliderFloat("Sampling offset", &kernel_offset, 0.0f, 1.0f); 
				ImGui::RadioButton("Use blur kernel", &rad_ker, 0);
				ImGui::RadioButton("Use sharp kernel", &rad_ker, 1);
				ImGui::RadioButton("Use edge detection kernel", &rad_ker, 2);
				ImGui::RadioButton("Use custom kernel", &rad_ker, 3);
				ImGui::DragFloat3("Row 1", (float*)&kernel_row1, 0.1f, -100.0f, 100.0f);
				ImGui::DragFloat3("Row 2", (float*)&kernel_row2, 0.1f, -100.0f, 100.0f);
				ImGui::DragFloat3("Row 3", (float*)&kernel_row3, 0.1f, -100.0f, 100.0f);

				switch (rad_ker) {
				case 0:
					kernel[0][0] = 1.f / 16.f; kernel[0][1] = 2.f / 16.f; kernel[0][2] = 1.f / 16.f;
					kernel[1][0] = 2.f / 16.f; kernel[1][1] = 4.f / 16.f; kernel[1][2] = 2.f / 16.f;
					kernel[2][0] = 1.f / 16.f; kernel[2][1] = 2.f / 16.f; kernel[2][2] = 1.f / 16.f;
					break;
				case 1:
					kernel[0][0] = 2.f; kernel[0][1] =   2.f; kernel[0][2] = 2.f;
					kernel[1][0] = 2.f; kernel[1][1] = -15.f; kernel[1][2] = 2.f;
					kernel[2][0] = 2.f; kernel[2][1] =   2.f; kernel[2][2] = 2.f;
					break;
				case 2:
					kernel[0][0] = 1.f; kernel[0][1] =  1.f; kernel[0][2] = 1.f;
					kernel[1][0] = 1.f; kernel[1][1] = -8.f; kernel[1][2] = 1.f;
					kernel[2][0] = 1.f; kernel[2][1] =  1.f; kernel[2][2] = 1.f;
					break;
				case 3:
					kernel[0] = kernel_row1; kernel[1] = kernel_row2; kernel[2] = kernel_row3;
					kernel = glm::transpose(kernel);
					break;
				}
				ImGui::TreePop();
				ImGui::Spacing();
			}
			if (ImGui::TreeNode("Color inversion")) { 
				ImGui::ColorEdit3("Inverted color", (float*)&inverted_color);
				ImGui::TreePop();
				ImGui::Spacing();
			}
			if (ImGui::TreeNode("Gamma")) {
				ImGui::SliderFloat("Gamma exponent", &gamma, 0.0f, 50.0f); 
				ImGui::TreePop();
				ImGui::Spacing();
			}
			if (ImGui::TreeNode("Other")) { 
				ImGui::SeparatorText("Normal visualization");
				ImGui::SliderFloat("Normal vector magnitude", &normal_mag, 0.0f, 100.0f); 
				ImGui::ColorEdit3("Normal vector color", (float*)&normal_color);
				ImGui::SeparatorText("Multi lights");
				ImGui::SliderFloat("Fog density", &fog_dens, 0.0f, 10.0f); 
				ImGui::ColorEdit3("Fog color", (float*)&fog_color);
				ImGui::Text("Blinn-phong: "); ImGui::SameLine();
				ToggleButton("toggl-blin-phong", &blinn_phong);
				ImGui::TreePop();
				ImGui::Spacing();
			}

			if (e == 1) {
				samples = (samples == 1) ? 2 : samples;
				auto& fb = scene.get_post_fb();
				fb.set_samples(samples);
			}
			else {
				auto& fb = scene.get_post_fb();
				fb.set_samples(1);
			}

			shader_state.m_fog_dens = fog_dens;
			shader_state.m_fog_color = fog_color; 
			shader_state.m_normal_color = normal_color;
			shader_state.m_normal_mag = normal_mag;
			shader_state.m_MSAA_samples = samples;
			shader_state.m_kernel = kernel; 
			shader_state.m_kernel_offset = kernel_offset;
			shader_state.m_inv_color = inverted_color; 
			shader_state.m_blinn_phong = blinn_phong;
			shader_state.m_gamma = gamma;

			switch (e) {
			case 0: scene.set_cur_shader(CurShaderType::POST_NONE); break;
			case 1: scene.set_cur_shader(CurShaderType::MSAA_POST_NONE); break;
			case 2: scene.set_cur_shader(CurShaderType::POST_KERNEL); break;
			case 3: scene.set_cur_shader(CurShaderType::POST_INV); break;
			case 4: scene.set_cur_shader(CurShaderType::POST_GRAY_AVG); break;
			case 5: scene.set_cur_shader(CurShaderType::POST_GRAY_WGT); break;
			case 6: scene.set_cur_shader(CurShaderType::NORMAL_VIS); break;
			case 7: scene.set_cur_shader(CurShaderType::POST_GAMMA); break;
			}
		}
		if (ImGui::Button("Change skybox")) {
			static int skybox_id = 0;
			if (skybox_id == 0) {
				scene.set_skybox(skybox1);
				skybox_id++;
			}
			else if (skybox_id++ == 1) {
				scene.set_skybox(skybox2); 
				skybox_id = 0;
			}
		}

		// Performance
		ImGui::SeparatorText("Misc");
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / im_io->Framerate, im_io->Framerate);
		ImGui::Text("Camera front vector: (%.2f, %.2f, %.2f)", cam.get_target()[0], cam.get_target()[1], cam.get_target()[2]);
		ImGui::Text("Camera right vector: (%.2f, %.2f, %.2f)", cam.get_right()[0], cam.get_right()[1], cam.get_right()[2]);
		ImGui::Text("Camera up vector: (%.2f, %.2f, %.2f)", cam.get_up()[0], cam.get_up()[1], cam.get_up()[2]);
		ImGui::Text("Camera world_up vector: (%.2f, %.2f, %.2f)", cam.get_world_up()[0], cam.get_world_up()[1], cam.get_world_up()[2]);
		ImGui::Text("Camera position vector: (%.2f, %.2f, %.2f)", cam.get_position()[0], cam.get_position()[1], cam.get_position()[2]);
		ImGui::End();
	}

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData()); 
}

void imgui_cam(Camera& cam) {
	float im_fov = cam.get_fov();
	float im_speed = cam.get_movement_speed();
	float im_far_plane = cam.get_far_plane();
	float im_near_plane = cam.get_near_plane();

	// Camera config
	ImGui::SliderFloat("Speed", &im_speed, 1.0f, 100.0f);
	ImGui::SliderFloat("Fov", &im_fov, 0.0f, 360.f);
	ImGui::SliderFloat("Near plane", &im_near_plane, 0.0f, 10.f);
	ImGui::SliderFloat("Far plane", &im_far_plane, 0.0f, 3000.f);

	cam.set_movement_speed(im_speed);
	cam.set_fov(im_fov);
	cam.set_near_plane(im_near_plane);
	cam.set_far_plane(im_far_plane);
}

void imgui_dirlight(DirLight& dirlight_source) {
	glm::vec3 im_dirlight_color = dirlight_source.get_color();
	glm::vec3 im_dirlight_dir = dirlight_source.get_dir();
	float im_dirlight_ambient_intens = dirlight_source.get_ambient()[0];
	float im_dirlight_diffuse_intens = dirlight_source.get_diffuse()[0];
	float im_dirlight_specular_intens = dirlight_source.get_specular()[0];

	// dirlights config
	ImGui::ColorEdit3("Light color", (float*)&im_dirlight_color);
	ImGui::DragFloat3("Light direction", (float*)&im_dirlight_dir, 0.1f, -10.0f, 10.0f);
	ImGui::SliderFloat("Ambient intensity", &im_dirlight_ambient_intens, 0.0f, 1.0f);
	ImGui::SliderFloat("Diffuse intensity", &im_dirlight_diffuse_intens, 0.0f, 1.0f);
	ImGui::SliderFloat("Specular intensity", &im_dirlight_specular_intens, 0.0f, 1.0f);

	dirlight_source.set_color(im_dirlight_color);
	dirlight_source.set_dir(im_dirlight_dir);
	dirlight_source.set_ambient_intens(glm::vec3(im_dirlight_ambient_intens));
	dirlight_source.set_diffuse_intens(glm::vec3(im_dirlight_diffuse_intens));
	dirlight_source.set_specular_intens(glm::vec3(im_dirlight_specular_intens));
}

void imgui_pointlight(PointLight& pointlight_source) {
	glm::vec3 im_pointlight_color = pointlight_source.get_color();
	float im_pointlight_ambient_intens = pointlight_source.get_ambient()[0];
	float im_pointlight_diffuse_intens = pointlight_source.get_diffuse()[0];
	float im_pointlight_specular_intens = pointlight_source.get_specular()[0];
	float im_pointlight_max_distance = pointlight_source.get_max_dist();
	float im_pointlight_linear_att = pointlight_source.get_linear();
	float im_pointlight_constant_att = pointlight_source.get_constant();
	float im_pointlight_quadratic_att = pointlight_source.get_quadratic();

	// Pointlights config
	ImGui::SeparatorText("Color");
	ImGui::ColorEdit3("Light color", (float*)&im_pointlight_color);
	ImGui::SeparatorText("Travel Distance");
	ImGui::SliderFloat("Max distance", &im_pointlight_max_distance, 0.0f, 300.0f);
	ImGui::Text("Constant attenuation: %.3f", im_pointlight_constant_att);
	ImGui::Text("Linear attenuation: %.3f", im_pointlight_linear_att);
	ImGui::Text("Quadratic attenuation: %.3f", im_pointlight_quadratic_att);
	ImGui::SeparatorText("Intensities");
	ImGui::SliderFloat("Ambient intensity", &im_pointlight_ambient_intens, 0.0f, 1.0f);
	ImGui::SliderFloat("Diffuse intensity", &im_pointlight_diffuse_intens, 0.0f, 1.0f);
	ImGui::SliderFloat("Specular intensity", &im_pointlight_specular_intens, 0.0f, 1.0f);

	pointlight_source.set_color(im_pointlight_color);
	pointlight_source.set_max_dist(im_pointlight_max_distance);
	pointlight_source.set_ambient_intens(glm::vec3(im_pointlight_ambient_intens));
	pointlight_source.set_diffuse_intens(glm::vec3(im_pointlight_diffuse_intens));
	pointlight_source.set_specular_intens(glm::vec3(im_pointlight_specular_intens));
}

void imgui_spotlight(SpotLight& spotlight_source) {
	glm::vec3 im_spotlight_color = spotlight_source.get_color();
	float colors[3] = { im_spotlight_color[0], im_spotlight_color[1], im_spotlight_color[2] };
	auto im_spotlight_ambient_intens = spotlight_source.get_ambient()[0];
	auto im_spotlight_diffuse_intens = spotlight_source.get_diffuse()[0];
	auto im_spotlight_specular_intens = spotlight_source.get_specular()[0];
	auto im_spotlight_max_distance = spotlight_source.get_max_dist();
	auto im_spotlight_linear_att = spotlight_source.get_linear();
	auto im_spotlight_constant_att = spotlight_source.get_constant();
	auto im_spotlight_quadratic_att = spotlight_source.get_quadratic();
	auto im_spotlight_inner_cutoff = spotlight_source.get_inner_cutoff_deg();
	auto im_spotlight_outer_cutoff = spotlight_source.get_outer_cutoff_deg();

	ImGui::SeparatorText("Color");
	ImGui::ColorEdit3("Light color", colors);
	ImGui::SeparatorText("Travel Distance");
	ImGui::SliderFloat("Max distance", &im_spotlight_max_distance, 0.0f, 300.0f);
	ImGui::Text("Constant attenuation: %.3f", im_spotlight_constant_att);
	ImGui::Text("Linear attenuation: %.3f", im_spotlight_linear_att);
	ImGui::Text("Quadratic attenuation: %.3f", im_spotlight_quadratic_att);
	ImGui::SeparatorText("Intensities");
	ImGui::SliderFloat("Ambient intensity", &im_spotlight_ambient_intens, 0.0f, 1.0f);
	ImGui::SliderFloat("Diffuse intensity", &im_spotlight_diffuse_intens, 0.0f, 1.0f);
	ImGui::SliderFloat("Specular intensity", &im_spotlight_specular_intens, 0.0f, 1.0f);
	ImGui::SeparatorText("Angles");
	ImGui::SliderFloat("Inner cutoff angle", &im_spotlight_inner_cutoff, 0.0f, 360.0f);
	ImGui::SliderFloat("Outer cutoff angle", &im_spotlight_outer_cutoff, 0.0f, 360.0f);

	spotlight_source.set_color(glm::vec3(colors[0], colors[1], colors[2]));
	spotlight_source.set_max_dist(im_spotlight_max_distance);
	spotlight_source.set_ambient_intens(glm::vec3(im_spotlight_ambient_intens));
	spotlight_source.set_diffuse_intens(glm::vec3(im_spotlight_diffuse_intens));
	spotlight_source.set_specular_intens(glm::vec3(im_spotlight_specular_intens));
	spotlight_source.set_inner_cutoff(im_spotlight_inner_cutoff);
	spotlight_source.set_outer_cutoff(im_spotlight_outer_cutoff);
}

void imgui_model(Model& model) {
	auto pos = model.get_pos();
	auto siz = model.get_size();
	auto rot = model.get_rotation();
	auto outline = model.is_outlined();
	auto outline_thicc = model.get_outline_thickness();
	auto outline_color = model.get_outline_color();
	float scale_siz = 1.f;

	float pos_vec[3] = { pos.x, pos.y, pos.z };
	float siz_vec[3] = { siz.x, siz.y, siz.z };
	float angles_deg_vec[3] = { rot.x, rot.y, rot.z };

	ImGui::SeparatorText("Info");
	ImGui::Text(std::format("Full path: {}", wstos(model.get_dir())).c_str());
	ImGui::Text(std::format("Position: ({},{},{})", pos_vec[0], pos[1], pos[2]).c_str());
	ImGui::Text(std::format("Size: ({},{},{})", siz[0], siz[1], siz[2]).c_str());
	ImGui::Text(std::format("Rotation (deg): (X:{}, Y:{}, Z:{})", angles_deg_vec[0], angles_deg_vec[1], angles_deg_vec[2]).c_str());

	ImGui::SeparatorText("Config");
	ImGui::DragFloat3("Set position: ", pos_vec, 1.0f, -1000.0f, 1000.0f);
	ImGui::DragFloat3("Set size: ", siz_vec, 0.1f, -100.0f, 100.0f);
	ImGui::DragFloat3("Set rotation: ", angles_deg_vec, 0.1f, -360.0f, 360.0f);
	ImGui::DragFloat("Scale size: ", &scale_siz, 0.1f, 0.f, 100.f);
	ImGui::SeparatorText("Outline config");
	ImGui::Checkbox("Enable",  &outline);
	ImGui::SliderFloat("Thickness", &outline_thicc, 1.0f, 2.f); 
	ImGui::ColorEdit3("Color", (float*)&outline_color);

	if (ImGui::TreeNode("Meshes")) {
		auto& meshes = model.get_meshes();
		for (size_t i = 0; i < meshes.size(); i++) {
			bool visible_id = meshes[i].get_visibility();
			bool wireframe_id = meshes[i].get_wireframe();
			float shininess = meshes[i].get_material_map().get_shininess();
			BufferDrawType draw_type = meshes[i].get_draw_mode();
			if (ImGui::TreeNode(std::format("mesh {}", i).c_str())) {
				ImGui::Text("Visibility: "); ImGui::SameLine();
				ToggleButton("toggl_visib", &visible_id); 
				ImGui::Text("Wireframe: "); ImGui::SameLine();
				ToggleButton("toggl_wire", &wireframe_id);
				ImGui::SliderFloat("Shininess", &shininess, 0.0f, 256.f); 

				auto val = to_enum_elem_type(draw_type);
				if (ImGui::Combo("Draw type", &val, "Points\0Lines\0Line Strip\0Line Loop\0Triangles\0Triangle Strip\0Triangle Fan\0")) {
					draw_type = BufferDrawType(val);
				}

				ImGui::TreePop();
				ImGui::Spacing();
			}

			meshes[i].set_visibility(visible_id);
			meshes[i].set_wireframe(wireframe_id);
			meshes[i].set_draw_mode(draw_type);
			meshes[i].get_material_map().set_shininess(shininess);
		}
		ImGui::TreePop();
		ImGui::Spacing();
	}

	model.set_pos(glm::vec3(pos_vec[0], pos_vec[1], pos_vec[2]));
	model.set_size(glm::vec3(siz_vec[0], siz_vec[1], siz_vec[2]) * glm::vec3(scale_siz));
	model.set_rotation(glm::vec3(angles_deg_vec[0], angles_deg_vec[1], angles_deg_vec[2]));
	model.set_outline(outline);
	model.set_outline_thickness(outline_thicc);
	model.set_outline_color(outline_color);
}


/* 
================= GRAVEYARD =================
{
	Model copy_screen(basic_plane);
	rear_window.activate_color();
	copy_screen.set_size(basic_plane.get_size() * glm::vec3(0.3) * glm::vec3(1.5, 0.5, 1));
	copy_screen.set_pos(basic_plane.get_pos() + (basic_plane.get_size() * glm::vec3(0, 0.85, 0)));
	draw_with_shader(copy_screen, post_none_shader);

	basic_plane.set_size(glm::vec3(0.5)); 

	use_shader(post_grayscale_average_shader);
	basic_plane.set_pos(glm::vec3(-0.5, 0.5, 0));
	basic_plane.draw(); 

	use_shader(post_inverse_shader);
	basic_plane.set_pos(glm::vec3(0.5, 0.5, 0));
	basic_plane.draw();

	use_shader(post_grayscale_weighted_shader);
	basic_plane.set_pos(glm::vec3(-0.5, -0.5, 0));
	basic_plane.draw();

	use_shader(post_sharpen_kernel);
	basic_plane.set_pos(glm::vec3(0.5, -0.5, 0));
	basic_plane.draw(); 
}
*/
