#pragma once

#include "core.hpp"
#include "units/vector2.hpp"

struct GLFWmonitor;
struct GLFWvidmode;
struct GLFWgammaramp;

namespace xen {
class XEN_API Monitor {
private:
    GLFWmonitor* monitor;

    friend class Window;

public:
    explicit Monitor(GLFWmonitor* monitor = nullptr);

    [[nodiscard]] GLFWmonitor* get_monitor() const { return monitor; }

    [[nodiscard]] bool is_primary() const;

    [[nodiscard]] Vector2ui get_workarea_size() const;

    [[nodiscard]] Vector2ui get_workarea_pos() const;

    [[nodiscard]] Vector2ui get_size() const;

    [[nodiscard]] Vector2f get_content_scale() const;

    [[nodiscard]] Vector2ui get_pos() const;

    [[nodiscard]] std::string get_name() const;

    [[nodiscard]] std::vector<GLFWvidmode> get_video_modes() const;

    [[nodiscard]] GLFWvidmode get_video_mode() const;

    [[nodiscard]] GLFWgammaramp get_gamma_ramp() const;

    void set_gamma_ramp(GLFWgammaramp const& gamma_ramp) const;
};
}