#include "physics.hpp"
#include "collision_object.hpp"
#include "rigidbody.hpp"
#include "kinematic_character.hpp"
#include "colliders/collider.hpp"
#include "application.hpp"

// #include "engine/engine.hpp"
// #include "scene/components/fps_player.hpp"
// #include "scene/scenes.hpp"

#include <BulletCollision/BroadphaseCollision/btBroadphaseInterface.h>
#include <BulletCollision/BroadphaseCollision/btDbvtBroadphase.h>
#include <BulletCollision/CollisionDispatch/btCollisionDispatcher.h>
#include <BulletCollision/CollisionDispatch/btCollisionObject.h>
#include <BulletCollision/CollisionShapes/btCollisionShape.h>
#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.h>
#include <BulletSoftBody/btSoftBodyRigidBodyCollisionConfiguration.h>
#include <BulletSoftBody/btSoftRigidDynamicsWorld.h>

#include <algorithm>
#include <iterator>

namespace xen {
PhysicsSystem::PhysicsSystem() :
    collision_configuration(std::make_unique<btSoftBodyRigidBodyCollisionConfiguration>()),
    broadphase(std::make_unique<btDbvtBroadphase>()),
    dispatcher(std::make_unique<btCollisionDispatcher>(collision_configuration.get())),
    solver(std::make_unique<btSequentialImpulseConstraintSolver>()),
    dynamics_world(std::make_unique<btSoftRigidDynamicsWorld>(
        dispatcher.get(), broadphase.get(), solver.get(), collision_configuration.get()
    ))
{
    dynamics_world->setGravity(Collider::convert(gravity));
    dynamics_world->getDispatchInfo().m_enableSPU = true;
    dynamics_world->getSolverInfo().m_minimumSolverBatchSize = 128;
    dynamics_world->getSolverInfo().m_globalCfm = 0.00001f;

    auto* softDynamicsWorld = dynamic_cast<btSoftRigidDynamicsWorld*>(dynamics_world.get());
    softDynamicsWorld->getWorldInfo().water_density = 0.0f;
    softDynamicsWorld->getWorldInfo().water_offset = 0.0f;
    softDynamicsWorld->getWorldInfo().water_normal = btVector3(0.0f, 0.0f, 0.0f);
    softDynamicsWorld->getWorldInfo().m_gravity.setValue(0.0f, -9.81f, 0.0f);
    softDynamicsWorld->getWorldInfo().air_density = air_density;
    softDynamicsWorld->getWorldInfo().m_sparsesdf.Initialize();

    register_components<Transform, Rigidbody, KinematicCharacter>();
}

PhysicsSystem::~PhysicsSystem()
{
    for (int32_t i = dynamics_world->getNumCollisionObjects() - 1; i >= 0; i--) {
        auto* obj = dynamics_world->getCollisionObjectArray()[i];
        auto* body = btRigidBody::upcast(obj);

        if (body && body->getMotionState()) {
            delete body->getMotionState();
            body->setMotionState(nullptr);
        }

        dynamics_world->removeCollisionObject(obj);
    }
}

void PhysicsSystem::start()
{
    for (Entity* entity : entities) {
        if (entity->has_component<Rigidbody>()) {
            auto& rigidbody = entity->get_component<Rigidbody>();
            rigidbody.start(entity->get_component<Transform>());
        }
        if (entity->has_component<KinematicCharacter>()) {
            auto& kinematic_character = entity->get_component<KinematicCharacter>();
            kinematic_character.start(entity->get_component<Transform>());
        }
    }
}

bool PhysicsSystem::update(FrameTimeInfo const& time_info)
{
    if (paused) {
        return true;
    }

    dynamics_world->stepSimulation(time_info.delta_time);
    for (Entity* entity : entities) {
        if (entity->has_component<Rigidbody>()) {
            auto& rigidbody = entity->get_component<Rigidbody>();
            rigidbody.update(time_info, entity->get_component<Transform>());
        }
        if (entity->has_component<KinematicCharacter>()) {
            auto& kinematic_character = entity->get_component<KinematicCharacter>();
            kinematic_character.update(time_info, entity->get_component<Transform>());
        }
    }
    check_for_collision_events();

    return true;
}

Raycast PhysicsSystem::raytest(Vector3f const& start, Vector3f const& end) const
{
    auto startBt = Collider::convert(start);
    auto endBt = Collider::convert(end);

    btCollisionWorld::ClosestRayResultCallback result(startBt, endBt);
    dynamics_world->getCollisionWorld()->rayTest(startBt, endBt, result);

    return {
        result.hasHit(), Collider::convert(result.m_hitPointWorld),
        result.m_collisionObject ? static_cast<CollisionObject*>(result.m_collisionObject->getUserPointer()) : nullptr
    };
}

void PhysicsSystem::set_gravity(Vector3f const& gravity)
{
    this->gravity = gravity;
    dynamics_world->setGravity(Collider::convert(gravity));
}

void PhysicsSystem::set_air_dencity(float air_density)
{
    this->air_density = air_density;
    auto* soft_dynamics_world = dynamic_cast<btSoftRigidDynamicsWorld*>(dynamics_world.get());
    soft_dynamics_world->getWorldInfo().air_density = air_density;
    soft_dynamics_world->getWorldInfo().m_sparsesdf.Initialize();
}

void PhysicsSystem::check_for_collision_events()
{
    // Keep a list of the collision pairs found during the current update.
    CollisionPairs pairs_this_update;

    // Iterate through all of the manifolds in the dispatcher.
    for (int32_t i = 0; i < dispatcher->getNumManifolds(); ++i) {
        // Get the manifold.
        auto* manifold = dispatcher->getManifoldByIndexInternal(i);

        // Ignore manifolds that have no contact points..
        if (manifold->getNumContacts() == 0) {
            continue;
        }

        // Get the two rigid bodies involved in the collision.
        auto const* body0 = manifold->getBody0();
        auto const* body1 = manifold->getBody1();

        // Always create the pair in a predictable order (use the pointer value..).
        auto const swapped = body0 > body1;
        auto const* const sorted_body_a = swapped ? body1 : body0;
        auto const* const sorted_body_b = swapped ? body0 : body1;

        // Create the pair.
        auto thisPair = std::make_pair(sorted_body_a, sorted_body_b);

        // Insert the pair into the current list.
        pairs_this_update.insert(thisPair);

        // If this pair doesn't exist in the list from the previous update, it is a new pair and we must send a
        // collision event.
        if (pairs_this_update.find(thisPair) == pairs_this_update.end()) {
            // Gets the user pointer (entity).
            auto* collision_object_a = static_cast<CollisionObject*>(sorted_body_a->getUserPointer());
            auto* collision_object_b = static_cast<CollisionObject*>(sorted_body_b->getUserPointer());

            // collision_object_a->on_collision(collision_object_b);
        }
    }

    // Creates another list for pairs that were removed this update.
    CollisionPairs removed_pairs;

    // This handy function gets the difference between two sets. It takes the difference between collision pairs from
    // the last update, and this update and pushes them into the removed pairs list.
    std::ranges::set_difference(
        pairs_last_update, pairs_this_update, std::inserter(removed_pairs, removed_pairs.begin())
    );

    // Iterate through all of the removed pairs sending separation events for them.
    for (auto const& [removedObject0, removedObject1] : removed_pairs) {
        // Gets the user pointer (entity).
        auto* collision_object_a = static_cast<CollisionObject*>(removedObject0->getUserPointer());
        auto* collision_object_b = static_cast<CollisionObject*>(removedObject1->getUserPointer());

        // collision_object_a->on_separation(collision_object_b);
    }

    // In the next iteration we'll want to compare against the pairs we found in this iteration.
    pairs_last_update = pairs_this_update;
}
}