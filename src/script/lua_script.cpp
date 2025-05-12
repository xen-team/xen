#include "lua_script.hpp"

#include <application.hpp>
#include <script/lua_wrapper.hpp>
#include <utils/filepath.hpp>
#include <utils/file_utils.hpp>

#define SOL_SAFE_GETTER 0 // Allowing implicit conversion to bool
#include "sol/sol.hpp"

#include <tracy/Tracy.hpp>

namespace xen {
LuaScript::LuaScript(std::string const& code)
{
    Log::debug("[LuaScript] Creating script...");

    xen::LuaWrapper::register_types();
    load_code(code);

    Log::debug("[LuaScript] Created script");
}

void LuaScript::load_code(std::string const& code)
{
    ZoneScopedN("LuaScript::load_code");

    Log::debug("[LuaScript] Loading code...");

    sol::object const owning_entity = environment.get("this");

    environment.clear();

    if (!environment.execute(code)) {
        throw std::invalid_argument("Error: The given Lua script is invalid");
    }

    if (environment.get("update").get_type() != sol::type::function) {
        throw std::invalid_argument("Error: A Lua script must have an update() function");
    }

    if (owning_entity.is<Entity>()) {
        environment.register_entity(owning_entity.as<Entity>(), "this");
        setup();
    }

    Log::debug("[LuaScript] Loaded code");
}

void LuaScript::load_code_from_file(FilePath const& filepath)
{
    ZoneScopedN("LuaScript::load_code_from_file");
    ZoneTextF("Path: %s", filepath.to_utf8().c_str());

    Log::debug("[LuaScript] Loading code from file ('" + filepath + "')...");
    load_code(FileUtils::read_file_to_string(filepath));
    Log::debug("[LuaScript] Loaded code from file");
}

bool LuaScript::update(FrameTimeInfo const& time_info) const
{
    ZoneScopedN("LuaScript::update");

    sol::unsafe_function const update_func = environment.get("update");
    sol::unsafe_function_result const update_result = update_func(time_info);
    return (update_result.get_type() == sol::type::none || update_result);
}

void LuaScript::setup() const
{
    ZoneScopedN("LuaScript::setup");

    if (!environment.exists("setup")) {
        return;
    }

    sol::reference const setup_ref = environment.get("setup");

    if (setup_ref.get_type() != sol::type::function) {
        return;
    }

    Log::debug("[LuaScript] Running script setup...");

    sol::unsafe_function const setup_func = setup_ref;

    if (!setup_func().valid()) {
        throw std::runtime_error("Error: The Lua script failed to be setup");
    }

    Log::debug("[LuaScript] Ran script setup");
}
}