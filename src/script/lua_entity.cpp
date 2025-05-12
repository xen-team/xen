#include <entity.hpp>
#if defined(XEN_USE_AUDIO)
#include <audio/listener.hpp>
#include <audio/sound.hpp>
#endif
#include <data/mesh.hpp>
#include <math/transform/transform.hpp>
#include <physics/collider.hpp>
#include <physics/rigid_body.hpp>
#include <render/camera.hpp>
#include <render/light.hpp>
#include <render/mesh_renderer.hpp>
#include <script/lua_wrapper.hpp>
#include <utils/type_utils.hpp>

#define SOL_ALL_SAFETIES_ON 1
#include "sol/sol.hpp"

namespace xen {

using namespace TypeUtils;

template <typename... Args>
auto bindComponents()
{
    return sol::overload([](Entity& entity, Args& comp) -> Args& {
        return entity.add_component<Args>(std::move(comp));
    }...);
}

void LuaWrapper::register_entity_types()
{
    sol::state& state = get_state();

    {
        sol::usertype<Entity> entity =
            state.new_usertype<Entity>("Entity", sol::constructors<Entity(size_t), Entity(size_t, bool)>());
        entity["get_id"] = &Entity::get_id;
        entity["is_enabled"] = &Entity::is_enabled;
        entity["get_enabled_components"] = &Entity::get_enabled_components;
        entity["enable"] = sol::overload([](Entity& e) { e.enable(); }, PickOverload<bool>(&Entity::enable));
        entity["disable"] = &Entity::disable;
        entity["add_component"] = bindComponents<
            Camera, Collider, Light,
#if defined(XEN_USE_AUDIO)
            Listener,
#endif
            Mesh, MeshRenderer, RigidBody,
#if defined(XEN_USE_AUDIO)
            Sound,
#endif
            Transform>();
        entity["has_samera"] = &Entity::has_component<Camera>;
        entity["ha_collider"] = &Entity::has_component<Collider>;
        entity["has_light"] = &Entity::has_component<Light>;
#if defined(XEN_USE_AUDIO)
        entity["has_listener"] = &Entity::has_component<Listener>;
#endif
        entity["has_mesh"] = &Entity::has_component<Mesh>;
        entity["has_mesh_renderer"] = &Entity::has_component<MeshRenderer>;
        entity["has_rigid_body"] = &Entity::has_component<RigidBody>;
#if defined(XEN_USE_AUDIO)
        entity["has_sound"] = &Entity::has_component<Sound>;
#endif
        entity["has_transform"] = &Entity::has_component<Transform>;
        entity["get_camera"] = [](Entity& e) { return &e.get_component<Camera>(); };
        entity["get_collider"] = [](Entity& e) { return &e.get_component<Collider>(); };
        entity["get_light"] = [](Entity& e) { return &e.get_component<Light>(); };
#if defined(XEN_USE_AUDIO)
        entity["get_listener"] = [](Entity& e) { return &e.get_component<Listener>(); };
#endif
        entity["get_mesh"] = [](Entity& e) { return &e.get_component<Mesh>(); };
        entity["get_mesh_renderer"] = [](Entity& e) { return &e.get_component<MeshRenderer>(); };
        entity["get_rigid_body"] = [](Entity& e) { return &e.get_component<RigidBody>(); };
#if defined(XEN_USE_AUDIO)
        entity["get_sound"] = [](Entity& e) { return &e.get_component<Sound>(); };
#endif
        entity["get_transform"] = [](Entity& e) { return &e.get_component<Transform>(); };
    }
}
}