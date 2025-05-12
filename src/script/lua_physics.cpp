#include <physics/collider.hpp>
#include <physics/physics_system.hpp>
#include <physics/rigid_body.hpp>
#include <script/lua_wrapper.hpp>
#include <utils/shape.hpp>
#include <utils/type_utils.hpp>

#define SOL_ALL_SAFETIES_ON 1
#include "sol/sol.hpp"

namespace xen {

using namespace TypeUtils;

void LuaWrapper::register_physics_types()
{
    sol::state& state = get_state();

    {
        sol::usertype<Collider> collider = state.new_usertype<Collider>(
            "Collider", sol::constructors<Collider()>(), sol::base_classes, sol::bases<Component>()
        );
        collider["get_shape_type"] = &Collider::get_shape_type;
        collider["has_shape"] = &Collider::has_shape;
        collider["get_shape"] = [](Collider& c) { return &c.get_shape(); };
        collider["set_shape"] = [](Collider& c, Shape& s) { c.set_shape(std::move(s)); };
        collider["intersects"] = sol::overload(
            PickOverload<Collider const&>(&Collider::intersects), PickOverload<Shape const&>(&Collider::intersects),
            [](Collider& c, Ray const& r) { return c.intersects(r); },
            PickOverload<Ray const&, RayHit*>(&Collider::intersects)
        );
    }

    {
        sol::usertype<PhysicsSystem> physicsSystem = state.new_usertype<PhysicsSystem>(
            "PhysicsSystem", sol::constructors<PhysicsSystem()>(), sol::base_classes, sol::bases<System>()
        );
        physicsSystem["gravity"] = sol::property(&PhysicsSystem::get_gravity, &PhysicsSystem::set_gravity);
        physicsSystem["friction"] = sol::property(&PhysicsSystem::get_friction, &PhysicsSystem::set_friction);
    }

    {
        sol::usertype<RigidBody> rigid_body = state.new_usertype<RigidBody>(
            "RigidBody", sol::constructors<RigidBody(float, float)>(), sol::base_classes, sol::bases<Component>()
        );
        rigid_body["mass"] = sol::property(&RigidBody::get_mass, &RigidBody::set_mass);
        rigid_body["get_inv_mass"] = &RigidBody::get_inv_mass;
        rigid_body["bounciness"] = sol::property(&RigidBody::get_bounciness, &RigidBody::set_bounciness);
        rigid_body["velocity"] = sol::property(&RigidBody::get_velocity, &RigidBody::set_velocity);
        rigid_body["forces"] = sol::property(&RigidBody::get_forces, &RigidBody::set_forces<Vector3f>);
    }
}

}
