#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/trigonometric.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/fwd.hpp>

#include <chrono>
#include <random>
#include <filesystem>

#include "chill_renderer/meshes.hpp"
#include "chill_renderer/light.hpp"
#include "chill_renderer/application.hpp"
#include "chill_renderer/window.hpp"
#include "chill_renderer/shaders.hpp"
#include "chill_renderer/model.hpp"
#include "chill_renderer/buffers.hpp"
#include "chill_renderer/presets.hpp"
#include "scene.hpp"

using namespace chill_renderer;
namespace fs = std::filesystem;

int main() {
	Application::init(1280, 720, "OpenGL", CursorMode::NORMAL);
	ResourceManager& rmanager = Application::get_instance().get_rmanager();

	Scene main_scene;
	main_scene.set_window(&Application::get_instance().get_win());
	main_scene.set_camera(&Application::get_instance().get_win().get_camera());

	main_scene.get_camera()->set_movement_speed(70.0f);
	main_scene.get_camera()->set_far_plane(800.f);
	main_scene.get_camera()->set_position(glm::vec3(0.f, 30.0f, 60.f));

	auto gpath = [](const auto& p) { return fs::path(p).wstring(); };

	main_scene.set_default_material(gpath("resources/Public/default.png"));

	// SHADERS
	main_scene.push_shader("multi", rmanager.new_shader(
		{ ShaderType::VERTEX, gpath("shaders/main.vert") },
		{ ShaderType::FRAGMENT, gpath("shaders/multi_light.frag") },
		//{ ShaderType::GEOMETRY, gpath("shaders/explosion.geom") })
		{ ShaderType::GEOMETRY, gpath("shaders/pass_through.geom") })
	);
	main_scene.push_shader("multi_instanced", rmanager.new_shader(
		{ ShaderType::VERTEX, gpath("shaders/multi_instanced.vert") },
		{ ShaderType::FRAGMENT, gpath("shaders/instanced.frag") },
		{ ShaderType::GEOMETRY, gpath("shaders/pass_through.geom") })
	);
	main_scene.push_shader("normal_vis", rmanager.new_shader(
		{ ShaderType::VERTEX, gpath("shaders/NormalVisualizer/normal_vis.vert") },
		{ ShaderType::FRAGMENT, gpath("shaders/NormalVisualizer/normal_vis.frag") },
		{ ShaderType::GEOMETRY, gpath("shaders/NormalVisualizer/normal_vis.geom") })
	);
	main_scene.push_shader("single", rmanager.new_shader(
		{ ShaderType::VERTEX, gpath("shaders/SingleColor/single_color.vert") },
		{ ShaderType::FRAGMENT, gpath("shaders/SingleColor/single_color.frag") })
	);
	main_scene.push_shader("skybox", rmanager.new_shader(
		{ ShaderType::VERTEX, gpath("shaders/skybox.vert") },
		{ ShaderType::FRAGMENT, gpath("shaders/skybox.frag") })
	);
	main_scene.push_shader("refl", rmanager.new_shader(
		{ ShaderType::VERTEX, gpath("shaders/main.vert") },
		{ ShaderType::FRAGMENT, gpath("shaders/reflection.frag") },
		{ ShaderType::GEOMETRY, gpath("shaders/pass_through.geom") })
	);
	main_scene.push_shader("refr", rmanager.new_shader(
		{ ShaderType::VERTEX, gpath("shaders/main.vert") },
		{ ShaderType::FRAGMENT, gpath("shaders/refraction.frag") },
		{ ShaderType::GEOMETRY, gpath("shaders/pass_through.geom") })
	);
	main_scene.push_shader("dynamic_env", rmanager.new_shader(
		{ ShaderType::VERTEX, gpath("shaders/dynamic_env.vert") },
		{ ShaderType::FRAGMENT, gpath("shaders/dynamic_env.frag") })
	);
	main_scene.push_shader("post_inv", rmanager.new_shader(
		{ ShaderType::VERTEX, gpath("shaders/PostProcess/post.vert") },
		{ ShaderType::FRAGMENT, gpath("shaders/PostProcess/post_inverse.frag") })
	);
	main_scene.push_shader("post_gray_avg", rmanager.new_shader(
		{ ShaderType::VERTEX, gpath("shaders/PostProcess/post.vert") },
		{ ShaderType::FRAGMENT, gpath("shaders/PostProcess/post_grayscale_average.frag") })
	);
	main_scene.push_shader("post_gray_wgt", rmanager.new_shader(
		{ ShaderType::VERTEX, gpath("shaders/PostProcess/post.vert") },
		{ ShaderType::FRAGMENT, gpath("shaders/PostProcess/post_grayscale_weighted.frag") })
	);
	main_scene.push_shader("post_kernel", rmanager.new_shader(
		{ ShaderType::VERTEX, gpath("shaders/PostProcess/post.vert") },
		{ ShaderType::FRAGMENT, gpath("shaders/PostProcess/post_kernel.frag") })
	);
	main_scene.push_shader("post_none", rmanager.new_shader(
		{ ShaderType::VERTEX, gpath("shaders/PostProcess/post.vert") },
		{ ShaderType::FRAGMENT, gpath("shaders/PostProcess/post_none.frag") })
	);
	main_scene.push_shader("MSAA_post_none", rmanager.new_shader(
		{ ShaderType::VERTEX, gpath("shaders/PostProcess/post.vert") },
		{ ShaderType::FRAGMENT, gpath("shaders/PostProcess/post_none_MSAA.frag") })
	);
	main_scene.push_shader("post_gamma", rmanager.new_shader(
		{ ShaderType::VERTEX, gpath("shaders/PostProcess/post.vert") },
		{ ShaderType::FRAGMENT, gpath("shaders/PostProcess/post_gamma.frag") })
	);

	// UBO
	UniformBuffer UBO{};
	UBO.push_elements<glm::mat4, glm::mat4>({ "view", "projection" });
	UBO.set_binding_point(0);
	if (!UBO.check_status())
		ERROR("[MAIN] Couldn't successfully create uniform buffer object.", Error_action::throwing);
	main_scene.push_uniform_buffer(UBO);

	// LIGHTS
	Model light_obj = rmanager.load_model(gpath("resources/Public/MIT/basic-shapes/sphere/sphere.obj"));
	light_obj.set_size(1.f / 3.f);
	Model dirlight_model = light_obj;
	dirlight_model.set_pos(glm::vec3(0.0f, 135.0f, 0.0f));
	dirlight_model.set_size(2.f);

	DirLight dirl(glm::vec3(2, -1, -0.5));
	dirl.set_color({0,0,0});
	main_scene.push_dirlight(LitModel{ .light = dirl, .model = dirlight_model });
	main_scene.push_spotlight(LitModel{ .light = SpotLight(15, 22, main_scene.get_camera()->get_target(), 0, main_scene.get_camera()->get_position()), .model = light_obj });
	main_scene.push_pointlight(LitModel{ .light = PointLight(50, main_scene.get_camera()->get_position()), .model = light_obj });

	// SKYBOX
	Skybox skybox_river{
		.cubemap = rmanager.load_cubemap({
				gpath("resources/Public/skybox/RiverMountains/right.jpg"),
				gpath("resources/Public/skybox/RiverMountains/left.jpg"),
				gpath("resources/Public/skybox/RiverMountains/top.jpg"),
				gpath("resources/Public/skybox/RiverMountains/bottom.jpg"),
				gpath("resources/Public/skybox/RiverMountains/front.jpg"),
				gpath("resources/Public/skybox/RiverMountains/back.jpg")
			}, 0),
		.cube = rmanager.create_model({ Mesh(presets::g_skybox_data, MaterialMap()) })
	};
	Skybox skybox_starmap{
		.cubemap = rmanager.load_cubemap({
				gpath("resources/Public/skybox/Starmap/4k/right.jpg"),
				gpath("resources/Public/skybox/Starmap/4k/left.jpg"),
				gpath("resources/Public/skybox/Starmap/4k/top.jpg"),
				gpath("resources/Public/skybox/Starmap/4k/bottom.jpg"),
				gpath("resources/Public/skybox/Starmap/4k/front.jpg"),
				gpath("resources/Public/skybox/Starmap/4k/back.jpg")
			}, 0),
		.cube = rmanager.create_model({ Mesh(presets::g_skybox_data, MaterialMap()) })
	};
	main_scene.set_skybox(skybox_starmap);

	// MODELS 
	// Planet
	Model planet = rmanager.load_model(gpath("resources/Public/LearnOpenGL/planet/planet.obj"));
	planet.set_size(10.f);
	main_scene.push_generic_model(planet);
	// Rock
	Model rock = rmanager.load_model(gpath("resources/Public/LearnOpenGL/rock/rock.obj"));
	ModelInstanced cloud(rock);
	Rand dice{};
	float R = 200.f;
	float displacement = 50.f;
	const float PI2 = 6.283f;
	for (int i = 0, N = 500; i < N; ++i) {
		float deg = float(i) / N * PI2;
		glm::vec3 obj_pos(
			std::cos(deg) * R + dice.roll_f(-displacement, displacement),
			dice.roll_f(-displacement, displacement) / 3, 
			std::sin(deg) * R + dice.roll_f(-displacement, displacement)
		);
		glm::vec3 obj_rot(dice.roll_vec3(0.f, 360.f));
		glm::vec3 obj_siz(dice.roll_f(0.5f, 2.f));

		cloud.push_position(obj_pos);
		cloud.push_rotation(obj_rot);
		cloud.push_size(obj_siz);
	}
	cloud.populate_model_mat_buffer();
	cloud.populate_normal_mat_buffer();
	main_scene.push_model_instanced(cloud);

	// POSTPROCESS FRAMEBUFFER
	// MSAA
	{
		FrameBuffer fb_MSAA_post(main_scene.get_window()->get_width(), main_scene.get_window()->get_height());
		fb_MSAA_post.attach(AttachmentType::COLOR_2D, AttachmentBufferType::MSAA_TEXTURE, 4);
		fb_MSAA_post.attach(AttachmentType::DEPTH_STENCIL, AttachmentBufferType::MSAA_RENDER_BUFFER, 4);
		if (!fb_MSAA_post.check_status()) {
			ERROR("[MAIN] Framebuffer fb_post is not complete!", Error_action::throwing);
		}
		main_scene.push_frame_buffer_post(std::move(fb_MSAA_post)); 
	}

	while (!main_scene.get_window()->closed()) {
		glfwPollEvents();

		process_input(main_scene); 

		main_scene.draw(); 
		main_scene.post_process();

		draw_gui(main_scene, skybox_river, skybox_starmap);

		glfwSwapBuffers(main_scene.get_window()->get_obj());
	}
}

