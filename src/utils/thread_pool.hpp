#pragma once

// std::thread is not available on MinGW with Win32 threads
#if defined(__MINGW32__) && !defined(_GLIBCXX_HAS_GTHREADS)
#pragma message(                                                                                                       \
    "Warning: Threads are not available with your compiler; check that you're using POSIX threads and not Win32 ones." \
)
#else
#define XEN_THREADS_AVAILABLE
#endif

#if defined(XEN_THREADS_AVAILABLE)

namespace xen {
class ThreadPool {
public:
    ThreadPool();
    explicit ThreadPool(uint32_t thread_count);
    ~ThreadPool();

    void add_task(std::function<void()> task);

private:
    std::vector<std::thread> threads{};
    bool should_stop = false;

    std::mutex tasks_mutex{};
    std::condition_variable cond_var{};
    std::queue<std::function<void()>> tasks{};
};
}

#endif // XEN_THREADS_AVAILABLE