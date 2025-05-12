#include <entity.hpp>
#include <script/lua_environment.hpp>
#include <script/lua_wrapper.hpp>
#include <utils/filepath.hpp>

#define SOL_ALL_SAFETIES_ON 1
#define SOL_PRINT_ERRORS 0
#include "sol/sol.hpp"

#include <tracy/Tracy.hpp>

namespace xen {
LuaEnvironment::LuaEnvironment() :
    environment{
        std::make_unique<sol::environment>(LuaWrapper::get_state(), sol::create, LuaWrapper::get_state().globals())
    }
{
}

bool LuaEnvironment::execute(std::string const& code) const
{
    ZoneScopedN("LuaEnvironment::execute");

    if (code.empty()) {
        return false;
    }

    Log::debug("[LuaEnvironment] Executing code...");

    try {
        LuaWrapper::get_state().script(code, *environment);
    }
    catch (sol::error const& err) {
        Log::verror("[LuaEnvironment] Error executing code: '{}'.", err.what());
        return false;
    }

    Log::debug("[LuaEnvironment] Executed code");

    return true;
}

bool LuaEnvironment::execute_from_file(FilePath const& filepath) const
{
    ZoneScopedN("LuaEnvironment::execute_from_file");
    ZoneTextF("Path: %s", filepath.to_utf8().c_str());

    if (filepath.empty()) {
        return false;
    }

    Log::debug("[LuaEnvironment] Executing code from file ('" + filepath + "')...");

    try {
        LuaWrapper::get_state().script_file(filepath.to_utf8(), *environment);
    }
    catch (sol::error const& err) {
        Log::error("[LuaEnvironment] Error executing code from file: '{}'.", err.what());
        return false;
    }

    Log::debug("[LuaEnvironment] Executed code from file");

    return true;
}

bool LuaEnvironment::exists(char const* name) const
{
    return get(name).valid();
}

void LuaEnvironment::clear()
{
    Log::debug("[LuaEnvironment] Clearing environment...");
    environment->clear();
    Log::debug("[LuaEnvironment] Cleared environment");
}

LuaEnvironment::~LuaEnvironment() = default;

void LuaEnvironment::register_entity(Entity const& entity, std::string const& name)
{
    Log::debug("[LuaEnvironment] Registering entity (ID: {}) as '{}'...", entity.get_id(), name);
    environment->operator[](name) = &entity;
    Log::debug("[LuaEnvironment] Registered entity");
}

sol::reference LuaEnvironment::get(char const* name) const
{
    return environment->operator[](name);
}
}