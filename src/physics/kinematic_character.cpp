#include "kinematic_character.hpp"
#include "physics.hpp"
#include <scene/entity.hpp>
#include <scene/scenes.hpp>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include <BulletCollision/CollisionShapes/btCompoundShape.h>
#include <BulletSoftBody/btSoftRigidDynamicsWorld.h>
#include <BulletDynamics/Character/btKinematicCharacterController.h>

namespace xen {
KinematicCharacter::KinematicCharacter(std::unique_ptr<Collider>&& collider, float mass, float friction) :
    CollisionObject({}, mass, friction), up(Vector3f::up), step_height(0.0f), fall_speed(55.0f), jump_speed(10.0f),
    max_height(1.5f), interpolate(true)
{
    add_collider(std::move(collider));
}

KinematicCharacter::~KinematicCharacter()
{
    if (auto* physics = Scenes::get()->get_scene()->get_system<Physics>()) {
        // TODO: Are these being deleted?
        physics->get_dynamics_world()->removeCollisionObject(ghost_object.get());
        physics->get_dynamics_world()->removeAction(controller.get());
    }
}

void KinematicCharacter::start()
{
    std::cerr << "lalalalaaaaaaaaa 1\n";

    auto* physics = Scenes::get()->get_scene()->get_system<Physics>();

    std::cerr << "lalalalaaaaaaaaa 2\n";

    if (ghost_object) {
        physics->get_dynamics_world()->removeCollisionObject(ghost_object.get());
    }

    if (controller) {
        physics->get_dynamics_world()->removeAction(controller.get());
    }

    std::cerr << "lalalalaaaaaaaaa 3\n";

    create_shape(true);

    std::cerr << "lalalalaaaaaaaaa 228\n";

    assert((shape || shape->getShapeType() != INVALID_SHAPE_PROXYTYPE) && "Invalid ghost object shape!");

    std::cerr << "lalalalaaaaaaaaa 1228\n";

    gravity = physics->get_gravity();

    std::cerr << "lalalalaaaaaaaaa 2228\n";

    btVector3 localInertia;

    std::cerr << "lalalalaaaaaaaaa 4\n";

    // Rigidbody is dynamic if and only if mass is non zero, otherwise static.
    if (mass != 0.0f) {
        shape->calculateLocalInertia(mass, localInertia);
    }

    std::cerr << "lalalalaaaaaaaaa 5\n";

    auto const& transform =
        Scenes::get()->get_scene()->get_entity_by_component(this).get_component<TransformComponent>();
    auto const world_transform = Collider::convert(transform);

    std::cerr << "lalalalaaaaaaaaa 6\n";

    ghost_object = std::make_unique<btPairCachingGhostObject>();
    ghost_object->setWorldTransform(world_transform);
    physics->get_broadphase()->getOverlappingPairCache()->setInternalGhostPairCallback(new btGhostPairCallback());
    ghost_object->setCollisionShape(shape.get());
    ghost_object->setCollisionFlags(btCollisionObject::CF_CHARACTER_OBJECT);
    ghost_object->setFriction(friction);
    ghost_object->setRollingFriction(friction_rolling);
    ghost_object->setSpinningFriction(friction_spinning);
    ghost_object->setUserPointer(dynamic_cast<CollisionObject*>(this));
    physics->get_dynamics_world()->addCollisionObject(
        ghost_object.get(), btBroadphaseProxy::CharacterFilter, btBroadphaseProxy::AllFilter
    );
    body = ghost_object.get();

    controller = std::make_unique<btKinematicCharacterController>(
        ghost_object.get(), dynamic_cast<btConvexShape*>(shape.get()), 0.03f
    );
    controller->setGravity(Collider::convert(gravity));
    controller->setUp(Collider::convert(up));
    controller->setStepHeight(step_height);
    controller->setFallSpeed(fall_speed);
    controller->setJumpSpeed(jump_speed);
    controller->setMaxJumpHeight(max_height);
    controller->setUpInterpolate(interpolate);
    physics->get_dynamics_world()->addAction(controller.get());
}

void KinematicCharacter::update()
{
    if (shape.get() != body->getCollisionShape()) {
        body->setCollisionShape(shape.get());
    }
    auto& transform = Scenes::get()->get_scene()->get_entity_by_component(this).get_component<TransformComponent>();
    auto const world_transform = ghost_object->getWorldTransform();
    transform = Collider::convert(world_transform, transform.scale);

    linear_velocity = Collider::convert(controller->getLinearVelocity());
    angular_velocity = Collider::convert(controller->getAngularVelocity());
}

bool KinematicCharacter::in_frustum(Frustum const& frustum)
{
    btVector3 min;
    btVector3 max;

    if (body && shape) {
        auto const& transform =
            Scenes::get()->get_scene()->get_entity_by_component(this).get_component<TransformComponent>();
        shape->getAabb(Collider::convert(transform), min, max);
    }

    return frustum.cube_in(Collider::convert(min), Collider::convert(max));
}

void KinematicCharacter::set_mass(float mass)
{
    this->mass = mass;
    recalculate_mass();
}

void KinematicCharacter::set_gravity(Vector3f const& gravity)
{
    this->gravity = gravity;
    controller->setGravity(Collider::convert(gravity));
}

void KinematicCharacter::set_linear_factor(Vector3f const& linear_factor)
{
    this->linear_factor = linear_factor;
    // controller->setLinearFactor(Collider::Convert(linearFactor)); // TODO
}

void KinematicCharacter::set_angular_factor(Vector3f const& angular_factor)
{
    this->angular_factor = angular_factor;
    // controller->setAngularFactor(Collider::Convert(angularFactor)); // TODO
}

void KinematicCharacter::set_linear_velocity(Vector3f const& linear_velocity)
{
    this->linear_velocity = linear_velocity;
    controller->setLinearVelocity(Collider::convert(linear_velocity));
}

void KinematicCharacter::set_angular_velocity(Vector3f const& angular_velocity)
{
    this->angular_velocity = angular_velocity;
    controller->setAngularVelocity(Collider::convert(angular_velocity));
}

void KinematicCharacter::set_up(Vector3f const& up)
{
    this->up = up;
    controller->setUp(Collider::convert(up));
}

void KinematicCharacter::set_step_height(float step_height)
{
    this->step_height = step_height;
    controller->setStepHeight(step_height);
}

void KinematicCharacter::set_fall_speed(float fall_speed)
{
    this->fall_speed = fall_speed;
    controller->setFallSpeed(fall_speed);
}

void KinematicCharacter::set_jump_speed(float jump_speed)
{
    this->jump_speed = jump_speed;
    controller->setJumpSpeed(jump_speed);
}

void KinematicCharacter::set_max_jump_height(float max_height)
{
    this->max_height = max_height;
    controller->setMaxJumpHeight(max_height);
}

void KinematicCharacter::set_interpolate(bool interpolate)
{
    this->interpolate = interpolate;
    controller->setUpInterpolate(interpolate);
}

bool KinematicCharacter::is_on_ground() const
{
    return controller->onGround();
}

void KinematicCharacter::jump(Vector3f const& direction)
{
    controller->jump(Collider::convert(direction));
}

void KinematicCharacter::set_walk_direction(Vector3f const& direction)
{
    controller->setWalkDirection(Collider::convert(direction));
}

void KinematicCharacter::recalculate_mass()
{
    // TODO
}
}