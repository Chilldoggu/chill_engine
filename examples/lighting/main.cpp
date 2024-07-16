#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/trigonometric.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/fwd.hpp>
#include <imgui/imgui.h>
#include <imgui/backend/imgui_impl_glfw.h>

#include <string>
#include <format>

#include "light.hpp"
#include "app.hpp"
#include "window.hpp"
#include "shaders.hpp"
#include "model.hpp"

void imgui_pointlight(PointLight& pointlight_source);
void imgui_dirlight(DirLight& dirlight_source);
void imgui_cam(Camera& cam);
void imgui_spotlight(SpotLight& spotlight_source);

int main() {
	App app{ 1920, 1080, "OpenGL", CursorMode::NORMAL, glm::vec3(0.0f, 0.0f, 10.0f) };
	Camera& cam = app.get_cam();
	Window& win = app.get_win();

	// Configure player attributes
	cam.set_movement_speed(8.0f);

	// Configure shaders
	app.new_shader("multi_light", {{ ShaderType::VERTEX, "main.vsh" }, { ShaderType::FRAGMENT, "multi_light.fsh" }});
	app.new_shader("lightbulb", {{ ShaderType::VERTEX, "main.vsh" }, { ShaderType::FRAGMENT, "lightbulb.fsh" }});

	ShaderProgram& lightbulb_shader = app.get_shader("lightbulb");
	ShaderProgram& multi_light_shader = app.get_shader("multi_light");

	// Set light sources
	DirLight dirlight_source{ glm::vec3(2.0f, -1.0f, -0.5f) };
	SpotLight spotlight_source{ 15, 22, cam.get_target(), 50, cam.get_position() };
	std::vector<PointLight> pointlight_sources{ 
		PointLight(50, glm::vec3( 2.3f, -3.3f, -4.0f)),
		PointLight(50, glm::vec3( 0.7f,  0.2f,  2.0f)), 
		PointLight(50, glm::vec3( 0.0f,  0.0f, -3.0f)) ,
		PointLight(50, glm::vec3(-4.0f,  2.0f, -12.0f)), 
	};

	// Set point light sources cube representations
	std::vector<Model> pointlight_cubes(pointlight_sources.size(), Model("assets/container/container.obj"));
	for (size_t i = 0; i < pointlight_sources.size(); i++) {
		pointlight_cubes[i].set_pos(pointlight_sources[i].get_pos());
		pointlight_cubes[i].set_size(0.5);
	}

	std::vector<Model> boxes(4, Model("assets/container/container.obj"));
	boxes[0].set_pos(glm::vec3(0.0f, 0.0f, 0.0f));
	boxes[1].set_pos(glm::vec3(3.0f, 0.0f, 0.0f));
	boxes[2].set_pos(glm::vec3(8.0f, 0.0f, 0.0f));
	boxes[3].set_pos(glm::vec3(15.0f, 0.0f, 0.0f));

	auto imgui_body = [&]{
		ImGui::ShowDemoWindow();

		static ImGuiIO* im_io = win.get_io();

		ImGui::Begin("Config");

		if (ImGui::CollapsingHeader("Camera")) {
			imgui_cam(app.get_cam());
		}

		if (ImGui::CollapsingHeader("Lights")) {

			if (ImGui::TreeNode("Directional light")) {
				imgui_dirlight(dirlight_source);
				ImGui::TreePop();
				ImGui::Spacing();
			}

			if (ImGui::TreeNode("Spotlight")) {
				imgui_spotlight(spotlight_source);
				ImGui::TreePop();
				ImGui::Spacing();
			}

			for (size_t i = 0; i < pointlight_sources.size(); i++) {
				if (ImGui::TreeNode(std::format("Point light {}", i).c_str())) {
					imgui_pointlight(pointlight_sources[i]);
					if (ImGui::Button("Pop")) {
						multi_light_shader.set_uniform(std::format("pointlight_sources[{}]", i), PointLight(0));
						pointlight_sources.erase(pointlight_sources.begin() + i);
						pointlight_cubes.erase(pointlight_cubes.begin() + i);
					}
					ImGui::TreePop();
					ImGui::Spacing();
				}
			}
			if (ImGui::Button("Create point light")) {
				pointlight_sources.push_back(PointLight(50, cam.get_position() + cam.get_target()));
				pointlight_cubes.emplace_back("assets/container/container.obj");
				pointlight_cubes.back().set_pos(cam.get_position() + cam.get_target());
				pointlight_cubes.back().set_size(0.5);
			}
		}

		// Performance
		ImGui::SeparatorText("Misc");
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / im_io->Framerate, im_io->Framerate);
		ImGui::Text("Camera front vector: (%.2f, %.2f, %.2f)", cam.get_target()[0], cam.get_target()[1], cam.get_target()[2]);
		ImGui::Text("Camera position vector: (%.2f, %.2f, %.2f)", cam.get_position()[0], cam.get_position()[1], cam.get_position()[2]);
		ImGui::End();
	};

	// Define input and draw bodies
	auto custom_input = [&]{
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
			if (CursorMode mode = win.get_cursor_mode(); mode == CursorMode::NORMAL || mode == CursorMode::IDLE)
				win.change_cursor_mode(CursorMode::FIRST_PERSON);
			else
				win.change_cursor_mode(CursorMode::IDLE);
		}
	};

	auto draw_body = [&]{
		glm::mat4 view_mat = cam.get_look_at();
		glm::mat4 projection_mat = cam.get_projection_matrix(win.get_width(), win.get_height());
		spotlight_source.set_pos(cam.get_position());
		spotlight_source.set_spot_dir(cam.get_target());

		// Uniforms
		multi_light_shader["view"] = view_mat;
		multi_light_shader["projection"] = projection_mat;
		multi_light_shader["view_pos"] = cam.get_position();
		// Set light source uniforms
		multi_light_shader.set_uniform("dirlight_source", dirlight_source);
		multi_light_shader.set_uniform("spotlight_source", spotlight_source);
		for (size_t i = 0; i < pointlight_sources.size(); i++) {
			multi_light_shader.set_uniform(std::format("pointlight_sources[{}]", i), pointlight_sources[i]);
		}

		lightbulb_shader["view"] = view_mat;
		lightbulb_shader["projection"] = projection_mat;

		for (size_t i = 0; i < pointlight_cubes.size(); i++) {
			lightbulb_shader["model"] = pointlight_cubes[i].get_model_mat();
			lightbulb_shader["light_color"] = pointlight_sources[i].get_color();
			pointlight_cubes[i].draw(lightbulb_shader, "");
		}

		for (auto& box : boxes) {
			multi_light_shader["normal_mat"] = box.get_normal_mat();
			multi_light_shader["model"] = box.get_model_mat();
			box.draw(multi_light_shader, "material");
		}
	};

	win.draw(glm::vec3(0.1), draw_body, custom_input, imgui_body);
}