/* ========== REFERENCE ==========
// Container
Model container = rmanager.load_model(L"assets\\LearnOpenGL\\container\\container.obj");
container.set_pos(glm::vec3(-8.0, 0.0, 0.0));
main_scene.push_generic_model(container);

// Bunny
Model bunny = rmanager.load_model(L"assets\\Standford\\bunny\\stanford-bunny.obj");

// Transparent Grass
// Model grass_model = rmanager.create_model({ Mesh(presets::g_plane_data, MaterialMap({ { L"assets\\grass.png", TextureType::DIFFUSE, true } })) });
// grass_model.set_pos(glm::vec3(-4.0, 0.0, 0.0));

// Transparent Window
// Model window_model = rmanager.create_model({ Mesh(presets::g_plane_data, MaterialMap({ { L"assets\\blending_transparent_window.png", TextureType::DIFFUSE, false } })) });
// std::vector<Model> windows(2, window_model);
// windows[0].set_pos(glm::vec3(4.0, 0.0, 0.0));
// windows[1].set_pos(glm::vec3(4.0, 0.0, -2.0));

// Gura
// Model gura = rmanager.load_model(L"assets\\DinoGura\\DinoGura.obj");
// gura.set_size(4);

// Dragon
// Model dragon = rmanager.load_model(L"assets\\xyz_dragon\\xyzrgb_dragon.obj");
// dragon.set_size(1.f / 8.f);
// dragon.set_pos({ 16.f, 0.f, 0.f });
// dragon.rotate(90.f, Axis::Y);

// Sphere
// Model sphere1 = rmanager.load_model(L"assets\\MIT\\basic-shapes\\sphere\\sphere.obj");
// sphere1.set_size(1.f / 3.f);
// Model sphere2 = sphere1;
// Model sphere3 = sphere1;
// Model sphere4 = sphere1;
// main_scene.push_generic_model(sphere1);
// main_scene.push_generic_model(sphere2);
// main_scene.push_generic_model(sphere3);
// main_scene.push_generic_model(sphere4); 
*/
