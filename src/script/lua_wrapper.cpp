#include "lua_wrapper.hpp"

#include <utils/filepath.hpp>

#define SOL_ALL_SAFETIES_ON 1
#define SOL_PRINT_ERRORS 0
#include "sol/sol.hpp"

#include <tracy/Tracy.hpp>

namespace xen {
void LuaWrapper::register_types()
{
    [[maybe_unused]] static bool const _ = []() {
        ZoneScopedN("LuaWrapper::register_types");

        Log::debug("[LuaWrapper] Registering types...");

        register_animation_types();
#if defined(XEN_USE_AUDIO)
        register_audio_types();
#endif
        register_core_types();
        register_data_types();
        register_entity_types();
        register_file_format_types();
        register_image_types();
        register_math_types();
        register_matrix_types();
        register_mesh_types();
        register_mesh_renderer_types();
#if !defined(XEN_NO_OVERLAY)
        register_overlay_base_types();
        register_overlay_widget_types();
#endif
        register_physics_types();
        register_render_types();
        register_render_graph_types();
        register_render_system_types();
        register_shader_types();
        register_shader_program_types();
        register_shape_types();
        register_texture_types();
        register_utils_types();
        register_vector_types();
#if !defined(XEN_NO_WINDOW)
        register_window_types();
#endif
#if !defined(__APPLE__) && !defined(__EMSCRIPTEN__) && !defined(XEN_NO_WINDOW)
        register_xr_types();
#endif

        Log::debug("[LuaWrapper] Registered types");

        return true;
    }();
}

bool LuaWrapper::execute(std::string const& code)
{
    ZoneScopedN("LuaWrapper::execute");

    if (code.empty()) {
        return false;
    }

    Log::debug("[LuaWrapper] Executing code...");

    try {
        get_state().script(code);
    }
    catch (sol::error const& err) {
        Log::verror("[LuaWrapper] Error executing code: '{}'.", err.what());
        return false;
    }

    Log::debug("[LuaWrapper] Executed code");

    return true;
}

bool LuaWrapper::execute_from_file(FilePath const& filepath)
{
    ZoneScopedN("LuaWrapper::execute_from_file");
    ZoneTextF("Path: %s", filepath.to_utf8().c_str());

    if (filepath.empty()) {
        return false;
    }

    Log::debug("[LuaWrapper] Executing code from file ('" + filepath + "')...");

    try {
        get_state().script_file(filepath.to_utf8());
    }
    catch (sol::error const& err) {
        Log::verror("[LuaWrapper] Error executing code from file: '{}.", err.what());
        return false;
    }

    Log::debug("[LuaWrapper] Executed code from file");

    return true;
}

void LuaWrapper::collect_garbage()
{
    get_state().collect_garbage();
}

sol::state& LuaWrapper::get_state()
{
    static sol::state state = []() {
        Log::debug("[LuaWrapper] Initializing state...");

        sol::state lua_state;
        lua_state.open_libraries(sol::lib::base, sol::lib::math, sol::lib::string);

        Log::debug("[LuaWrapper] Initialized state");

        return lua_state;
    }();

    return state;
}
}