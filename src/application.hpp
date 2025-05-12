#pragma once

#include <world.hpp>
#include <data/bitset.hpp>

namespace xen {
struct FrameTimeInfo {
    float delta_time{};   ///< Time elapsed since the application's last execution, in seconds.
    float global_time{};  ///< Time elapsed since the application started, in seconds.
    int substep_count{};  ///< Amount of fixed time steps to process.
    float substep_time{}; ///< Time to be used by each fixed time step, in seconds.
};

class Application {
public:
    explicit Application(size_t world_count = 1);

    std::vector<WorldPtr> const& get_worlds() const { return worlds; }

    std::vector<WorldPtr>& get_worlds() { return worlds; }

    FrameTimeInfo const& get_time_info() const { return time_info; }

    void set_fixed_time_step(float fixed_time_step)
    {
        Log::rt_assert("Error: Fixed time step must be positive." && fixed_time_step > 0.f);
        time_info.substep_time = fixed_time_step;
    }

    /// Adds a world into the application.
    /// \tparam Args Types of the arguments to be forwarded to the world.
    /// \param args Arguments to be forwarded to the world.
    /// \return Reference to the newly added world.
    template <typename... Args>
    World& add_world(Args&&... args);

    /// Runs the application.
    void run();

    /// Runs the application and call the given callable on each cycle.
    /// \tparam FuncT Type of the callback to call on each cycle.
    /// \param callback Callback to call on each cycle.
    template <typename FuncT>
    void run(FuncT&& callback);

    /// Runs one cycle of the application.
    /// \return True if the application is still running, false otherwise.
    bool run_once();

    /// Tells the application to stop running.
    void quit() { running = false; }

private:
    std::vector<WorldPtr> worlds{};
    Bitset active_worlds{};

    FrameTimeInfo time_info{0.f, 0.f, 0, 0.016666f}; ///< Time-related attributes for each cycle.
    std::chrono::time_point<std::chrono::system_clock> last_frame_time = std::chrono::system_clock::now();
    float remaining_time{}; ///< Extra time remaining after executing the systems' fixed step update.

    bool running = true;
};
}

#include "application.inl"