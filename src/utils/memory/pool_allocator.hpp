#pragma once

#include <cassert>
#include <cstdint>
#include <ostream>

namespace xen {
template <typename T>
class PoolAllocator {
public:
    struct Block {
        alignas(T) T data;
        size_t next;

        static constexpr size_t last_bit = 1ULL << (8 * sizeof(size_t) - 1);

        void mark_busy() { next |= last_bit; }

        [[nodiscard]] bool is_free() const { return !(next & last_bit); }
    };

private:
    Block* current = nullptr;

    size_t first = invalid_offset;

    size_t count = 0;

    constexpr static size_t invalid_offset = std::numeric_limits<size_t>::max() - Block::last_bit;

public:
    using data_ptr = uint8_t*;

    PoolAllocator() = default;

    PoolAllocator(data_ptr data, size_t bytes) { init(data, bytes); }

    ~PoolAllocator()
    {
        if (!current) {
            return;
        }

        constexpr size_t marked = invalid_offset - 1;
        size_t offset = first;
        while (offset != invalid_offset) {
            Block* block = current + offset;
            offset = block->next;
            block->next = marked;
        }
        for (Block* block = current; block != current + count; block++) {
            if (block->next != marked) {
                block->data.~T();
            }
        }
        count = 0;
        current = nullptr;
        first = invalid_offset;
    }

    // void init(data_ptr data, size_t bytes)
    // {
    //     Log::rt_assert(data != nullptr);
    //     Log::rt_assert(bytes >= sizeof(Block));

    //     current = reinterpret_cast<Block*>(data);
    //     first = 0;
    //     count = bytes / sizeof(Block);

    //     Block* last = current + count - 1;
    //     size_t offset = 1;
    //     for (Block* begin = current; begin != last; begin++, offset++) {
    //         begin->next = offset;
    //     }
    //     last->next = invalid_offset;
    // }
    void init(data_ptr data, size_t bytes)
    {
        Log::rt_assert(data != nullptr);

        if (data == nullptr || bytes < sizeof(Block)) {
            Log::rt_assert(data != nullptr && "Provided data pointer is null");
            Log::rt_assert(bytes >= sizeof(Block) && "Provided bytes is less than sizeof(Block)");

            current = nullptr;
            count = 0;
            first = invalid_offset;
            return;
        }

        Log::rt_assert(
            reinterpret_cast<uintptr_t>(data) % alignof(Block) == 0 &&
            "Provided data pointer is not aligned correctly for Block"
        );

        current = reinterpret_cast<Block*>(data);
        first = 0;
        count = bytes / sizeof(Block);

        if (count > 0) {
            Block* last = current + count - 1;
            size_t offset = 1;
            for (Block* begin = current; begin != last; ++begin, ++offset) {
                begin->next = offset;
            }
            last->next = invalid_offset;
        }
        else {
            first = invalid_offset;
        }
    }

    void transfer(data_ptr new_data, size_t new_bytes)
    {
        if (current == nullptr) {
            init(new_data, new_bytes);
            return;
        }

        size_t new_count = new_bytes / sizeof(Block);
        Log::rt_assert(new_data != nullptr);
        Log::rt_assert(count <= new_count);
        Log::rt_assert(
            reinterpret_cast<uintptr_t>(new_data) % alignof(Block) == 0 &&
            "New data pointer is not aligned correctly for Block"
        );

        if (new_count > count) {
            memcpy(new_data, current, count * sizeof(Block));

            auto* new_current = reinterpret_cast<Block*>(new_data);

            Block* first_new_block = new_current + count;
            Block* last_new_block = new_current + new_count - 1;

            size_t offset = count + 1;
            for (Block* begin = first_new_block; begin != last_new_block; ++begin, ++offset) {
                begin->next = offset;
            }

            last_new_block->next = first;

            first = count;

            current = new_current;
            count = new_count;
        }
        else if (new_data != reinterpret_cast<data_ptr>(current)) {
            memcpy(new_data, current, count * sizeof(Block));
            current = reinterpret_cast<Block*>(new_data);
        }
    }

    // memcpy(new_data, current, count * sizeof(Block)); // copy old blocks to new memory chunk

    // some objects in MxEngine tend to invoke clean up on destruction, even if they are moved.
    // to avoid this we put restriction on pool allocator objects to not to refer to themselves. Its better to use
    // macros like offsetof() than to store raw pointers to data which can be easily invalidated by rellocator

    // for (size_t i = 0; i < count; i++)
    // {
    //     auto& oldBlock = *(current + i);
    //     auto* _ = new((Block*)new_data + i) Block(std::move(oldBlock)); //-V799
    //     oldBlock.~Block();
    // }

    // current = (Block*)new_data;
    // Block* last = current + newCount - 1; // calculate last block to chain new ones
    // size_t offset = count + 1;
    // for (Block* begin = current + count; begin != last; begin++, offset++) {
    //     begin->next = offset;
    // }
    // last->next = first; // make last of new blocks point to the first free block of old
    // first = count;      // chain all new blocks to the free list
    // count = newCount;
    //}

    data_ptr get_base() { return current; }

    template <typename... Args>
    [[nodiscard]] T* alloc(Args&&... args)
    {
        Log::rt_assert(current != nullptr && "Allocator not initialized");
        Log::rt_assert(first != invalid_offset && "PoolAllocator is full");

        Block* block_to_alloc = current + first;
        T* data_ptr = &block_to_alloc->data;
        first = block_to_alloc->next;
        block_to_alloc->mark_busy();

        try {
            return new (data_ptr) T(std::forward<Args>(args)...);
        }
        catch (...) {
            block_to_alloc->next = first;
            first = static_cast<size_t>(block_to_alloc - current);
            // throw;
            // acquire error
        }
    }

    void free(T* object)
    {
        static_assert(offsetof(Block, data) == 0, "PoolAllocator requires T data to be the first member of Block.");

        if (!object) {
            return;
        }

        auto* block = reinterpret_cast<Block*>(object);

        Log::rt_assert(current != nullptr && "Allocator not initialized or already destroyed");
        Log::rt_assert(block >= current && block < current + count && "Pointer is out of pool bounds");
        Log::rt_assert(!block->is_free() && "Attempting to free an already free block");

        object->~T();

        block->next = first;
        first = static_cast<size_t>(block - current);
    }

    template <typename... Args>
    [[nodiscard]] auto stack_alloc(Args&&... args)
    {
        auto deleter = [this](T* ptr) { this->free(ptr); };
        using smart_ptr = std::unique_ptr<T, decltype(deleter)>;
        T* allocated_ptr = this->alloc(std::forward<Args>(args)...);
        return smart_ptr(allocated_ptr, std::move(deleter));
    }

    void dump(std::ostream& out) const
    {
        if (!current) {
            out << "--- PoolAllocator is not initialized --- \n";
            return;
        }
        out << std::hex;
        auto* raw_data = reinterpret_cast<data_ptr>(current);
        for (size_t i = 0; i < count * sizeof(Block); ++i) {
            out << static_cast<int>(raw_data[i]) << " ";
        }
        out << std::dec << "\n --- dumped " << count * sizeof(Block) << " bytes --- \n";
    }
};
}