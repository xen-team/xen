#include <application.hpp>
#include <component.hpp>
#include <system.hpp>
#include <world.hpp>
#include <audio/audio_system.hpp>
#include <data/bvh_system.hpp>
// #include <physics/physics_system.hpp>
#include <render/render_system.hpp>
#include <script/lua_wrapper.hpp>
#include <utils/trigger_system.hpp>
#include <utils/type_utils.hpp>
#include <xr/xr_system.hpp>

#define SOL_ALL_SAFETIES_ON 1
#include "sol/sol.hpp"

namespace xen {
using namespace TypeUtils;

void LuaWrapper::register_core_types()
{
    sol::state& state = get_state();

    {
        sol::usertype<FrameTimeInfo> frameTimeInfo =
            state.new_usertype<FrameTimeInfo>("FrameTimeInfo", sol::constructors<FrameTimeInfo()>());
        frameTimeInfo["delta_time"] = &FrameTimeInfo::delta_time;
        frameTimeInfo["global_time"] = &FrameTimeInfo::global_time;
        frameTimeInfo["substep_count"] = &FrameTimeInfo::substep_count;
        frameTimeInfo["substep_time"] = &FrameTimeInfo::substep_time;

        sol::usertype<Application> application =
            state.new_usertype<Application>("Application", sol::constructors<Application(), Application(size_t)>());
        application["get_worlds"] = PickNonConstOverload<>(&Application::get_worlds);
        application["get_time_info"] = &Application::get_time_info;
        application["set_fixed_time_step"] = &Application::set_fixed_time_step;
        application["add_world"] = &Application::add_world<>;
        application["run"] = sol::overload(
            [](Application& app) { app.run(); },
            [](Application& app, std::function<void(FrameTimeInfo const&)> const& func) { app.run(func); }
        );
        application["run_once"] = &Application::run_once;
        application["quit"] = &Application::quit;
    }

    {
        state.new_usertype<Component>("Component", sol::no_constructor);
    }

    {
        sol::usertype<System> system = state.new_usertype<System>("System", sol::no_constructor);
        system["get_accepted_components"] = &System::get_accepted_components;
        system["contains_entity"] = &System::contains_entity;
        system["update"] = &System::update;
        system["destroy"] = &System::destroy;
    }

    {
        sol::usertype<World> world = state.new_usertype<World>("World", sol::constructors<World(), World(size_t)>());
#if defined(XEN_USE_AUDIO)
        world["add_audio_system"] =
            sol::overload(&World::add_system<AudioSystem>, &World::add_system<AudioSystem, const char*>);
#endif
        world["add_bvh_system"] = &World::add_system<BoundingVolumeHierarchySystem>;
        // world["add_physics_system"] = &World::add_system<PhysicsSystemSystem>;
        world["add_render_system"] = sol::overload(
            &World::add_system<RenderSystem>, &World::add_system<RenderSystem, Vector2ui const&>
#if !defined(XEN_NO_WINDOW)
            ,
            &World::add_system<RenderSystem, Vector2ui const&, std::string const&>,
            &World::add_system<RenderSystem, Vector2ui const&, std::string const&, WindowSetting>,
            &World::add_system<RenderSystem, Vector2ui const&, std::string const&, WindowSetting, uint8_t>
#endif
        );
        world["add_trigger_system"] = &World::add_system<TriggerSystem>;
#if defined(XEN_USE_XR)
        world["add_xr_system"] = &World::add_system<XrSystem, const std::string&>;
#endif
        world["add_entity"] =
            sol::overload([](World& w) { return &w.add_entity(); }, PickOverload<bool>(&World::add_entity));
        world["remove_entity"] = &World::remove_entity;
        world["update"] = &World::update;
        world["refresh"] = &World::refresh;
        world["destroy"] = &World::destroy;
        world["get_player"] = &World::get_player;
    }
}
}