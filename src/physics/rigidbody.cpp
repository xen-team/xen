#include "rigidbody.hpp"
#include "physics.hpp"
#include "colliders/collider.hpp"

#include <scene/entity.hpp>
#include <scene/scenes.hpp>
#include <engine/engine.hpp>
#include <BulletCollision/CollisionShapes/btCollisionShape.h>
#include <BulletCollision/CollisionShapes/btCompoundShape.h>
#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>
#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <LinearMath/btDefaultMotionState.h>
#include <entt.hpp>

namespace xen {
Rigidbody::Rigidbody(
    std::unique_ptr<Collider>&& collider, float mass, float friction, Vector3f const& linear_factor,
    Vector3f const& angular_factor
) : CollisionObject({}, mass, friction, linear_factor, angular_factor)
{
    add_collider(std::move(collider));
}

Rigidbody::~Rigidbody()
{
    if (auto* body = btRigidBody::upcast(this->body); body && body->getMotionState()) {
        delete body->getMotionState();
    }

    // if (auto physics = Scenes::get()->get_scene()->GetSystem<Physics>())
    // physics->GetDynamicsWorld()->removeRigidBody(rigidBody.get());
}

void Rigidbody::start()
{
    auto* scene = Scenes::get()->get_scene();
    auto* physics = scene->get_system<Physics>();

    if (rigid_body) {
        physics->get_dynamics_world()->removeRigidBody(rigid_body.get());
    }

    create_shape();
    assert((!shape || shape->getShapeType() != INVALID_SHAPE_PROXYTYPE) && "Invalid rigidbody shape!");
    gravity = physics->get_gravity();
    btVector3 local_inertia;

    // Rigidbody is dynamic if and only if mass is non zero, otherwise static.
    if (mass != 0.0f) {
        shape->calculateLocalInertia(mass, local_inertia);
    }

    auto const& transform = scene->get_entity_by_component(this).get_component<TransformComponent>();

    auto const world_transform = Collider::convert(transform);

    // Using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects.
    auto* motion_state = new btDefaultMotionState(world_transform);
    btRigidBody::btRigidBodyConstructionInfo c_info(mass, motion_state, shape.get(), local_inertia);

    rigid_body = std::make_unique<btRigidBody>(c_info);
    // rigid_body->setContactProcessingThreshold(m_defaultContactProcessingThreshold);
    rigid_body->setWorldTransform(world_transform);
    // rigid_body->setContactStiffnessAndDamping(1000.0f, 0.1f);
    rigid_body->setFriction(friction);
    rigid_body->setRollingFriction(friction_rolling);
    rigid_body->setSpinningFriction(friction_spinning);
    rigid_body->setGravity(Collider::convert(gravity));
    rigid_body->setLinearFactor(Collider::convert(linear_factor));
    rigid_body->setAngularFactor(Collider::convert(angular_factor));
    rigid_body->setUserPointer(dynamic_cast<CollisionObject*>(this));
    body = rigid_body.get();
    physics->get_dynamics_world()->addRigidBody(rigid_body.get());
    rigid_body->activate(true);
    recalculate_mass();
}

void Rigidbody::update()
{
    if (shape.get() != body->getCollisionShape()) {
        body->setCollisionShape(shape.get());
    }

    auto const delta = Engine::get()->get_delta();

    for (auto it = forces.begin(); it != forces.end();) {
        (*it)->update(delta);
        rigid_body->applyForce(Collider::convert((*it)->get_force()), Collider::convert((*it)->get_position()));

        if ((*it)->is_expired()) {
            it = forces.erase(it);
            continue;
        }

        ++it;
    }

    auto& transform = Scenes::get()->get_scene()->get_entity_by_component(this).get_component<TransformComponent>();

    btTransform motion_transform;
    rigid_body->getMotionState()->getWorldTransform(motion_transform);
    transform = Collider::convert(motion_transform, transform.scale);

    shape->setLocalScaling(Collider::convert(transform.scale));
    // rigidBody->getMotionState()->setWorldTransform(Collider::convert(transform));
    linear_velocity = Collider::convert(rigid_body->getLinearVelocity());
    angular_velocity = Collider::convert(rigid_body->getAngularVelocity());
}

bool Rigidbody::in_frustum(Frustum const& frustum)
{
    btVector3 min;
    btVector3 max;

    if (body && shape) {
        rigid_body->getAabb(min, max);
    }

    return frustum.cube_in(Collider::convert(min), Collider::convert(max));
}

void Rigidbody::clear_forces()
{
    if (rigid_body) {
        rigid_body->clearForces();
    }
}

void Rigidbody::set_mass(float mass)
{
    this->mass = mass;
    recalculate_mass();
}

void Rigidbody::set_gravity(Vector3f const& gravity)
{
    this->gravity = gravity;

    if (rigid_body) {
        rigid_body->setGravity(Collider::convert(gravity));
    }
}

void Rigidbody::set_linear_factor(Vector3f const& linear_factor)
{
    this->linear_factor = linear_factor;

    if (rigid_body) {
        rigid_body->setLinearFactor(Collider::convert(linear_factor));
    }
}

void Rigidbody::set_angular_factor(Vector3f const& angular_factor)
{
    this->angular_factor = angular_factor;

    if (rigid_body) {
        rigid_body->setAngularFactor(Collider::convert(angular_factor));
    }
}

void Rigidbody::set_linear_velocity(Vector3f const& linear_velocity)
{
    this->linear_velocity = linear_velocity;

    if (rigid_body) {
        rigid_body->setLinearVelocity(Collider::convert(linear_velocity));
    }
}

void Rigidbody::set_angular_velocity(Vector3f const& angular_velocity)
{
    this->angular_velocity = angular_velocity;

    if (rigid_body) {
        rigid_body->setAngularVelocity(Collider::convert(angular_velocity));
    }
}

void Rigidbody::recalculate_mass()
{
    if (!rigid_body) {
        return;
    }

    auto const is_dynamic = mass != 0.0f;

    btVector3 local_inertia;

    if (!colliders.empty() && is_dynamic) {
        colliders[0]->get_collision_shape()->calculateLocalInertia(mass, local_inertia);
    }

    rigid_body->setMassProps(mass, local_inertia);
}
}