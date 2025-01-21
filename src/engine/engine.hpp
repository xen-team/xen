#pragma once

#include "units/time.hpp"
#include "units/elapsed_time.hpp"
#include "utils/classes.hpp"
#include "module.hpp"
#include "app.hpp"

#include <cmath>

namespace xen {
class Delta {
public:
	void update() {
		current_frame = Time::now();
		change = current_frame - last_frame;
		last_frame = current_frame;
	}

	Time current_frame;
	Time last_frame;
	Time change;
};

class ChangePerSecond {
public:
	void update(const Time &time) {
		tmp_value++;

		if (std::floor(time.as_seconds()) > std::floor(time_value.as_seconds())) {
			value = tmp_value;
			tmp_value = 0;
		}

		time_value = time;
	}

	uint32_t tmp_value = 0, value = 0;
	Time time_value;
};

struct Version {
	uint8_t major, minor, patch;
};

class Engine : NonCopyable {
private:
	static Engine *instance;

	Version version;

	std::unique_ptr<App> app;

	std::map<std::type_index, std::unique_ptr<Module>> modules;
	std::map<Module::Stage, std::vector<std::type_index>> module_stages;

	float fps_limit;
	bool running;

	Delta delta_update, delta_render;
	ElapsedTime elapsed_update, elapsed_render;
	ChangePerSecond ups, fps;

public:
	static Engine* get() { return instance; }

	Engine();
	~Engine();

	int32_t run();

	const Version& get_version() const { return version; }

	App* get_app() const { return app.get(); }

	void set_app(std::unique_ptr<App>&& app) { this->app = std::move(app); }

	float get_fps_limit() const { return fps_limit; }

	void set_fps_limit(float fps_limit);

	bool is_running() const { return running; }

	const Time& get_delta() const { return delta_update.change; }

	const Time& get_delta_render() const { return delta_render.change; }

	uint32_t get_ups() const { return ups.value; }

	uint32_t get_fps() const { return fps.value; }

	void request_close() { running = false; }

private:
	void create_module(Module::RegistryMapT::const_iterator it);
	void destroy_module(std::type_index id);
	void update_stage(Module::Stage stage);

	void process_updates();
	void process_rendering();
};
}