#pragma once

#include "utils/memory/pool_allocator.hpp"

namespace xen {
template <typename T, template <typename, typename...> typename Container = std::vector>
class PoolVector {
public:
    using Allocator = PoolAllocator<T>;
    using Block = typename Allocator::Block;
    using UnderlyingContainer = Container<uint8_t>;

    using value_type = T;
    using pointer = T*;
    using const_pointer = T const*;
    using reference = T&;
    using const_reference = T const&;
    using difference_type = std::ptrdiff_t;
    using size_type = size_t;

    class PoolIterator {
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = T;
        using pointer = T*;
        using reference = T&;

    private:
        size_t current_index = 0;

        PoolVector<T, Container> const* pool_ptr;

    private:
        void find_next_valid()
        {
            while (current_index < pool_ptr->capacity() && !pool_ptr->is_allocated(current_index)) {
                ++current_index;
            }
        }

        void find_prev_valid()
        {
            while (current_index > 0) {
                --current_index;
                if (pool_ptr->is_allocated(current_index)) {
                    return;
                }
            }

            if (current_index == 0 && !pool_ptr->is_allocated(0)) {
                // something
            }
        }

    public:
        PoolIterator() noexcept = default;
        PoolIterator(size_t index, PoolVector& vec) noexcept : current_index(index), pool_ptr(&vec)
        {
            if (current_index < pool_ptr->capacity()) {
                find_next_valid();
            }
        }

        reference operator*() const
        {
            Log::rt_assert(pool_ptr && current_index < pool_ptr->capacity(), "Dereferencing invalid iterator");
            return (*pool_ptr)[current_index];
        }

        pointer operator->() const
        {
            Log::rt_assert(pool_ptr && current_index < pool_ptr->capacity(), "Dereferencing invalid iterator");
            return std::addressof((*pool_ptr)[current_index]);
        }

        PoolIterator& operator++()
        {
            if (pool_ptr && current_index < pool_ptr->capacity()) {
                ++current_index;
                find_next_valid();
            }
            else {
                Log::rt_assert(pool_ptr, "Incrementing invalid iterator");
            }
            return *this;
        }

        PoolIterator operator++(int)
        {
            PoolIterator temp = *this;
            ++(*this);
            return temp;
        }

        PoolIterator& operator--()
        {
            if (pool_ptr) {
                if (current_index == 0) {
                    Log::rt_assert(false, "Decrementing begin iterator");
                    current_index = pool_ptr->capacity();
                }
                else {
                    find_prev_valid();
                }
            }
            else {
                Log::rt_assert(pool_ptr, "Decrementing invalid iterator");
            }

            return *this;
        }

        PoolIterator operator--(int)
        {
            PoolIterator temp = *this;
            --(*this);
            return temp;
        }

        friend bool operator==(PoolIterator const& a, PoolIterator const& b) noexcept
        {
            return a.pool_ptr == b.pool_ptr && a.current_index == b.current_index;
        }
        friend bool operator!=(PoolIterator const& a, PoolIterator const& b) noexcept { return !(a == b); }

        [[nodiscard]] size_t get_index() const { return current_index; }
    };

private:
    UnderlyingContainer memory_storage;
    Allocator allocator;
    size_t allocated_count = 0;

private:
    Block* get_block_by_index(size_t index)
    {
        size_t const byte_index = index * sizeof(Block);
        Log::rt_assert(byte_index < memory_storage.size(), "Index out of bounds");
        return reinterpret_cast<Block*>(memory_storage.data() + byte_index);
    }

    Block const* get_block_by_index(size_t index) const
    {
        size_t const byte_index = index * sizeof(Block);
        Log::rt_assert(byte_index < memory_storage.size(), "Index out of bounds");
        return reinterpret_cast<Block*>(memory_storage.data() + byte_index);
    }

public:
    PoolVector() = default;
    explicit PoolVector(size_t initial_capacity) { resize(initial_capacity); }

    ~PoolVector() = default;

    void resize(size_t new_capacity)
    {
        if (new_capacity <= capacity()) {
            return;
        }

        UnderlyingContainer new_memory(new_capacity * sizeof(Block));

        allocator.transfer(new_memory.data(), new_memory.size());

        memory_storage = std::move(new_memory);
    }

    [[nodiscard]] size_t capacity() const { return memory_storage.size() / sizeof(Block); }

    [[nodiscard]] size_t capacity_in_bytes() const { return memory_storage.size(); }

    [[nodiscard]] bool empty() const { return allocated_count == 0; }

    [[nodiscard]] size_t size() const { return allocated_count; }

    T& operator[](size_t index)
    {
        Log::rt_assert(is_allocated(index), "Accessing non-allocated element via operator[]");
        return get_block_by_index(index)->data;
    }

    T const& operator[](size_t index) const
    {
        Log::rt_assert(is_allocated(index), "Accessing non-allocated element via operator[]");
        return get_block_by_index(index)->data;
    }

    void clear()
    {
        allocator = Allocator();
        memory_storage.clear();
        allocated_count = 0;
    }

    [[nodiscard]] bool is_allocated(size_t index) const
    {
        return index < capacity() && !get_block_by_index(index)->is_free();
    }

    template <typename... Args>
    size_t allocate(Args&&... args)
    {
        if (allocated_count == capacity()) {
            size_t new_cap = capacity() * 3 / 2;
            if (new_cap <= capacity()) {
                new_cap = capacity() + 1;
            }
            resize(new_cap);
        }

        T* new_obj_ptr = allocator.alloc(std::forward<Args>(args)...);
        allocated_count++;
        return index_of(*new_obj_ptr);
    }

    void deallocate(size_t index)
    {
        if (is_allocated(index)) {
            T& obj_ref = get_block_by_index(index)->data;
            allocator.free(&obj_ref);
            allocated_count--;
        }
    }

    void deallocate(PoolIterator const& it)
    {
        Log::rt_assert(it.vector_ptr == this, "Iterator does not belong to this PoolVector");
        deallocate(it.get_base());
    }

    size_t index_of(const_reference obj) const
    {
        auto const* block_ptr = reinterpret_cast<Block const*>(std::addressof(obj));
        auto const* start_ptr = reinterpret_cast<Block const*>(memory_storage.data());

        Log::rt_assert(memory_storage.data() != nullptr, "Memory storage is null");
        Log::rt_assert(
            reinterpret_cast<uint8_t const*>(block_ptr) >= memory_storage.data() &&
                reinterpret_cast<uint8_t const*>(block_ptr) < memory_storage.data() + memory_storage.size(),
            "Object reference is outside the bounds of the memory storage"
        );

        difference_type diff = block_ptr - start_ptr;
        Log::rt_assert(diff >= 0 && static_cast<size_t>(diff) < capacity(), "Invalid object reference for index_of");
        return static_cast<size_t>(diff);
    }

    size_t index_of(const_pointer obj) const
    {
        Log::rt_assert(obj != nullptr, "Null pointer passed to index_of");
        return index_of(*obj);
    }

    auto begin() { return PoolIterator(0, *this); }
    auto end() { return PoolIterator(capacity(), *this); }
};
}