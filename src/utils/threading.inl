#include <utils/thread_pool.hpp>

namespace xen {
template <typename FuncT, typename... Args, typename ResultT>
std::future<ResultT> launch_async(FuncT&& action, Args&&... args)
{
#if !defined(XEN_IS_PLATFORM_EMSCRIPTEN)
    return std::async(std::forward<FuncT>(action), std::forward<Args>(args)...);
#else
    std::promise<ResultT> promise;
    promise.set_value(action(std::forward<Args>(args)...));
    return promise.get_future();
#endif
}

template <typename BegIndexT, typename EndIndexT, typename FuncT, typename>
void parallelize(BegIndexT begin_index, EndIndexT end_index, FuncT const& action, uint32_t task_count)
{
    static_assert(
        std::is_invocable_v<FuncT, IndexRange>, "Error: The given action must take an IndexRange as parameter"
    );

    if (task_count == 0) {
        throw std::invalid_argument("[Threading] The number of tasks cannot be 0.");
    }

    if (static_cast<std::ptrdiff_t>(begin_index) >= static_cast<std::ptrdiff_t>(end_index)) {
        throw std::invalid_argument("[Threading] The given index range is invalid.");
    }

#if !defined(XEN_IS_PLATFORM_EMSCRIPTEN)
    ThreadPool& thread_pool = get_default_thread_pool();

    auto const total_range_count = static_cast<size_t>(end_index) - static_cast<size_t>(begin_index);
    size_t const max_task_count = std::min(static_cast<size_t>(task_count), total_range_count);

    size_t const per_task_range_count = total_range_count / max_task_count;
    size_t remainder_element_count = total_range_count % max_task_count;
    auto task_begin_index = static_cast<size_t>(begin_index);

    std::vector<std::promise<void>> promises;
    promises.resize(max_task_count);

    for (size_t task_index = 0; task_index < max_task_count; ++task_index) {
        size_t const task_end_index = task_begin_index + per_task_range_count + (remainder_element_count > 0 ? 1 : 0);

        thread_pool.add_task([&action, &promises, task_begin_index, task_end_index,
                              task_index]() noexcept(std::is_nothrow_invocable_v<FuncT, IndexRange>) {
            action(IndexRange{task_begin_index, task_end_index});
            promises[task_index].set_value();
        });

        task_begin_index = task_end_index;

        if (remainder_element_count > 0) {
            --remainder_element_count;
        }
    }

    // Blocking here waiting for all tasks to be finished
    for (std::promise<void>& promise : promises) {
        promise.get_future().wait();
    }

#else
    static_cast<void>(task_count);
    action(IndexRange{static_cast<size_t>(begin_index), static_cast<size_t>(end_index)});
#endif
}

template <typename IterT, typename FuncT, typename>
void parallelize(IterT begin, IterT end, FuncT const& action, uint32_t task_count)
{
    static_assert(
        std::is_invocable_v<FuncT, IterRange<IterT>>, "Error: The given action must take an IterRange as parameter"
    );

    if (task_count == 0) {
        throw std::invalid_argument("[Threading] The number of tasks cannot be 0.");
    }

    auto const total_range_count = std::distance(begin, end);

    if (total_range_count <= 0) {
        throw std::invalid_argument("[Threading] The given iterator range is invalid.");
    }

#if !defined(XEN_IS_PLATFORM_EMSCRIPTEN)
    ThreadPool& thread_pool = get_default_thread_pool();

    size_t const max_task_count = std::min(static_cast<size_t>(task_count), static_cast<size_t>(total_range_count));

    size_t const per_task_range_count = static_cast<size_t>(total_range_count) / max_task_count;
    size_t remainder_element_count = static_cast<size_t>(total_range_count) % max_task_count;
    IterT task_begin_it = begin;

    std::vector<std::promise<void>> promises;
    promises.resize(max_task_count);

    for (size_t task_index = 0; task_index < max_task_count; ++task_index) {
        IterT const task_end_it = std::next(
            task_begin_it, static_cast<std::ptrdiff_t>(per_task_range_count + (remainder_element_count > 0 ? 1 : 0))
        );

        thread_pool.add_task([&action, &promises, task_begin_it, task_end_it,
                              task_index]() noexcept(std::is_nothrow_invocable_v<FuncT, IterRange<IterT>>) {
            action(IterRange<IterT>(task_begin_it, task_end_it));
            promises[task_index].set_value();
        });

        task_begin_it = task_end_it;

        if (remainder_element_count > 0) {
            --remainder_element_count;
        }
    }

    // Blocking here waiting for all tasks to be finished
    for (std::promise<void>& promise : promises) {
        promise.get_future().wait();
    }

#else
    static_cast<void>(task_count);
    action(IterRange<IterT>(begin, end));
#endif
}
}