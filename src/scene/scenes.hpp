#pragma once

#include "scene.hpp"
#include <engine/module.hpp>

#include "render/render.hpp"

namespace xen {
class XEN_API Scenes : public Module::Registrar<Scenes> {
    inline static bool const registered = Register("Scenes", Stage::PreRender, Depends<Render>());

private:
    std::unique_ptr<Scene> current_scene{nullptr};

public:
    void update() override;

    [[nodiscard]] Scene* get_scene() const { return current_scene.get(); }

    Scene* set_scene(std::unique_ptr<Scene>&& scene);
};
}