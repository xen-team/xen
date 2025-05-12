#if defined(XEN_IS_PLATFORM_EMSCRIPTEN)
#include <emscripten.h>
#endif

namespace xen {
template <typename... Args>
World& Application::add_world(Args&&... args)
{
    worlds.emplace_back(std::make_unique<World>(std::forward<Args>(args)...));
    active_worlds.set_bit(worlds.size() - 1);

    return *worlds.back();
}

template <typename FuncT>
void Application::run(FuncT&& callback)
{
#if defined(XEN_IS_PLATFORM_EMSCRIPTEN)
    static auto emscripten_callback = [this, callback = std::forward<FuncT>(callback)]() {
        run_once();
        callback(time_info);
    };

    emscripten_set_main_loop_arg(
        [](void* lambda) { (*static_cast<decltype(&emscripten_callback)>(lambda))(); }, &emscripten_callback, 0, 1
    );
#else
    while (run_once())
        callback(time_info);
#endif
}
}