#include "scene.hpp"
#include <physics/physics.hpp>
#include <scene/components/hierarchy.hpp>
#include <scene/components/tag.hpp>
#include <scene/components/id.hpp>
#include <scene/components/transform.hpp>
#include <scene/components/mesh.hpp>
#include <scene/components/pose_animator.hpp>
#include <animation/pose_animator.hpp>
#include <scene/entity.hpp>
#include <render/render.hpp>
#include <render/terrain/terrain.hpp>
#include <render/skybox.hpp>

#include <cereal/cereal.hpp>
#include <cereal/archives/portable_binary.hpp>

namespace xen {
Scene::Scene() :
    root{entt::null}, current_camera{std::make_shared<DebugCamera>(debug_camera)},
    probe_manager{scene_probe_blend_settings}
{
    root = create_entity("Root");
    root.add_component<HierarchyComponent>();
    root.add_component<TransformComponent>();

    terrain = std::make_unique<Terrain>();
    terrain->from_texture("res/terrain/height_map.png");
    terrain->set_position({-256.f, -40.f, -256.f});

    // Skybox init needs to happen before probes are generated
    std::array<std::string_view, 6> skybox_filepaths{"res/skybox/right.png", "res/skybox/left.png",
                                                     "res/skybox/top.png",   "res/skybox/bottom.png",
                                                     "res/skybox/back.png",  "res/skybox/front.png"};

    skybox = std::make_unique<Skybox>(skybox_filepaths);

    add_system<Physics>();
    add_system<LightSystem>();
    add_system<WaterSystem>();
}

void Scene::start()
{
    systems.for_each([]([[maybe_unused]] auto type_id, auto system) {
        if (system->is_enabled()) {
            system->start();
        }
    });
}

void Scene::update()
{
    on_init.invoke();

    systems.for_each([]([[maybe_unused]] auto type_id, auto system) {
        if (system->is_enabled()) {
            system->update();
        }
    });

    // DO components update here
    current_camera->update();

    // std::cerr << current_camera->get_position() << " and " << current_camera->get_rotation() << '\n';

    on_update.invoke();

    on_cleanup.invoke();
}

void Scene::clear_systems()
{
    systems.clear();
}

Entity Scene::get_entity(std::string_view name)
{
    auto view = entities.registry.view<TagComponent>();
    for (auto const entity : view) {
        TagComponent const& tc = view.get<TagComponent>(entity);
        if (tc.tag == name) {
            return {entity, &entities};
        }
    }
    return entt::null;
}

Entity Scene::create_entity()
{
    auto entity = entities.registry.create();

    auto managed_entity = Entity(entity, &entities);

    managed_entity.add_component<IDComponent>();
    managed_entity.add_component<TransformComponent>();

    if (root) {
        auto& root_hierarchy = entities.registry.get<HierarchyComponent>(root);

        if (root_hierarchy.childs == 0) {
            root_hierarchy.first = entity;
        }
        else {
            auto& last_child_hierarchy = entities.registry.get<HierarchyComponent>(root_hierarchy.first);
            while (last_child_hierarchy.next != entt::null) {
                last_child_hierarchy = entities.registry.get<HierarchyComponent>(last_child_hierarchy.next);
            }
            last_child_hierarchy.next = entity;
        }
        auto& entity_hierarchy = managed_entity.add_component<HierarchyComponent>();
        entity_hierarchy.parent = root;

        root_hierarchy.childs++;

        on_entity_add.invoke(managed_entity);
    }

    return managed_entity;
}

Entity Scene::create_entity(std::string_view name)
{
    auto entity = create_entity();
    entity.add_component<TagComponent>(std::string(name));
    return entity;
}

Entity Scene::create_entity_prefab(std::string_view filename) {}

void Scene::remove_entity(entt::entity entity)
{
    auto& entity_hierarchy = entities.registry.get<HierarchyComponent>(entity);

    if (entity_hierarchy.parent != entt::null) {
        auto& parent_hierarchy = entities.registry.get<HierarchyComponent>(entity_hierarchy.parent);

        if (parent_hierarchy.first == entity) {
            parent_hierarchy.first = entity_hierarchy.next;
        }

        if (entity_hierarchy.prev != entt::null) {
            auto& prev_hierarchy = entities.registry.get<HierarchyComponent>(entity_hierarchy.prev);
            prev_hierarchy.next = entity_hierarchy.next;
        }

        if (entity_hierarchy.next != entt::null) {
            auto& next_hierarchy = entities.registry.get<HierarchyComponent>(entity_hierarchy.next);
            next_hierarchy.prev = entity_hierarchy.prev;
        }

        parent_hierarchy.childs--;
    }

    if (entity_hierarchy.childs > 0) {
        auto child = entity_hierarchy.first;
        while (child != entt::null) {
            auto next_child = entities.registry.get<HierarchyComponent>(child).next;
            auto managed_child = Entity(child, &entities);
            remove_entity(managed_child);
            child = next_child;
        }
    }

    auto managed_entity = Entity(entity, &entities);

    on_entity_remove.invoke(managed_entity);

    entities.registry.destroy(entity);
}

void Scene::remove_entity(Entity entity)
{
    remove_entity(static_cast<entt::entity>(entity));
}

void Scene::remove_entity(std::string_view name)
{
    auto entity = get_entity(name);
    remove_entity(entity);
}

void Scene::add_models_to_renderer(ModelFilterType filter)
{
    // auto group = entities.registry.group<TransformComponent, MeshComponent>();
    auto view = entities.registry.view<TransformComponent, MeshComponent>();
    for (auto entity : view) {
        auto& transform = view.get<TransformComponent>(entity);
        auto& mesh = view.get<MeshComponent>(entity);

        // auto [transform, model] = group.get<TransformComponent, MeshComponent>(entity);
        Entity current_entity(entity, &entities);

        PoseAnimator* pose_animator = nullptr;
        if (current_entity.has_component<PoseAnimatorComponent>()) {
            pose_animator = &current_entity.get_component<PoseAnimatorComponent>().pose_animator;
        }

        switch (filter) {
        case ModelFilterType::AllModels:
            Render::get()->get_renderer()->queue_mesh(
                mesh.model, transform.transform(), pose_animator, mesh.is_transparent, mesh.should_backface_cull
            );
            break;
        case ModelFilterType::StaticModels:
            if (mesh.is_static) {
                Render::get()->get_renderer()->queue_mesh(
                    mesh.model, transform.transform(), pose_animator, mesh.is_transparent, mesh.should_backface_cull
                );
            }
            break;
        case ModelFilterType::OpaqueModels:
            if (!mesh.is_transparent) {
                Render::get()->get_renderer()->queue_mesh(
                    mesh.model, transform.transform(), pose_animator, mesh.is_transparent, mesh.should_backface_cull
                );
            }
            break;
        case ModelFilterType::OpaqueStaticModels:
            if (!mesh.is_transparent && mesh.is_static) {
                Render::get()->get_renderer()->queue_mesh(
                    mesh.model, transform.transform(), pose_animator, mesh.is_transparent, mesh.should_backface_cull
                );
            }
            break;
        case ModelFilterType::TransparentModels:
            if (mesh.is_transparent) {
                Render::get()->get_renderer()->queue_mesh(
                    mesh.model, transform.transform(), pose_animator, mesh.is_transparent, mesh.should_backface_cull
                );
            }
            break;
        case ModelFilterType::TransparentStaticModels:
            if (mesh.is_transparent && mesh.is_static) {
                Render::get()->get_renderer()->queue_mesh(
                    mesh.model, transform.transform(), pose_animator, mesh.is_transparent, mesh.should_backface_cull
                );
            }
            break;
        }
    }
}

void Scene::serialize(std::string_view path) const
{
    std::ofstream os(path.data(), std::ios::binary);
    cereal::PortableBinaryOutputArchive archive(os);
    // archive(entities.registry);
}

void Scene::deserialize(std::string_view path)
{
    std::ifstream os(path.data(), std::ios::binary);
    cereal::PortableBinaryInputArchive archive(os);
    // archive(entities.registry);
}
}