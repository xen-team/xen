#include "timers.hpp"

#include <algorithm>

namespace xen {
SimpleTimer::SimpleTimer() : start_time(glfwGetTime()) {}

void SimpleTimer::reset()
{
    start_time = glfwGetTime();
}

void SimpleTimer::rewind(double time)
{
    start_time += time;
    if (elapsed() < 0.0) {
        start_time = glfwGetTime();
    }
}

double SimpleTimer::elapsed() const
{
    return glfwGetTime() - start_time;
}

Timers::Timers()
{
    std::unique_lock<std::mutex> lock(mutex);
    worker = std::thread([this] { thread_run(); });
}

Timers::~Timers()
{
    stop = true;

    condition.notify_all();
    worker.join();
}

void Timers::update() {}

void Timers::thread_run()
{
    std::unique_lock<std::mutex> lock(mutex);

    while (!stop) {
        if (timers.empty()) {
            condition.wait(lock);
        }
        else {
            std::ranges::sort(timers, [](auto const& a, auto const& b) { return a->next < b->next; });

            auto& instance = timers.front();
            auto time = Time::now();

            if (time >= instance->next) {
                // Log::Error("Timer variation: ", (time - instance->next).AsMilliseconds<float>(), "ms\n");
                lock.unlock();
                instance->on_tick.invoke();
                lock.lock();

                instance->next += instance->interval;

                if (instance->repeat) {
                    if (--*instance->repeat == 0) {
                        timers.erase(std::remove(timers.begin(), timers.end(), instance), timers.end());
                    }
                }
            }
            else {
                std::chrono::microseconds time_point(instance->next - time);
                condition.wait_for(lock, time_point);
            }
        }
    }
}
}