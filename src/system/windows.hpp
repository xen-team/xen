#pragma once

#include "core.hpp"
#include "engine/module.hpp"
#include "window.hpp"

#include <memory>

namespace xen {
class XEN_API Windows : public Module::Registrar<Windows> {
    inline static bool const registered = Register("Windows", Stage::Pre);
                                    
private:
    WindowId focused_window_id = 0;
    std::vector<std::unique_ptr<Window>> windows;
    std::vector<std::unique_ptr<Monitor>> monitors;

public:
    rocket::signal<void(Window*, bool)> on_add_window;
    rocket::signal<void(Monitor*, bool)> on_monitor_connect;

public:
    Windows();
    ~Windows() override;

    void update() override;

    Window* add_window();
    [[nodiscard]] Window const* get_window(WindowId id) const;
    [[nodiscard]] Window* get_window(WindowId id);
    [[nodiscard]] Window* get_main_window();

    void set_focused_window(WindowId id);
    [[nodiscard]] Window const* get_focused_window() const;
    [[nodiscard]] Window* get_focused_window();

    [[nodiscard]] bool is_empty() const { return windows.empty(); }

    [[nodiscard]] std::vector<std::unique_ptr<Monitor>> const& get_monitors() const { return monitors; };

    [[nodiscard]] Monitor const* get_primary_monitor() const;

    [[nodiscard]] static std::string glfw_result_as_str(int32_t result);
    static void check_glfw(int32_t result);

private:
    friend void callback_error(int32_t error, char const* description);
    friend void callback_monitor(GLFWmonitor* glfw_monitor, int32_t event);
};
}