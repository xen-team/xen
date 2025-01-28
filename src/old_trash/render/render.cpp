#include "render.hpp"
#include "model.hpp"
#include "frustum.hpp"

render::RenderManager::RenderManager() {
    default_shader = std::make_unique<Shader>("default.vs", "default.fs");

    ubo = std::make_unique<UniformBufferObject>("View", 2 * sizeof(glm::mat4));
    ubo->bind_shader(default_shader->ID);
    ubo->init();

    static Model model((std::filesystem::current_path() / "resources/planet.obj").generic_string());
	
    Entity ourEntity(model, *default_shader);
	ourEntity.transform.setLocalPosition({ 0, 0, 0 });
	const float scale = 1.0;
	ourEntity.transform.setLocalScale({ scale, scale, scale });

	{
		Entity* lastEntity = &ourEntity;

		for (unsigned int x = 0; x < 20; ++x)
		{
			for (unsigned int z = 0; z < 20; ++z)
			{
				ourEntity.add_child(model, *default_shader);
				lastEntity = ourEntity.children.back().get();

				//Set transform values
				lastEntity->transform.setLocalPosition({ x * 10.f - 100.f,  0.f, z * 10.f - 100.f });
			}
		}
	}
	ourEntity.update_tree();
    
    default_scene.add_entity(std::move(ourEntity));

    // debug_camera.pos = {0.f, 120.f, -10.f};
    // debug_camera.direction = {0.f, -1.f, 0.f};
    // debug_camera.far = 1000.f;
    // debug_camera.euler[0] = -90.f;
    // debug_camera.fov = 90.f;
    // debug_camera.update();
}

void render::RenderManager::draw(Scene& scene, PerspectiveCamera& camera) {
    auto const& framebuffer = scene.get_framebuffer();
    framebuffer.bind();

    auto const& fb_size = framebuffer.get_size();
    resize_viewport(0, 0, fb_size);

    clear_frame(br_color_normal{0.2f, 0.3f, 0.3f, 1.0f}, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 projection = camera.GetProjection();
    ubo->update_data(glm::value_ptr(projection), sizeof(glm::mat4));

    glm::mat4 view = camera.GetView();
    ubo->update_data(glm::value_ptr(view), sizeof(glm::mat4), sizeof(glm::mat4));

    Frustum const cam_frustum = createFrustumFromCamera(camera);

    scene.draw(cam_frustum);
    scene.update();

    framebuffer.unbind();
}

void render::RenderManager::draw_debug() {
    draw(default_scene, debug_camera);
}