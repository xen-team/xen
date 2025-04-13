#pragma once

#include "core.hpp"
#include <scene/entity.hpp>
#include <scene/debug_camera.hpp>
#include <render/lights/light_system.hpp>
#include <render/ibl/probe_manager.hpp>
#include <render/ibl/reflection_probe.hpp>
#include <render/ibl/light_probe.hpp>
#include <render/water/water_system.hpp>
#include <scene/system_manager.hpp>
#include <render/skybox.hpp>
#include <render/terrain/terrain.hpp>

#include <rocket.hpp>
#include <entt.hpp>
#include <utility>

namespace xen {
class Entity;
// class Terrain;
// class Skybox;

enum class ModelFilterType : uint8_t {
    AllModels,
    StaticModels,
    OpaqueModels,
    OpaqueStaticModels,
    TransparentModels,
    TransparentStaticModels
};

class XEN_API Scene : public virtual rocket::trackable {
    friend class Scenes;
    friend class Entity;

private:
    SystemManager systems;

    EntityManager entities;
    Entity root = entt::null;

    std::shared_ptr<CameraComponent> current_camera = nullptr;
    DebugCamera debug_camera;

    ProbeBlendSetting scene_probe_blend_settings = ProbeBlendSetting::PROBES_SIMPLE;
    ProbeManager probe_manager;

    std::unique_ptr<Terrain> terrain;
    std::unique_ptr<Skybox> skybox;

    bool started = false;
    bool paused = false;

public:
    rocket::signal<void()> on_init;
    rocket::signal<void()> on_update;
    rocket::signal<void()> on_cleanup;

    rocket::signal<void(Entity)> on_entity_add;
    rocket::signal<void(Entity)> on_entity_remove;

public:
    Scene();
    virtual ~Scene() = default;

    virtual void start();

    virtual void update();

    template <SystemDerived S>
    bool has_system() const
    {
        return systems.has<S>();
    }

    template <SystemDerived S>
    S* get_system() const
    {
        return systems.get<S>();
    }

    template <SystemDerived S, typename... Args>
    void add_system(Args&&... args)
    {
        auto system = std::make_unique<S>(std::forward<Args>(args)...);
        system->scene = this;
        systems.add<S>(std::move(system));
    }

    template <SystemDerived S>
    void remove_system()
    {
        systems.remove<S>();
    }

    void clear_systems();

    [[nodiscard]] CameraComponent& get_camera() const { return *current_camera; };

    void set_camera(std::shared_ptr<CameraComponent> camera) { current_camera = std::move(camera); }

    [[nodiscard]] EntityManager& get_entity_manager() { return entities; };

    [[nodiscard]] Terrain& get_terrain() { return *terrain; };

    [[nodiscard]] Skybox& get_skybox() { return *skybox; };

    [[nodiscard]] LightSystem* get_light_system() { return systems.get<LightSystem>(); }

    [[nodiscard]] WaterSystem* get_water_system() { return systems.get<WaterSystem>(); }

    [[nodiscard]] ProbeManager& get_probe_manager() { return probe_manager; }

    [[nodiscard]] Entity get_root() { return root; };

    [[nodiscard]] Entity get_entity(std::string_view name);

    template <typename Component>
    [[nodiscard]] Entity get_entity_by_component(Component* instance)
    {
        // Profile this
        auto const entity = entt::to_entity(entities.registry.storage<Component>(), *instance);
        return Entity{entity, &entities};
    }

    Entity create_entity();

    Entity create_entity(std::string_view name);

    Entity create_entity_prefab(std::string_view filename);

    void remove_entity(entt::entity entity);

    void remove_entity(Entity entity);

    void remove_entity(std::string_view name);

    void serialize(std::string_view path) const;

    void deserialize(std::string_view path);

    void add_models_to_renderer(ModelFilterType filter);

    [[nodiscard]] bool is_paused() const { return paused; };
};
}