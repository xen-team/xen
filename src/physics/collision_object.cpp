#include "collision_object.hpp"

#include "colliders/collider.hpp"

#include <BulletCollision/CollisionShapes/btCompoundShape.h>
#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>

namespace xen {
CollisionObject::CollisionObject(
    std::vector<std::unique_ptr<Collider>>&& colliders, float mass, float friction, Vector3f const& linear_factor,
    Vector3f const& angular_factor
) :
    colliders(std::move(colliders)), mass(mass), friction(friction), friction_rolling(0.1f), friction_spinning(0.2f),
    linear_factor(linear_factor), angular_factor(angular_factor)
{
    // for (auto& collider : colliders) {
    //     collider->on_transform_change.connect([this](Collider* collider, Transform const& local_transform) {
    //         set_child_transform(collider, local_transform);
    //     });
    // }
}

Collider* CollisionObject::add_collider(std::unique_ptr<Collider>&& collider)
{
    if (!collider) {
        return nullptr;
    }

    auto* ret = colliders.emplace_back(std::move(collider)).get();
    // ret->on_transform_change.connect([this](Collider* collider, Transform const& local_transform) {
    //     set_child_transform(collider, local_transform);
    // });
    return ret;
}

void CollisionObject::remove_collider(Collider* collider)
{
    if (!collider) {
        return;
    }

    std::erase_if(colliders, [collider](auto const& c) { return c.get() == collider; });
}

Force* CollisionObject::add_force(std::unique_ptr<Force>&& force)
{
    return forces.emplace_back(std::move(force)).get();
}

// void CollisionObject::SetChildTransform(Collider* child, Transform const& transform)
// {
//     auto compoundShape = dynamic_cast<btCompoundShape*>(shape.get());
//     if (!compoundShape)
//         return;

//     for (int32_t i = 0; i < compoundShape->getNumChildShapes(); i++) {
//         if (compoundShape->getChildShape(i) == child->get_collision_shape()) {
//             compoundShape->updateChildTransform(i, Collider::convert(transform));
//             break;
//         }
//     }

//     recalculate_mass();
// }

// void CollisionObject::AddChild(Collider* child)
// {
//     if (auto compoundShape = dynamic_cast<btCompoundShape*>(shape.get())) {
//         compoundShape->addChildShape(Collider::convert(child->get_local_transform()), child->get_collision_shape());
//         recalculate_mass();
//     }
// }

// void CollisionObject::RemoveChild(Collider* child)
// {
//     if (auto compoundShape = dynamic_cast<btCompoundShape*>(shape.get())) {
//         compoundShape->removeChildShape(child->get_collision_shape());
//         recalculate_mass();
//     }
// }

void CollisionObject::set_ignore_collision_check(CollisionObject* other, bool const ignore)
{
    body->setIgnoreCollisionCheck(other->body, ignore);
}

void CollisionObject::set_friction(float const friction)
{
    this->friction = friction;
    body->setFriction(friction);
}

void CollisionObject::set_friction_rolling(float const friction_rolling)
{
    this->friction_rolling = friction_rolling;
    body->setRollingFriction(friction_rolling);
}

void CollisionObject::set_friction_spinning(float const friction_spinning)
{
    this->friction_spinning = friction_spinning;
    body->setSpinningFriction(friction_spinning);
}

void CollisionObject::create_shape(bool const force_single)
{

    if (force_single) { // && colliders.size() == 1
        shape.reset(colliders[0]->get_collision_shape());
        return;
    }
    if (colliders.empty()) {
        shape = nullptr;
        return;
    }

    if (!dynamic_cast<btCompoundShape*>(shape.get())) {
        shape.reset();
    }

    auto* compound_shape = new btCompoundShape();

    for (int32_t i = 0; i < compound_shape->getNumChildShapes(); i++) {
        compound_shape->removeChildShapeByIndex(i);
    }

    for (auto const& collider : colliders) {
        compound_shape->addChildShape(
            Collider::convert(collider->get_local_transform()), collider->get_collision_shape()
        );
    }

    shape.reset(compound_shape);
    recalculate_mass();
}
}