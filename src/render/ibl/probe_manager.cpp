#include "probe_manager.hpp"

#include <render/shader.hpp>
#include <render/ibl/light_probe.hpp>
#include <render/ibl/reflection_probe.hpp>
#include <render/texture/cubemap.hpp>

namespace xen {
ProbeManager::ProbeManager(ProbeBlendSetting settings) : settings(settings) {}

void ProbeManager::add_probe(std::unique_ptr<LightProbe>&& probe)
{
    light_probes.emplace_back(std::move(probe));
}

void ProbeManager::add_probe(std::unique_ptr<ReflectionProbe>&& probe)
{
    reflection_probes.emplace_back(std::move(probe));
}

void ProbeManager::bind_probes(Vector3f const& position, Shader& shader)
{
    // If simple blending is enabled just use the closest probe
    if (settings == ProbeBlendSetting::PROBES_SIMPLE) {
        if (!light_probes.empty()) {
            uint closest_index = 0;

            for (uint i = 1; i < light_probes.size(); i++) {
                if ((light_probes[i]->get_position() - position).length_squared() <
                    (light_probes[closest_index]->get_position() - position).length_squared()) {
                    closest_index = i;
                }
            }

            light_probes[closest_index]->bind(shader);
        }
        else {
            light_probe_fallback->bind(shader);
        }

        if (!reflection_probes.empty()) {
            uint closest_index = 0;

            for (uint i = 1; i < reflection_probes.size(); i++) {
                if ((reflection_probes[i]->get_position() - position).length_squared() <
                    (reflection_probes[closest_index]->get_position() - position).length_squared()) {
                    closest_index = i;
                }
            }
            
            reflection_probes[closest_index]->bind(shader);
        }
        else {
            reflection_probe_fallback->bind(shader);
        }
    }

    // If probes are disabled just use the skybox
    else if (settings == ProbeBlendSetting::PROBES_DISABLED) {
        light_probe_fallback->bind(shader);
        reflection_probe_fallback->bind(shader);
    }
}

void ProbeManager::set_light_probe_fallback(std::unique_ptr<LightProbe>&& fallback)
{
    light_probe_fallback = std::move(fallback);
}

void ProbeManager::set_reflection_probe_fallback(std::unique_ptr<ReflectionProbe>&& fallback)
{
    reflection_probe_fallback = std::move(fallback);
}
}