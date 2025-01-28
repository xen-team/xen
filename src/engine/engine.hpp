#pragma once

#include "app.hpp"
#include "module.hpp"
#include "units/elapsed_time.hpp"
#include "units/time.hpp"
#include "utils/classes.hpp"

#include <cmath>

namespace xen {
class XEN_API Delta {
public:
    void update()
    {
        current_frame = Time::now();
        change = current_frame - last_frame;
        last_frame = current_frame;
    }

    Time current_frame;
    Time last_frame;
    Time change;
};

class XEN_API ChangePerSecond {
public:
    void update(Time const& time)
    {
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

struct XEN_API Version {
    uint8_t major, minor, patch;
};

class XEN_API Engine : NonCopyable {
private:
    static Engine* instance;

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
    [[nodiscard]] static Engine* get() { return instance; }

    Engine();
    ~Engine();

    [[nodiscard]] int32_t run();

    [[nodiscard]] Version const& get_version() const { return version; }

    [[nodiscard]] App* get_app() const { return app.get(); }

    void set_app(std::unique_ptr<App>&& app) { this->app = std::move(app); }

    [[nodiscard]] float get_fps_limit() const { return fps_limit; }

    void set_fps_limit(float fps_limit);

    [[nodiscard]] bool is_running() const { return running; }

    [[nodiscard]] Time const& get_delta() const { return delta_update.change; }

    [[nodiscard]] Time const& get_delta_render() const { return delta_render.change; }

    [[nodiscard]] uint32_t get_ups() const { return ups.value; }

    [[nodiscard]] uint32_t get_fps() const { return fps.value; }

    void request_close() { running = false; }

private:
    void create_module(Module::RegistryMapT::const_iterator it);
    void destroy_module(std::type_index id);
    void update_stage(Module::Stage stage);

    void process_updates();
    void process_rendering();
};
}