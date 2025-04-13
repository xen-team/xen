#include "skybox.hpp"

#include <render/shader.hpp>
#include <render/texture/cubemap.hpp>
#include <render/renderer/gl_cache.hpp>
#include <render/render.hpp>
#include <scene/components/camera.hpp>
#include <utils/shader_loader.hpp>
#include <resource/resources.hpp>

namespace xen {
Skybox::Skybox(std::span<std::string_view> filepaths) :
    skybox_shader(ShaderLoader::load_shader("skybox.glsl")), gl_cache{&GLCache::get()}
{

    CubemapSettings srgb_cubemap;
    srgb_cubemap.is_srgb = true;
    skybox_cubemap = Resources::get()->load_cubemap_texture_async(
        filepaths[0], filepaths[1], filepaths[2], filepaths[3], filepaths[4], filepaths[5], &srgb_cubemap
    );
}

void Skybox::draw(CameraComponent& camera)
{
    // If light intensity is set to 0.0, then no need to even render the skybox since it won't be seen
    if (light_intensity <= 0.0f) {
        return;
    }

    gl_cache->set_shader(*skybox_shader);

    // Pass the texture to the shader
    skybox_cubemap->bind(0);
    skybox_shader->set_uniform("skyboxCubemap", 0);

    // Setup uniforms
    skybox_shader->set_uniform("view", camera.get_view());
    skybox_shader->set_uniform("projection", camera.get_projection());
    skybox_shader->set_uniform("tintColour", tint_color);
    skybox_shader->set_uniform("lightIntensity", light_intensity);

    // Since the vertex shader is gonna make the depth value 1.0, and the default value in the depth buffer is 1.0 so
    // this is needed to draw the sky  box
    gl_cache->set_depth_test(true);
    gl_cache->set_face_cull(true);
    gl_cache->set_cull_face(GL_FRONT);
    gl_cache->set_depth_func(GL_LEQUAL);

    Render::get()->get_renderer()->draw_ndc_cube();

    gl_cache->set_cull_face(GL_BACK);
    gl_cache->set_depth_func(GL_LESS);

    skybox_cubemap->unbind();
}
}