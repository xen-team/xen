#include "script_system.hpp"

#include <script/lua_script.hpp>

#include <tracy/Tracy.hpp>

namespace xen {
ScriptSystem::ScriptSystem()
{
    register_components<LuaScript>();
}

bool ScriptSystem::update(FrameTimeInfo const& time_info)
{
    if (paused) {
        return true;
    }

    ZoneScopedN("ScriptSystem::update");

    bool res = true;

    for (Entity const* entity : entities) {
        res = entity->get_component<LuaScript>().update(time_info) && res;
    }

    return res;
}

void ScriptSystem::link_entity(EntityPtr const& entity)
{
    System::link_entity(entity);

    auto& lua_script = entity->get_component<LuaScript>();
    lua_script.register_entity(*entity, "this");
    lua_script.setup();
}
}