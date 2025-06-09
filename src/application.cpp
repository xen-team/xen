#include "application.hpp"

#include <tracy/Tracy.hpp>

#if defined(XEN_IS_PLATFORM_EMSCRIPTEN)
#include <emscripten.h>
#endif

namespace xen {
Application::Application(size_t world_count)
{
    worlds.reserve(world_count);
}

void Application::run()
{
    Log::debug("[Application] Running...");

#if defined(XEN_IS_PLATFORM_EMSCRIPTEN)
    emscripten_set_main_loop_arg([](void* instance) { static_cast<decltype(this)>(instance)->run_once(); }, this, 0, 1);
#else
    while (run_once())
        ;
#endif

    Log::debug("[Application] Exiting...");
}

bool Application::run_once()
{
    ZoneScopedN("Application::run_once");

    auto const current_time = std::chrono::system_clock::now();
    time_info.delta_time = std::chrono::duration<float>(current_time - last_frame_time).count();
    time_info.global_time += time_info.delta_time;
    last_frame_time = current_time;

    time_info.substep_count = 0;
    remaining_time += time_info.delta_time;

    while (remaining_time >= time_info.substep_time) {
        ++time_info.substep_count;
        remaining_time -= time_info.substep_time;
    }

    for (size_t world_index = 0; world_index < worlds.size(); ++world_index) {
        if (!active_worlds[world_index]) {
            continue;
        }
        if (!worlds[world_index]->update(time_info)) {
            active_worlds.set_bit(world_index, false);
        }
    }

    // Adding a frame mark registers the past frame
    // TODO: the application setup (everything up until Application::run() is called, hence including the main function)
    // is merged with the very first frame
    //  A "fix" would be to add another FrameMark at the top of the run function, but the currently templated callback
    //  overload being in a header, including Tracy to be used in there wouldn't be a good idea
    // FrameMark;

    return (running && !active_worlds.empty());
}
}