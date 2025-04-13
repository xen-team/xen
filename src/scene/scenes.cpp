#include "scenes.hpp"

#include <render/ibl/light_probe.hpp>
#include <render/ibl/reflection_probe.hpp>
#include <render/skybox.hpp>
#include <render/terrain/terrain.hpp>

namespace xen {
void Scenes::update()
{
    if (!current_scene) {
        return;
    }

    if (!current_scene->started) {
        current_scene->start();
        current_scene->started = true;
    }

    current_scene->update();
}

Scene* Scenes::set_scene(std::unique_ptr<Scene>&& scene)
{
    current_scene = std::move(scene);
    return current_scene.get();
}
}