#pragma once

namespace xen {
class Shader;
class Cubemap;
class GLCache;
class CameraComponent;

class Skybox {
private:
    std::shared_ptr<Shader> skybox_shader;
    GLCache* gl_cache;

    Cubemap* skybox_cubemap;

    // Settings to tune skybox look
    Vector3f tint_color = Vector3f(1.f);
    float light_intensity = 1.f;

public:
    Skybox(std::span<std::string_view> filepaths);

    void draw(CameraComponent& camera);

    Cubemap* get_skybox_cubemap() { return skybox_cubemap; }

    [[nodiscard]] Vector3f get_tint_color() const { return tint_color; }
    [[nodiscard]] float get_light_intensity() const { return light_intensity; }
};
}