#pragma once

#include "window.hpp"
#include "engine/module.hpp"

#include <memory>

namespace xen {
class Windows : public Module::Registrar<Windows> {
	inline static bool const Registered = Register(Stage::Pre);

private:
	std::vector<std::unique_ptr<Window>> windows;
	std::vector<std::unique_ptr<Monitor>> monitors;

public:
	rocket::signal<void(Window*, bool)> on_add_window;
	rocket::signal<void(Monitor*, bool)> on_monitor_connect;

public:
	Windows();
	~Windows();

	void update();

	Window* add_window();
	Window const* get_window(window_id id) const;
	Window* get_window(window_id id);

	std::vector<std::unique_ptr<Monitor>> const& get_monitors() const { return monitors; };

	Monitor const* get_primary_monitor() const;

	static std::string glfw_result_as_str(int32_t result);
	static void check_glfw(int32_t result);

private:
	friend void callback_error(int32_t error, char const* description);
	friend void callback_monitor(GLFWmonitor* glfwMonitor, int32_t event);
};
}