#pragma once

#include "data/mesh.hpp"
#include "physics/collider.hpp"
#include "world.hpp"
#include <system.hpp>

namespace xen {
class PhysicsSystem final : public System {
public:
    PhysicsSystem();

    constexpr Vector3f const& get_gravity() const { return gravity; }

    constexpr float get_friction() const { return friction; }

    void set_gravity(Vector3f const& gravity) { this->gravity = gravity; }

    void set_friction(float friction)
    {
        Log::rt_assert(friction >= 0.f && friction <= 1.f, "Error: Friction coefficient must be between 0 & 1.");
        this->friction = friction;
    }

    bool update(FrameTimeInfo const& time_info) override;

    void generateCollisionFromMesh(Entity& entity_with_mesh)
    {
        if (!entity_with_mesh.has_component<Mesh>()) {
            Log::warning("Entity does not have a Mesh component. Cannot generate collision.");
            return;
        }

        auto& mesh = entity_with_mesh.get_component<Mesh>();

        std::vector<xen::Submesh> const& submeshes = mesh.get_submeshes();
        if (submeshes.empty()) {
            Log::warning("Mesh component has no submeshes. Cannot generate collision.");
            return;
        }

        if (!entity_with_mesh.has_component<Collider>()) {
            entity_with_mesh.add_component<Collider>();
        }

        auto& collider = entity_with_mesh.get_component<Collider>();

        for (auto const& submesh : submeshes) {

            xen::AABB const& submeshAABB = submesh.get_bounding_box();

            // Проверяем, является ли AABB валидным
            if (submeshAABB.get_min_position() == submeshAABB.get_max_position()) {
                continue;
            }

            // TODO: ОЧЕНЬ ВАЖНО! ТРАНСФОРМАЦИЯ!
            // Если AABB подмеши вычислен в ЛОКАЛЬНОМ пространстве подмеши,
            // а сама сущность (targetEntity) имеет трансформацию (позицию, вращение, масштаб),
            // то AABB, который мы добавляем в Collider, должен быть в МИРОВОМ пространстве.
            // Либо нужно пересчитать AABB в мировое пространство здесь,
            // либо использовать OBB и применить к нему мировое вращение сущности.
            // Для статической карты, самый простой вариант: убедиться, что AABB подмешей
            // вычисляется в мировом пространстве во время загрузки, применяя трансформации узлов модели.
            // Если AABB локальный, а сущность трансформирована:
            // Vector3f worldMin = targetEntity.getTransform().apply(submeshAABB.get_min_position()); // Пример, зависит
            // от твоей реализации трансформаций Vector3f worldMax =
            // targetEntity.getTransform().apply(submeshAABB.get_max_position()); // AABB не масштабируется и не
            // вращается в мировом пространстве простым применением трансформации! Для AABB в мировом пространстве нужно
            // найти новые мин/макс точки после трансформации 8 углов локального AABB. xen::AABB worldSpaceAABB =
            // computeWorldSpaceAABB(submeshAABB, targetEntity.getTransform()); // Нужна такая функция

            // Для простоты примера, предположим, что submeshAABB уже в мировом пространстве
            // Или что AABB вычисляется относительно 0,0,0 и сущность карты всегда в 0,0,0, а геометрия смещена.

            // 2. Создаем экземпляр Shape (AABB)
            // Создаем unique_ptr для передачи владения
            std::unique_ptr<Shape> aabbShape =
                std::make_unique<xen::AABB>(submeshAABB.get_min_position(), submeshAABB.get_max_position());

            // 3. Добавляем Shape в компонент Collider этой же сущности
            collider->add_shape(std::move(aabbShape));

            Log::info("Added AABB shape to entity's collider for a submesh.");
        }

        // Теперь targetEntity имеет один Collider компонент,
        // который содержит список AABB форм для всех подмешей.
    }

private:
    Vector3f gravity = Vector3f(0.f, -9.80665f, 0.f); ///< Gravity acceleration.
    float friction = 0.95f;

private:
    void solve_constraints();
};
}