#pragma once

namespace xen {
class Shader;
class LightProbe;
class ReflectionProbe;

enum class ProbeBlendSetting : uint8_t {
    PROBES_DISABLED, // Ignores probes and uses the skybox
    PROBES_SIMPLE,   // Uses the closest probe (no blending)
    PROBES_BLEND     // Blends adjacent probes (TODO: Make this work)
};

class ProbeManager {
private:
    ProbeBlendSetting settings;

    std::vector<std::unique_ptr<LightProbe>> light_probes;
    std::vector<std::unique_ptr<ReflectionProbe>> reflection_probes;

    std::unique_ptr<LightProbe> light_probe_fallback;
    std::unique_ptr<ReflectionProbe> reflection_probe_fallback;

public:
    ProbeManager(ProbeBlendSetting settings);

    void add_probe(std::unique_ptr<LightProbe>&& probe);

    void add_probe(std::unique_ptr<ReflectionProbe>&& probe);

    void set_light_probe_fallback(std::unique_ptr<LightProbe>&& fallback);
    void set_reflection_probe_fallback(std::unique_ptr<ReflectionProbe>&& fallback);

    // Assumes shader is bound
    void bind_probes(Vector3f const& position, Shader& shader);
};
}