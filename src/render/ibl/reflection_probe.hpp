#pragma once

#include <render/texture/texture.hpp>

namespace xen {
class Shader;
class Cubemap;

class ReflectionProbe {
private:
    static std::unique_ptr<Texture> brdf_lut;
    std::unique_ptr<Cubemap> prefilter_map;

    Vector3f position;
    Vector2f resolution;
    bool generated = false;

public:
    ReflectionProbe(Vector3f const& position, Vector2f const& resolution);

    void generate();

    // Assumes the shader is bound
    void bind(Shader& shader);

    [[nodiscard]] Vector3f get_position() const { return position; }
    [[nodiscard]] Cubemap* get_prefilter_map() const { return prefilter_map.get(); }
    [[nodiscard]] static Texture* get_brdf_lut() { return brdf_lut.get(); }

    static void set_brdf_lut(std::unique_ptr<Texture>&& texture) { brdf_lut = std::move(texture); }
};
}