#pragma once

#include "units/vector2.hpp"

struct GLFWmonitor;
struct GLFWvidmode;
struct GLFWgammaramp;

namespace xen {
class Monitor {
private:
	GLFWmonitor* monitor;

	friend class Window;
public:
	explicit Monitor(GLFWmonitor* monitor = nullptr);

	GLFWmonitor* get_monitor() const { return monitor; }

	bool is_primary() const;

	Vector2ui get_workarea_size() const;

	Vector2ui get_workarea_pos() const;

	Vector2ui get_size() const;

	Vector2f get_content_scale() const;

	Vector2ui get_pos() const;

	std::string get_name() const;

	std::vector<GLFWvidmode> get_video_modes() const;

	GLFWvidmode get_video_mode() const;

	GLFWgammaramp get_gamma_ramp() const;

	void set_gamma_ramp(GLFWgammaramp const& gammaRamp) const;
};
}