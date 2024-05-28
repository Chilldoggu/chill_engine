#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/trigonometric.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/fwd.hpp>
#include <string>

#include "game.hpp"
#include "window.hpp"
#include "shaders.hpp"
#include "figures.hpp"
#include "figures_constants.hpp"

Game game{ 1920, 1080, "OpenGL", MODES::FPS, glm::vec3(0.0f, 8.0f, 10.0f) };

int main() {
	// Create Cube VBOs for later reuse
	VBO_FIGURES VBO_CUBE;
	VBO_CUBE.vert_sum = CUBE_VERT_CORDS.size() / 3;
	VBO_CUBE.VERTS = VBO_generate(CUBE_VERT_CORDS);
	VBO_CUBE.TEXTURE = VBO_generate(CUBE_TEXTURE_CORDS);
	VBO_CUBE.NORMALS = VBO_generate(CUBE_NORMAL_CORDS);

	// Configure player attributes
	game.m_player.m_head.set_movement_speed(8.0f);

	// Configure shaders
	game.new_shader("object", {{ ShaderType::VERTEX, "main.vsh" }, { ShaderType::FRAGMENT, "object.fsh" }});
	game.new_shader("lightbulb", {{ ShaderType::VERTEX, "main.vsh" }, { ShaderType::FRAGMENT, "lighbulb.fsh" }});
	Shader_program object_shader = game.get_shader("object");
	object_shader.set_depth_testing(true);
	object_shader.push_model("model");
	object_shader.push_view("view");
	object_shader.push_projection("projection");
	object_shader.push_normal("normal_mat");
	object_shader.push_material("material");
	object_shader.push_light("light_source");
	object_shader.push_uniform("time");

	Shader_program lightbulb_shader = game.get_shader("lightbulb");
	lightbulb_shader.set_depth_testing(true);
	lightbulb_shader.push_model("model");
	lightbulb_shader.push_view("view");
	lightbulb_shader.push_projection("projection");
	lightbulb_shader.push_light("light_source");

	// Configure lightbulb
	Light light_source;
	light_source.color = glm::vec3(ColorsRGB::WHITE[0], ColorsRGB::WHITE[1], ColorsRGB::WHITE[2]);
	light_source.ambient_intens = glm::vec3(0.05);
	light_source.diffuse_intens = glm::vec3(0.8f);
	light_source.specular_intens = glm::vec3(1.0f);

	Cube lightbulb{ Point3D(0.0f, 8.0f, 0.0f), 1.0f, 0.0f, VBO_CUBE };

	// Create test objects
	std::vector<Cube> boxes;
	boxes.emplace_back(Point3D(-12.0f), 0.5f, 0.0f, VBO_CUBE);
	boxes.emplace_back(Point3D( -9.0f), 1.0f, 0.0f, VBO_CUBE);
	boxes.emplace_back(Point3D( -6.0f), 1.5f, 0.0f, VBO_CUBE);
	boxes.emplace_back(Point3D( -3.0f), 1.0f, 0.0f, VBO_CUBE);
	boxes.emplace_back(Point3D(  0.0f), 0.5f, 0.0f, VBO_CUBE);
	boxes.emplace_back(Point3D(  3.0f), 1.0f, 0.0f, VBO_CUBE);
	boxes.emplace_back(Point3D(  6.0f), 1.5f, 0.0f, VBO_CUBE);
	boxes.emplace_back(Point3D(  9.0f), 1.0f, 0.0f, VBO_CUBE);
	boxes.emplace_back(Point3D( 12.0f), 0.5f, 0.0f, VBO_CUBE);

	MaterialMap container;
	container.set_diffuse_map("container2.png");
	container.set_specular_map("container2_specular.png");
	container.set_emission_map("matrix_emission.jpg");
	container.set_shininess(32.f);

	for (auto& box : boxes) {
		box.set_material_map(container);
	}

	// Define input and draw bodies
	GLFWwindow* win_obj = game.m_win.get_obj();

	auto custom_input = [&]{
		float delta = game.m_win.calculate_delta();

		if (glfwGetKey(win_obj, GLFW_KEY_W))
			game.m_player.m_head.process_keyboard(Camera_Movement::FORWARD, delta);
		if (glfwGetKey(win_obj, GLFW_KEY_S))
			game.m_player.m_head.process_keyboard(Camera_Movement::BACKWARD, delta);
		if (glfwGetKey(win_obj, GLFW_KEY_D))
			game.m_player.m_head.process_keyboard(Camera_Movement::RIGHT, delta);
		if (glfwGetKey(win_obj, GLFW_KEY_A))
			game.m_player.m_head.process_keyboard(Camera_Movement::LEFT, delta);
	};

	std::string light_name = object_shader.get_light_name(); // same for lightbulb shader
	auto draw_body = [&]{
		Point3D lightbulb_pos = lightbulb.get_pos();
		glm::mat4 view_mat = game.m_player.m_head.get_look_at();
		glm::mat4 projection_mat = game.m_player.m_head.get_projection_matrix(game.m_win.get_width(), game.m_win.get_height());

		// First populate uniforms
		object_shader["view"] = view_mat;
		object_shader["projection"] = projection_mat;
		object_shader[light_name + ".pos"] = std::vector<float>{ lightbulb_pos.x, lightbulb_pos.y, lightbulb_pos.z };
		object_shader[light_name + ".color"] = light_source.color;
		object_shader[light_name + ".ambient_intens"]  = light_source.ambient_intens;
		object_shader[light_name + ".diffuse_intens"]  = light_source.diffuse_intens;
		object_shader[light_name + ".specular_intens"] = light_source.specular_intens;
		object_shader["time"] = (float)glfwGetTime() * 0.7f;

		lightbulb_shader["view"] = view_mat;
		lightbulb_shader["projection"] = projection_mat;
		lightbulb_shader[light_name + ".color"] = light_source.color;

		// Second draw
		lightbulb.draw(lightbulb_shader);

		for (auto& box : boxes) {
			object_shader["normal_mat"] = glm::transpose(glm::inverse(glm::mat3(view_mat * box.get_model()))); 
			box.draw(object_shader);
		}

		// Third animations
		float time = glfwGetTime();
		float move_height = 8.0f;
		float move_width = 12.0f;
		std::vector<float> new_light_pos{ move_width * glm::sin(time), move_height * glm::cos(time), move_height * glm::sin(time) };
		lightbulb.set_pos(new_light_pos);
	};

	game.m_win.draw(draw_body, custom_input);
}

