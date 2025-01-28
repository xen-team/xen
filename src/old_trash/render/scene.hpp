#pragma once

#include "frustum.hpp"
#include "entity.hpp"
#include "framebuffer.hpp"

#include <vector>
#include <memory>

namespace render {
    class Scene {
        FrameBuffer framebuffer;

        std::vector<Entity> entities;
    public:
        void draw(Frustum const& cam_frustum);
        void update();

        void add_entity(Entity&& entity) {
            entities.emplace_back(std::move(entity));
        }

        FrameBuffer& get_framebuffer() { return framebuffer; };
    };
}