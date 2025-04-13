#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

#include <rocket.hpp>

#include "engine/engine.hpp"
#include "utils/time.hpp"

namespace xen {
class XEN_API SimpleTimer {
private:
    double start_time;

public:
    SimpleTimer();

    void reset();
    void rewind(double time);

    [[nodiscard]] double elapsed() const;
};

class XEN_API Timer {
    friend class Timers;

private:
    Time interval;
    Time next;
    std::optional<uint32_t> repeat;
    bool destroyed = false;

public:
    rocket::signal<void()> on_tick;

public:
    Timer(Time const& interval, std::optional<uint32_t> const& repeat) :
        interval(interval), next(Time::now() + interval), repeat(repeat)
    {
    }

    Time const& get_interval() const { return interval; }
    std::optional<uint32_t> const& get_repeat() const { return repeat; }
    bool is_destroyed() const { return destroyed; }
    void destroy() { destroyed = true; }
};

class XEN_API Timers : public Module::Registrar<Timers> {
    inline static bool const registered = Register("Timers", Stage::Post);

private:
    std::vector<std::unique_ptr<Timer>> timers;

    std::atomic_bool stop = false;
    std::thread worker;

    std::mutex mutex;
    std::condition_variable condition;

public:
    Timers();
    ~Timers();

    void update() override;

    template <class Instance>
    Timer* once(Instance* object, std::function<void()>&& function, Time const& delay)
    {
        std::unique_lock<std::mutex> lock(mutex);
        auto& instance = timers.emplace_back(std::make_unique<Timer>(delay, 1));
        instance->on_tick.connect(object, std::move(function));
        condition.notify_all();
        return instance.get();
    }

    template <class Instance>
    Timer* every(Instance* object, std::function<void()>&& function, Time const& interval)
    {
        std::unique_lock<std::mutex> lock(mutex);
        auto& instance = timers.emplace_back(std::make_unique<Timer>(interval, std::nullopt));
        instance->on_tick.connect(object, std::move(function));
        condition.notify_all();
        return instance.get();
    }

    template <class Instance>
    Timer* repeat(Instance* object, std::function<void()>&& function, Time const& interval, uint32_t repeat)
    {
        std::unique_lock<std::mutex> lock(mutex);
        auto& instance = timers.emplace_back(std::make_unique<Timer>(interval, repeat));
        instance->on_tick.connect(object, std::move(function));
        condition.notify_all();
        return instance.get();
    }

    Timer* once(std::function<void()>&& function, Time const& delay)
    {
        std::unique_lock<std::mutex> lock(mutex);
        auto& instance = timers.emplace_back(std::make_unique<Timer>(delay, 1));
        instance->on_tick.connect(std::move(function));
        condition.notify_all();
        return instance.get();
    }

    Timer* every(std::function<void()>&& function, Time const& interval)
    {
        std::unique_lock<std::mutex> lock(mutex);
        auto& instance = timers.emplace_back(std::make_unique<Timer>(interval, std::nullopt));
        instance->on_tick.connect(std::move(function));
        condition.notify_all();
        return instance.get();
    }

    Timer* repeat(std::function<void()>&& function, Time const& interval, uint32_t repeat)
    {
        std::unique_lock<std::mutex> lock(mutex);
        auto& instance = timers.emplace_back(std::make_unique<Timer>(interval, repeat));
        instance->on_tick.connect(std::move(function));
        condition.notify_all();
        return instance.get();
    }

private:
    void thread_run();
};
}