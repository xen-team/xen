#include "physics_system.hpp"

#include <application.hpp>
#include <math/transform/transform.hpp>
#include <physics/collider.hpp>
#include <physics/rigid_body.hpp>
#include <utils/shape.hpp>

#include <tracy/Tracy.hpp>

namespace xen {
PhysicsSystem::PhysicsSystem()
{
    register_components<Collider, RigidBody>();
}

bool PhysicsSystem::update(FrameTimeInfo const& time_info)
{
    ZoneScopedN("PhysicsSystem::update");

    float const relative_friction = std::pow(friction, time_info.substep_time);

    for (int i = 0; i < time_info.substep_count; ++i) {
        for (Entity* entity : entities) {
            if (!entity->is_enabled() || !entity->has_component<RigidBody>()) {
                continue;
            }

            auto& rigid_body = entity->get_component<RigidBody>();

            if (rigid_body.get_mass() <= 0.f) {
                continue;
            }

            Vector3f const acceleration =
                (rigid_body.get_mass() * gravity + rigid_body.get_forces()) * rigid_body.get_inv_mass();
            Vector3f const old_velocity = rigid_body.get_velocity();

            Vector3f const velocity = old_velocity * relative_friction + acceleration * time_info.substep_time;
            rigid_body.set_velocity(velocity);

            auto& transform = entity->get_component<Transform>();

            rigid_body.old_position = transform.get_position();
            transform.translate((old_velocity + velocity) * 0.5f * time_info.substep_time);

            // The following acceleration calculation should be added to the translation to get a more accurate result:
            //    acceleration * delta_time * delta_time * 0.5f
            //  However, the acceleration would be multiplied by a tiny factor, making its effect barely noticeable
            //  for a standard acceleration value. As such, it is left out of the displacement equation
        }

        solve_constraints();
    }

    return true;
}

void PhysicsSystem::solve_constraints()
{
    ZoneScopedN("PhysicsSystem::solve_constraints");

    for (Entity* entity : entities) {
        if (!entity->is_enabled() || !entity->has_component<RigidBody>()) {
            continue;
        }

        auto& rigid_body = entity->get_component<RigidBody>();

        if (rigid_body.get_mass() <= 0.f) {
            continue;
        }

        Vector3f const velocity = rigid_body.get_velocity();
        Vector3f const velocity_dir = (velocity.length_squared() != 0.f ? velocity.normalize() : Vector3f(0.f));

        for (Entity* collidable_entity : entities) {
            if (collidable_entity == entity || !collidable_entity->is_enabled() ||
                !collidable_entity->has_component<Collider>()) {
                continue;
            }

            Log::rt_assert(
                collidable_entity->has_component<Transform>(),
                "Error: A collidable entity must have a Transform component."
            );

            auto& collider = collidable_entity->get_component<Collider>();

            // The collision detection is made in the collider's local space
            // The test shapes/rays must thus be translated into that space
            Vector3f const collider_pos = collidable_entity->get_component<Transform>().get_position();
            Vector3f const local_start_pos = rigid_body.old_position - collider_pos;

            auto& transform = entity->get_component<Transform>();

            // We first try to determine if the last movement gave an intersection
            // This is necessary in case our object has travelled too fast right through the collider,
            //  ending behind it
            Line const movement_line(local_start_pos, transform.get_position() - collider_pos);
            if (!collider.intersects(movement_line)) {
                continue;
            }

            Ray const ray(local_start_pos, velocity_dir);

            RayHit hit;
            if (!collider.intersects(ray, &hit)) {
                continue;
            }

            // Setting the entity's new position a little above the collision point
            Vector3f const new_pos = hit.position + hit.normal * 0.002f + collider_pos;

            rigid_body.old_position = new_pos;
            transform.set_position(new_pos);

            //                                     Vt/para_vec
            //  Vel  N  Refl                  \---->
            //    \  ^  ^                     | \          Vn is the velocity's perpendicular component to the surface
            //     \ | /        ->            |   \        Vt is the velocity's parallel component to the surface
            // _____v|/______      Vn/perp_vec v    v Vel

            Vector3f const para_vec = hit.normal * velocity.dot(hit.normal);
            Vector3f const perp_vec = velocity - para_vec;

            rigid_body.set_velocity(perp_vec - para_vec * rigid_body.get_bounciness());

            break;
        }
    }
}
}