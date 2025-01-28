#include "monitor.hpp"

#include <GLFW/glfw3.h>

namespace xen {
Monitor::Monitor(GLFWmonitor* monitor) : monitor(monitor) {}

bool Monitor::is_primary() const
{
    return monitor == glfwGetPrimaryMonitor();
}

Vector2ui Monitor::get_workarea_size() const
{
    int32_t width = 0;
    int32_t height = 0;
    glfwGetMonitorWorkarea(monitor, nullptr, nullptr, &width, &height);
    return {width, height};
}

Vector2ui Monitor::get_workarea_pos() const
{
    int32_t x = 0;
    int32_t y = 0;
    glfwGetMonitorWorkarea(monitor, &x, &y, nullptr, nullptr);
    return {x, y};
}

Vector2ui Monitor::get_size() const
{
    int32_t width_mm = 0;
    int32_t height_mm = 0;
    glfwGetMonitorPhysicalSize(monitor, &width_mm, &height_mm);
    return Vector2ui(width_mm, height_mm);
}

Vector2f Monitor::get_content_scale() const
{
    float x = 0;
    float y = 0;
    glfwGetMonitorContentScale(monitor, &x, &y);
    return {x, y};
}

Vector2ui Monitor::get_pos() const
{
    int32_t x = 0;
    int32_t y = 0;
    glfwGetMonitorPos(monitor, &x, &y);
    return {x, y};
}

std::string Monitor::get_name() const
{
    return glfwGetMonitorName(monitor);
}

std::vector<GLFWvidmode> Monitor::get_video_modes() const
{
    int32_t video_modes_count = 0;
    GLFWvidmode const* video_modes = glfwGetVideoModes(monitor, &video_modes_count);
    std::vector<GLFWvidmode> modes(video_modes_count);
    std::copy(video_modes, video_modes + video_modes_count, modes.begin());
    return modes;
}

GLFWvidmode Monitor::get_video_mode() const
{
    return *glfwGetVideoMode(monitor);
}

GLFWgammaramp Monitor::get_gamma_ramp() const
{
    return *glfwGetGammaRamp(monitor);
}

void Monitor::set_gamma_ramp(GLFWgammaramp const& gamma_ramp) const
{
    glfwSetGammaRamp(monitor, &gamma_ramp);
}
}