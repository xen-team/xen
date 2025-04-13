#pragma once

#include <entt.hpp>

namespace xen {
class EntityManager {
    friend class Entity;
    friend class Scene;

private:
    inline static EntityManager* first_manager = nullptr;
    // std::unordered_map<UUID, Entity> entities;
    entt::registry registry;

public:
    EntityManager()
    {
        if (!first_manager) {
            first_manager = this;
        }
    }

    entt::registry& get_registry() { return registry; }
};
}