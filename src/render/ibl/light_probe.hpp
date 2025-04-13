#pragma once

namespace xen {
class Shader;
class Cubemap;

class LightProbe {
private:
    std::unique_ptr<Cubemap> irradiance_map;

    Vector3f position;
    Vector2f resolution;
    bool generated = false;

public:
    LightProbe(Vector3f const& position, Vector2f const& resolution);
    ~LightProbe() = default;

    void generate();

    // Assumes the shader is bound
    void bind(Shader& shader);

    [[nodiscard]] Vector3f get_position() const { return position; }
    [[nodiscard]] Cubemap* get_irradiance_map() const { return irradiance_map.get(); }
};
}