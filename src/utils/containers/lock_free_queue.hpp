#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>
#include <stdexcept>
#include <new>
#include <utility>
#include <concepts>
#include <bit>
#include <thread>
#include <cassert>

#ifdef __cpp_lib_hardware_interference_size
constexpr size_t cache_line_size = std::hardware_destructive_interference_size;
#else
constexpr size_t cache_line_size = 64;
#endif

template <typename T>
concept LockFreeQueueValue = std::movable<T> && std::destructible<T>;

template <LockFreeQueueValue T>
class LockFreeMPMCQueue {
private:
    struct Node {
        std::atomic<uint64_t> turn = 0;
        alignas(T) std::array<std::byte, sizeof(T)> data;

        Node() noexcept = default;
    };

    // avoid false sharing
    alignas(cache_line_size) std::atomic<uint64_t> head = 0;
    alignas(cache_line_size) std::atomic<uint64_t> tail = 0;
    alignas(cache_line_size) const size_t capacity;
    size_t const capacity_mask; // for fast cap - 1

    std::vector<Node> buffer;

public:
    explicit LockFreeMPMCQueue(size_t desired_capacity = 10) :
        // determine that cap is pow of two for fast bit ops
        capacity(std::bit_ceil(desired_capacity)), capacity_mask(capacity - 1), buffer(capacity)
    {
        if (desired_capacity == 0) {
            throw std::invalid_argument("Queue capacity must be greater than 0");
        }
        if (!std::has_single_bit(capacity)) {
            throw std::logic_error("Internal error: capacity is not power of 2");
        }
    }

    ~LockFreeMPMCQueue()
    {
        uint64_t current_head = head.load(std::memory_order_relaxed);
        uint64_t current_tail = tail.load(std::memory_order_relaxed);

        for (uint64_t i = current_head; i < current_tail; ++i) {
            Node& node = buffer[i & capacity_mask];
            uint64_t turn = node.turn.load(std::memory_order_relaxed);
            if (turn == (i / capacity) + 1) {
                T* ptr = std::launder(reinterpret_cast<T*>(node.data.data()));
                ptr->~T();
            }
        }
    }

    LockFreeMPMCQueue(LockFreeMPMCQueue const&) = delete;
    LockFreeMPMCQueue& operator=(LockFreeMPMCQueue const&) = delete;
    LockFreeMPMCQueue(LockFreeMPMCQueue&&) = delete;
    LockFreeMPMCQueue& operator=(LockFreeMPMCQueue&&) = delete;

    template <typename... Args>
        requires std::constructible_from<T, Args...>
    [[nodiscard]] bool try_enqueue(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
    {
        uint64_t current_tail = tail.load(std::memory_order_relaxed);
        Node& node = buffer[current_tail & capacity_mask];
        uint64_t current_turn = node.turn.load(std::memory_order_acquire);

        if (current_turn == (current_tail / capacity)) {
            if (tail.compare_exchange_weak(
                    current_tail, current_tail + 1, std::memory_order_release, std::memory_order_relaxed
                )) {
                if constexpr (std::is_nothrow_constructible_v<T, Args...>) {
                    new (node.data.data()) T(std::forward<Args>(args)...);
                }
                else {
                    try {
                        ::new (node.data.data()) T(std::forward<Args>(args)...);
                    }
                    catch (...) {
                        // what we need to do?
                        throw;
                    }
                }

                node.turn.store((current_tail / capacity) + 1, std::memory_order_release);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] std::optional<T>
    try_dequeue() noexcept(std::is_nothrow_move_constructible_v<T> && std::is_nothrow_destructible_v<T>)
    {
        uint64_t current_head = head.load(std::memory_order_relaxed);
        Node& node = buffer[current_head & capacity_mask];
        uint64_t current_turn = node.turn.load(std::memory_order_acquire);

        if (current_turn == (current_head / capacity) + 1) {
            if (head.compare_exchange_weak(
                    current_head, current_head + 1, std::memory_order_release, std::memory_order_relaxed
                )) {
                T* ptr = std::launder(reinterpret_cast<T*>(node.data.data()));
                T value = std::move(*ptr);
                ptr->~T();

                node.turn.store((current_head + capacity) / capacity, std::memory_order_release);
                return {std::move(value)};
            }
        }
        return std::nullopt;
    }

    template <typename... Args>
        requires std::constructible_from<T, Args...>
    void enqueue(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
    {
        while (!try_enqueue(std::forward<Args>(args)...)) {
            std::this_thread::yield();
            // cond var?
        }
    }

    T dequeue() noexcept(std::is_nothrow_move_constructible_v<T> && std::is_nothrow_destructible_v<T>)
    {
        std::optional<T> result;
        while (!(result = try_dequeue())) {
            std::this_thread::yield();
            // cond var?
        }
        return std::move(*result);
    }

    [[nodiscard]] size_t get_capacity() const noexcept { return capacity; }

    [[nodiscard]] size_t approx_size() const noexcept
    {
        uint64_t current_tail = tail.load(std::memory_order_relaxed);
        uint64_t current_head = head.load(std::memory_order_relaxed);
        return (current_tail >= current_head) ? (current_tail - current_head) : 0;
    }

    [[nodiscard]] bool empty() const noexcept
    {
        uint64_t current_tail = tail.load(std::memory_order_relaxed);
        uint64_t current_head = head.load(std::memory_order_relaxed);
        return current_head == current_tail;
    }
};