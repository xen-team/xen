
#include "thread_pool.hpp"

#include <utils/threading.hpp>

#include <tracy/Tracy.hpp>

namespace xen {
ThreadPool::ThreadPool() : ThreadPool(get_system_thread_count()) {}

ThreadPool::ThreadPool(uint32_t thread_count)
{
    ZoneScopedN("ThreadPool::ThreadPool");

    Log::debug("[ThreadPool] Initializing (with " + std::to_string(thread_count) + " thread(s))...");

    threads.reserve(thread_count);

    for (uint32_t thread_index = 0; thread_index < thread_count; ++thread_index) {
        threads.emplace_back([this
#if defined(TRACY_ENABLE)
                              ,
                              thread_index
#endif
        ]() {
#if defined(TRACY_ENABLE)
            const std::string thread_name = "Thread pool - #" + std::to_string(thread_index + 1);
            tracy::SetThreadName(thread_name.c_str());
#endif

            std::function<void()> task;

            while (true) {
                {
                    std::unique_lock<std::mutex> lock(tasks_mutex);
                    cond_var.wait(lock, [this]() { return (!tasks.empty() || should_stop); });

                    if (should_stop) {
                        return;
                    }

                    task = std::move(tasks.front());
                    tasks.pop();
                }

                task();
            }
        });
    }

    Log::debug("[ThreadPool] Initialized");
}

void ThreadPool::add_task(std::function<void()> task)
{
    {
        std::lock_guard<std::mutex> const lock(tasks_mutex);
        tasks.push(std::move(task));
    }

    cond_var.notify_one();
}

ThreadPool::~ThreadPool()
{
    ZoneScopedN("ThreadPool::~ThreadPool");

    Log::debug("[ThreadPool] Destroying...");

    {
        std::lock_guard<std::mutex> const lock(tasks_mutex);
        should_stop = true;
    }

    cond_var.notify_all();

    for (std::thread& thread : threads) {
        thread.join();
    }

    Log::debug("[ThreadPool] Destroyed");
}
}