void imgui_pointlight(PointLight& pointlight_source) {
	glm::vec3 im_pointlight_color = pointlight_source.get_color();
	float im_pointlight_ambient_intens  = pointlight_source.get_ambient()[0];
	float im_pointlight_diffuse_intens  = pointlight_source.get_diffuse()[0];
	float im_pointlight_specular_intens = pointlight_source.get_specular()[0];
	float im_pointlight_linear_att    = pointlight_source.get_linear();
	float im_pointlight_constant_att  = pointlight_source.get_constant();
	float im_pointlight_quadratic_att = pointlight_source.get_quadratic();

	// Pointlights config
	ImGui::ColorEdit3("Light color", (float*)&im_pointlight_color);
	ImGui::SliderFloat("Ambient intensity", &im_pointlight_ambient_intens, 0.0f, 1.0f);
	ImGui::SliderFloat("Diffuse intensity", &im_pointlight_diffuse_intens, 0.0f, 1.0f);
	ImGui::SliderFloat("Specular intensity", &im_pointlight_specular_intens, 0.0f, 1.0f);
	ImGui::SliderFloat("Constant attenuation", &im_pointlight_constant_att, 0.0f, 10.0f);
	ImGui::SliderFloat("Linear attenuation", &im_pointlight_linear_att, 0.0f, 1.0f);
	ImGui::SliderFloat("Quadratic attenuation", &im_pointlight_quadratic_att, 0.0f, 2.0f);

	pointlight_source.set_color(im_pointlight_color);
	pointlight_source.set_ambient_intens(glm::vec3(im_pointlight_ambient_intens));
	pointlight_source.set_diffuse_intens(glm::vec3(im_pointlight_diffuse_intens));
	pointlight_source.set_specular_intens(glm::vec3(im_pointlight_specular_intens));
	pointlight_source.set_const_att(im_pointlight_constant_att);
	pointlight_source.set_linear_att(im_pointlight_linear_att);
	pointlight_source.set_quadratic_att(im_pointlight_quadratic_att);
}

