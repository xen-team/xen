#pragma once

// std::thread is not available on MinGW with Win32 threads
#if defined(__MINGW32__) && !defined(_GLIBCXX_HAS_GTHREADS)
#pragma message(                                                                                                       \
    "Warning: Threads are not available with your compiler; check that you're using POSIX threads and not Win32 ones." \
)
#else
#define RAZ_THREADS_AVAILABLE
#endif

#if defined(RAZ_THREADS_AVAILABLE)

namespace xen {
class ThreadPool;

struct IndexRange {
    size_t begin_index;
    size_t end_index;
};

template <typename IterT, typename = void>
class IterRange {};

template <typename IterT>
class IterRange<IterT, std::void_t<typename std::iterator_traits<IterT>::iterator_category>> {
public:
    IterRange(IterT begin, IterT end) : begin_{begin}, end_{end} {}

    IterT begin() const { return begin_; }
    IterT end() const { return end_; }

private:
    IterT begin_;
    IterT end_;
};

/// Gets the number of concurrent threads available to the system.
/// This number doesn't necessarily represent the CPU's actual number of threads.
/// \return Number of threads available.
inline uint get_system_thread_count()
{
    return std::max(std::thread::hardware_concurrency(), 1u);
}

/// Gets the default thread pool, initialized with the default number of threads (defined by get_system_thread_count()).
/// \return Reference to the default thread pool.
ThreadPool& get_default_thread_pool();

/// Pauses the current thread for the specified amount of time.
/// \param milliseconds Pause duration in milliseconds.
inline void sleep(uint64_t milliseconds)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

/// Calls a function asynchronously, to be executed without blocking the calling thread.
/// \note If using Emscripten this call will be synchronous, threads being unsupported with it for now.
/// \tparam FuncT Type of the action to be executed.
/// \tparam Args Types of the arguments to be forwarded to the given function.
/// \tparam ResultT Return type of the given function.
/// \param action Action to be performed asynchronously.
/// \param args Arguments to be forwarded to the given function.
/// \return A std::future holding the future result of the process. This should not be discarded, as doing so will fail
/// to run asynchronously.
template <typename FuncT, typename... Args, typename ResultT = std::invoke_result_t<FuncT&&, Args&&...>>
[[nodiscard]] std::future<ResultT> launch_async(FuncT&& action, Args&&... args);

/// Calls a function in parallel.
/// \note If using Emscripten this call will be synchronous, threads being unsupported with it for now.
/// \param action Action to be performed in parallel.
/// \param task_count Amount of tasks to start.
void parallelize(std::function<void()> const& action, uint task_count = get_system_thread_count());

/// Calls the given functions in parallel.
/// \note If using Emscripten this call will be synchronous, threads being unsupported with it for now.
/// \param actions Actions to be performed in parallel.
void parallelize(std::initializer_list<std::function<void()>> actions);

/// Calls a function in parallel over an index range.
/// The given index range is automatically split, providing a separate start/past-the-end index sub-range to each task.
/// \note If using Emscripten this call will be synchronous, threads being unsupported with it for now.
/// \tparam BegIndexT Type of the begin index.
/// \tparam EndIndexT Type of the end index.
/// \tparam FuncT Type of the action to be executed.
/// \param begin_index Starting index of the whole range. Must be lower than the end index.
/// \param end_index Past-the-last index of the whole range. Must be greater than the begin index.
/// \param action Action to be performed in parallel, taking an index range as boundaries.
/// \param task_count Amount of tasks to start.
template <
    typename BegIndexT, typename EndIndexT, typename FuncT,
    typename = std::enable_if_t<std::is_integral_v<BegIndexT> && std::is_integral_v<EndIndexT>>>
void parallelize(
    BegIndexT begin_index, EndIndexT end_index, FuncT const& action, uint task_count = get_system_thread_count()
);

/// Calls a function in parallel over an iterator range.
/// The given iterator range is automatically split, providing a separate start/past-the-end iterator sub-range to each
/// task.
/// \note If using Emscripten this call will be synchronous, threads being unsupported with it for now.
/// \tparam IterT Type of the iterators.
/// \tparam FuncT Type of the action to be executed.
/// \param begin Begin iterator of the whole range. Must be lower than the end iterator.
/// \param end End iterator of the whole range. Must be greater than the begin iterator.
/// \param action Action to be performed in parallel, taking an iterator range as boundaries.
/// \param task_count Amount of tasks to start.
template <typename IterT, typename FuncT, typename = typename std::iterator_traits<IterT>::iterator_category>
void parallelize(IterT begin, IterT end, FuncT const& action, uint task_count = get_system_thread_count());

/// Calls a function in parallel over a collection.
/// The given collection is automatically split, providing a separate start/past-the-end iterator sub-range to each
/// task.
/// \note The container must either be a constant-size C array, or have public begin() & end() functions.
/// \note If using Emscripten this call will be synchronous, threads being unsupported with it for now.
/// \tparam ContainerT Type of the collection to iterate over.
/// \tparam FuncT Type of the action to be executed.
/// \param collection Collection to iterate over in parallel.
/// \param action Action to be performed in parallel, taking an iterator range as boundaries.
/// \param task_count Amount of tasks to start.
template <typename ContainerT, typename FuncT, typename = decltype(std::begin(std::declval<ContainerT>()))>
void parallelize(ContainerT&& collection, FuncT&& action, uint task_count = get_system_thread_count())
{
    parallelize(std::begin(collection), std::end(collection), std::forward<FuncT>(action), task_count);
}
}

#include "threading.inl"

#endif // RAZ_THREADS_AVAILABLE