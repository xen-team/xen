#include "windows.hpp"

#include "debug/log.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <algorithm>

namespace xen {
void callback_error(int32_t error, char const* description)
{
    Windows::check_glfw(error);
    Log::error("GLFW error: ", description, ", ", error, '\n');
}

void callback_monitor(GLFWmonitor* glfw_monitor, int32_t event)
{
    auto& monitors = Windows::get()->monitors;

    switch (event) {
    case GLFW_CONNECTED: {
        Monitor* monitor = monitors.emplace_back(std::make_unique<Monitor>(glfw_monitor)).get();
        Windows::get()->on_monitor_connect(monitor, true);

        break;
    }
    case GLFW_DISCONNECTED: {
        auto remove_if_monitor = std::remove_if(monitors.begin(), monitors.end(), [glfw_monitor](auto const& monitor) {
            bool is_matching = (monitor->get_monitor() == glfw_monitor);
            if (is_matching) {
                Windows::get()->on_monitor_connect(monitor.get(), false);
            }
            return is_matching;
        });

        monitors.erase(remove_if_monitor, monitors.end());

        break;
    }
    default:
        break;
    }
}

Windows::Windows()
{
    if (glfwInit() == GLFW_FALSE) {
        throw std::runtime_error("GLFW failed to initialize");
    }

    // Set the error error callback
    glfwSetErrorCallback(callback_error);

    // Set the monitor callback
    glfwSetMonitorCallback(callback_monitor);

    // The window will stay hidden until after creation.
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    // Fixes 16 bit stencil bits in macOS.
    glfwWindowHint(GLFW_STENCIL_BITS, 8);
    // No stereo view!
    glfwWindowHint(GLFW_STEREO, GLFW_FALSE);

    // Get connected monitors.
    int32_t monitors_count = 0;
    GLFWmonitor** monitors = glfwGetMonitors(&monitors_count);

    for (int32_t i = 0; i < monitors_count; i++) {
        this->monitors.emplace_back(std::make_unique<Monitor>(monitors[i]));
    }
}

Windows::~Windows()
{
    // idk why RAII is working bad there so we need to manually clear
    windows.clear();
    monitors.clear();

    glfwTerminate();
}

void Windows::update()
{
    glfwPollEvents();
    for (auto& window : windows) {
        window->update();
    }
}

Window* Windows::add_window()
{
    Window* window = windows.emplace_back(std::make_unique<Window>(windows.size())).get();

    static bool is_glad_initialized = false;
    if (!is_glad_initialized && gladLoadGLLoader((GLADloadproc)glfwGetProcAddress) == GLFW_FALSE) {
        throw std::runtime_error("Failed to initialize GLAD");
    }
    is_glad_initialized = true;

    on_add_window(window, true);
    return window;
}

Window const* Windows::get_window(WindowId id) const
{
    if (id >= windows.size()) {
        return nullptr;
    }
    return windows[id].get();
}

Window* Windows::get_window(WindowId id)
{
    if (id >= windows.size()) {
        return nullptr;
    }
    return windows[id].get();
}

void Windows::set_focused_window(WindowId id)
{
    focused_window_id = id;
}

Window const* Windows::get_focused_window() const
{
    return get_window(focused_window_id);
}
Window* Windows::get_focused_window()
{
    return get_window(focused_window_id);
}

Monitor const* Windows::get_primary_monitor() const
{
    for (auto const& monitor : monitors) {
        if (monitor->is_primary()) {
            return monitor.get();
        }
    }
    return nullptr;
}

std::string Windows::glfw_result_as_str(int32_t result)
{
    switch (result) {
    case GLFW_TRUE:
        return "Success";
    case GLFW_NOT_INITIALIZED:
        return "GLFW has not been initialized";
    case GLFW_NO_CURRENT_CONTEXT:
        return "No context is current for this thread";
    case GLFW_INVALID_ENUM:
        return "One of the arguments to the function was an invalid enum value";
    case GLFW_INVALID_VALUE:
        return "One of the arguments to the function was an invalid value";
    case GLFW_OUT_OF_MEMORY:
        return "A memory allocation failed";
    case GLFW_API_UNAVAILABLE:
        return "GLFW could not find support for the requested API on the system";
    case GLFW_VERSION_UNAVAILABLE:
        return "The requested OpenGL or OpenGL ES version is not available";
    case GLFW_PLATFORM_ERROR:
        return "A platform-specific error occurred that does not match any of the "
               "more specific categories";
    case GLFW_FORMAT_UNAVAILABLE:
        return "The requested format is not supported or available";
    case GLFW_NO_WINDOW_CONTEXT:
        return "The specified window does not have an OpenGL or OpenGL ES context";
    default:
        return "ERROR: UNKNOWN GLFW ERROR";
    }
}

void Windows::check_glfw(int32_t result)
{
    if (result != 0) {
        return;
    }

    std::string const failure = glfw_result_as_str(result);
    Log::error("GLFW error: ", failure, ", ", result, '\n');
    throw std::runtime_error("GLFW error: " + failure);
}
} // namespace xen