void imgui_dirlight(DirLight& dirlight_source) {
	glm::vec3 im_dirlight_color = dirlight_source.get_color();
	glm::vec3 im_dirlight_dir = dirlight_source.get_dir();
	float im_dirlight_ambient_intens  = dirlight_source.get_ambient()[0];
	float im_dirlight_diffuse_intens  = dirlight_source.get_diffuse()[0];
	float im_dirlight_specular_intens = dirlight_source.get_specular()[0];

	// dirlights config
	ImGui::DragFloat3("Light direction", (float*)&im_dirlight_dir, 0.1f, -10.0f, 10.0f);
	ImGui::ColorEdit3("Light color", (float*)&im_dirlight_color);
	ImGui::SliderFloat("Ambient intensity", &im_dirlight_ambient_intens, 0.0f, 1.0f);
	ImGui::SliderFloat("Diffuse intensity", &im_dirlight_diffuse_intens, 0.0f, 1.0f);
	ImGui::SliderFloat("Specular intensity", &im_dirlight_specular_intens, 0.0f, 1.0f);

	dirlight_source.set_color(im_dirlight_color);
	dirlight_source.set_dir(im_dirlight_dir);
	dirlight_source.set_ambient_intens(glm::vec3(im_dirlight_ambient_intens));
	dirlight_source.set_diffuse_intens(glm::vec3(im_dirlight_diffuse_intens));
	dirlight_source.set_specular_intens(glm::vec3(im_dirlight_specular_intens));
}

void imgui_cam(Camera& cam) {
	float im_fov = cam.get_fov();
	float im_speed = cam.get_movement_speed();
	float im_far_plane  = cam.get_far_plane();
	float im_near_plane = cam.get_near_plane();

	// Camera config
	ImGui::SliderFloat("Speed", &im_speed, 1.0f, 20.0f);
	ImGui::SliderFloat("Fov", &im_fov, 0.0f, 360.f);
	ImGui::SliderFloat("Near plane", &im_near_plane, 0.0f, 10.f);
	ImGui::SliderFloat("Far plane", &im_far_plane, 0.0f, 1000.f);

	cam.set_movement_speed(im_speed);
	cam.set_fov(im_fov);
	cam.set_near_plane(im_near_plane);
	cam.set_far_plane(im_far_plane);
}

void imgui_spotlight(SpotLight& spotlight_source) {
	glm::vec3 im_spotlight_color = spotlight_source.get_color();
	float colors[3] = { im_spotlight_color[0], im_spotlight_color[1], im_spotlight_color[2] };
	float im_spotlight_ambient_intens  = spotlight_source.get_ambient()[0];
	float im_spotlight_diffuse_intens  = spotlight_source.get_diffuse()[0];
	float im_spotlight_specular_intens = spotlight_source.get_specular()[0];
	float im_spotlight_linear_att    = spotlight_source.get_linear();
	float im_spotlight_constant_att  = spotlight_source.get_constant();
	float im_spotlight_quadratic_att = spotlight_source.get_quadratic();
	float im_spotlight_inner_cutoff = spotlight_source.get_inner_cutoff_deg();
	float im_spotlight_outer_cutoff = spotlight_source.get_outer_cutoff_deg();

	ImGui::ColorEdit3("Light color", colors);
	ImGui::SliderFloat("Spot Ambient intensity", &im_spotlight_ambient_intens, 0.0f, 1.0f);
	ImGui::SliderFloat("Diffuse intensity", &im_spotlight_diffuse_intens, 0.0f, 1.0f);
	ImGui::SliderFloat("Specular intensity", &im_spotlight_specular_intens, 0.0f, 1.0f);
	ImGui::SliderFloat("Constant attenuation", &im_spotlight_constant_att, 0.0f, 10.0f);
	ImGui::SliderFloat("Linear attenuation", &im_spotlight_linear_att, 0.0f, 1.0f);
	ImGui::SliderFloat("Quadratic attenuation", &im_spotlight_quadratic_att, 0.0f, 2.0f);
	ImGui::SliderFloat("Inner cutoff angle", &im_spotlight_inner_cutoff, 0.0f, 360.0f);
	ImGui::SliderFloat("Outer cutoff angle", &im_spotlight_outer_cutoff, 0.0f, 360.0f);

	spotlight_source.set_color(glm::vec3(colors[0], colors[1], colors[2]));
	spotlight_source.set_ambient_intens(glm::vec3(im_spotlight_ambient_intens));
	spotlight_source.set_diffuse_intens(glm::vec3(im_spotlight_diffuse_intens));
	spotlight_source.set_specular_intens(glm::vec3(im_spotlight_specular_intens));
	spotlight_source.set_const_att(im_spotlight_constant_att);
	spotlight_source.set_linear_att(im_spotlight_linear_att);
	spotlight_source.set_quadratic_att(im_spotlight_quadratic_att);
	spotlight_source.set_inner_cutoff(im_spotlight_inner_cutoff);
	spotlight_source.set_outer_cutoff(im_spotlight_outer_cutoff);
}
