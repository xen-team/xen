#include "threading.hpp"

#ifdef RAZ_THREADS_AVAILABLE

namespace xen {
ThreadPool& get_default_thread_pool()
{
    static ThreadPool thread_pool;
    return thread_pool;
}

void parallelize(std::function<void()> const& action, uint task_count)
{
    if (task_count == 0) {
        throw std::invalid_argument("[Threading] The number of tasks cannot be 0.");
    }

#if !defined(XEN_IS_PLATFORM_EMSCRIPTEN)
    ThreadPool& thread_pool = get_default_thread_pool();

    std::vector<std::promise<void>> promises;
    promises.resize(task_count);

    for (uint task_index = 0; task_index < task_count; ++task_index) {
        thread_pool.add_task([&action, &promises, task_index]() {
            action();
            promises[task_index].set_value();
        });
    }

    // Blocking here waiting for all tasks to be finished
    for (std::promise<void>& promise : promises) {
        promise.get_future().wait();
    }

#else
    for (uint i = 0; i < task_count; ++i) {
        action();
    }
#endif
}

void parallelize(std::initializer_list<std::function<void()>> actions)
{
#if !defined(XEN_IS_PLATFORM_EMSCRIPTEN)
    ThreadPool& thread_pool = get_default_thread_pool();

    std::vector<std::promise<void>> promises;
    promises.resize(actions.size());

    for (uint task_index = 0; task_index < actions.size(); ++task_index) {
        thread_pool.add_task([&actions, &promises, task_index]() {
            std::function<void()> const& action = *(actions.begin() + task_index);
            action();
            promises[task_index].set_value();
        });
    }

    // Blocking here waiting for all tasks to be finished
    for (std::promise<void>& promise : promises) {
        promise.get_future().wait();
    }

#else
    for (const std::function<void()>& action : actions) {
        action();
    }
#endif
}
}

#endif // RAZ_THREADS_AVAILABLE
