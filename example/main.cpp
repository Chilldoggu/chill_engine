#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/trigonometric.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/fwd.hpp>
#include <imgui/imgui.h>
#include <imgui/backend/imgui_impl_glfw.h>

#include <random>
#include <string>
#include <chrono>

#include "light.hpp"
#include "app.hpp"
#include "window.hpp"
#include "shaders.hpp"
#include "figures.hpp"
#include "figures_constants.hpp"

using rng_eng = std::default_random_engine;
using dist_real = std::uniform_real_distribution<>;

template<typename T, typename W = T::result_type>
class RNG_functor {
private:
	rng_eng generator;
	T distribution;
public:
	RNG_functor(W a, W b) :generator{}, distribution{a, b} { 
		generator.seed(std::chrono::system_clock::now().time_since_epoch().count()); 
	}

	W operator()() { return distribution(generator); }
};

int main() {
	App app{ 1920, 1080, "ChillEngine_example", CursorMode::FIRST_PERSON, glm::vec3(0.0f, 8.0f, 10.0f) };
	Camera& cam = app.get_cam();
	Window& win = app.get_win();

	// Create Cube VBOs for later reuse
	VBO_FIGURES VBO_CUBE;
	VBO_CUBE.vert_sum = CUBE_VERT_CORDS.size() / 3;
	VBO_CUBE.VERTS = VBO_generate(CUBE_VERT_CORDS);
	VBO_CUBE.TEXTURE = VBO_generate(CUBE_TEXTURE_CORDS);
	VBO_CUBE.NORMALS = VBO_generate(CUBE_NORMAL_CORDS);

	// Configure player attributes
	cam.set_movement_speed(8.0f);

	// Configure shaders
	app.new_shader("object", {{ ShaderType::VERTEX, "main.vsh" }, { ShaderType::FRAGMENT, "object.fsh" }});
	app.new_shader("lightbulb", {{ ShaderType::VERTEX, "main.vsh" }, { ShaderType::FRAGMENT, "lightbulb.fsh" }});

	Shader_program& object_shader = app.get_shader("object");
	object_shader.set_depth_testing(true);
	object_shader.push_uniform("model", UniformType::MODEL_MAT);
	object_shader.push_uniform("view", UniformType::VIEW_MAT);
	object_shader.push_uniform("projection", UniformType::PROJECTION_MAT);
	object_shader.push_uniform("normal_mat", UniformType::NORMAL_MAT);
	object_shader.push_uniform("material", UniformType::MATERIAL);
	object_shader.push_uniform("light_source", UniformType::LIGHT);
	object_shader.push_uniform("time");
	object_shader.push_uniform("view_light_pos");

	Shader_program& lightbulb_shader = app.get_shader("lightbulb");
	lightbulb_shader.set_depth_testing(true);
	lightbulb_shader.push_uniform("model", UniformType::MODEL_MAT);
	lightbulb_shader.push_uniform("view", UniformType::VIEW_MAT);
	lightbulb_shader.push_uniform("projection", UniformType::PROJECTION_MAT);
	lightbulb_shader.push_uniform("light_source.color");

	// Configure lightbulb
	Light light_source;
	light_source.set_color(glm::vec3(ColorsRGB::WHITE[0], ColorsRGB::WHITE[1], ColorsRGB::WHITE[2]));
	light_source.set_ambient_intens(glm::vec3(0.05));
	light_source.set_diffuse_intens(glm::vec3(0.8f));
	light_source.set_specular_intens(glm::vec3(1.0f));

	Cube lightbulb{ Point3D(0.0f, 8.0f, 0.0f), 1.0f, 0.0f, VBO_CUBE };

	// Create test objects
	RNG_functor<dist_real> pos_generator(-6.0f, 6.0f);
	RNG_functor<dist_real> size_generator(0.5f, 2.0f);
	std::vector<Cube> boxes;
	for (int i = 0; i < 25; i++) {
		boxes.emplace_back(Point3D(pos_generator(),  pos_generator(), pos_generator()), size_generator(), 0.0f, VBO_CUBE);
	}

	MaterialMap container;
	container.set_diffuse_map("container2.png");
	container.set_specular_map("container2_specular.png");
	container.set_emission_map("matrix_emission.jpg");
	container.set_shininess(32.f);

	for (auto& box : boxes)
		box.set_material_map(container);

	// Create big wireframe container for test objects
	Cube massive_box{ Point3D(), 15.0f, 0.0f, VBO_CUBE, true};

	// Define imgui interface (copy pasted from imgui example repo)
	auto imgui_body = [&]{
		static bool show_demo_window = false;
		static bool show_another_window = false;
		static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
		static ImGuiIO* io = win.get_io();
		static float f = 0.0f;
		static int counter = 0;

        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);
		{
			ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

			ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
			ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
			ImGui::Checkbox("Another Window", &show_another_window);

			ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
			ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

			if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
				counter++;
			ImGui::SameLine();
			ImGui::Text("counter = %d", counter);

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io->Framerate, io->Framerate);
			ImGui::End();
		}
        if (show_another_window)
        {
            ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me"))
                show_another_window = false;
            ImGui::End();
        }
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
		if (glfwGetKey(win.get_obj(), GLFW_KEY_TAB) == GLFW_PRESS)
			m_tab_pressed = true;
		if (m_tab_pressed && glfwGetKey(win.get_obj(), GLFW_KEY_TAB) == GLFW_RELEASE) {
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

		// First populate uniforms
		object_shader["view"] = view_mat;
		object_shader["projection"] = projection_mat;
		object_shader["time"] = (float)glfwGetTime() * 0.7f;
		object_shader["view_light_pos"] = glm::vec3(view_mat * light_source.get_pos_dir());
		object_shader.set_uniform(light_source);

		lightbulb_shader["view"] = view_mat;
		lightbulb_shader["projection"] = projection_mat;
		lightbulb_shader["light_source.color"] = light_source.get_color();

		// Second draw
		massive_box.draw(lightbulb_shader);
		lightbulb.draw(lightbulb_shader);

		for (auto& box : boxes) {
			object_shader["normal_mat"] = glm::transpose(glm::inverse(glm::mat3(view_mat * box.get_model()))); 
			box.draw(object_shader);
		}

		// Third animations
		float time = glfwGetTime();
		float move_height = 8.0f;
		float move_width = 12.0f;
		glm::vec3 new_light_pos{ move_width * glm::sin(time), move_height * glm::cos(time), move_height * glm::sin(time) };
		lightbulb.set_pos(new_light_pos);
		light_source.set_pos_dir(glm::vec4(new_light_pos, 1.0f));
	};

	win.draw(draw_body, custom_input, imgui_body);
}

