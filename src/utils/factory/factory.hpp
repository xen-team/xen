#pragma once

#include <utils/uuid.hpp>
#include <utils/containers/pool_vector.hpp>

namespace xen {
template <typename T>
struct ManagedResource {
    UUID uuid;
    T value;
    size_t ref_count = 0;

    template <typename... Args>
    ManagedResource(const UUID& uuid, Args&&... value) : uuid(uuid), value(std::forward<Args>(value)...)
    {
    }

    ManagedResource(ManagedResource const&) = delete;
    ManagedResource(ManagedResource&&) noexcept(std::is_nothrow_move_constructible_v<T>) = default;
    ManagedResource& operator=(ManagedResource const&) = delete;
    ManagedResource& operator=(ManagedResource&&) noexcept(std::is_nothrow_move_assignable_v<T>) = default;
};

template <typename T, typename F>
class Resource {
public:
    using Type = T;
    using Factory = F;

private:
    UUID uuid;
    size_t handle;

    static constexpr size_t invalid_handle = std::numeric_limits<size_t>::max();

private:
    void inc_ref()
    {
        if (is_valid()) {
            ++(dereference().ref_count);
        }
    }

    void dec_ref()
    {
        if (is_valid()) {
            auto& resource = dereference();
            if ((--resource.ref_count) == 0) {
                destroy_this(*this);
            }
        }
    }

    void destroy_this(Resource<T, F>& resource) { F::destroy(resource); }

    [[nodiscard]] ManagedResource<T>& dereference() const { return access_this(handle); }

    [[nodiscard]] ManagedResource<T>& access_this(size_t handle) const { return F::template get_pool<T>()[handle]; }

public:
    Resource() : handle(Resource<T, F>::invalid_handle) {};
    Resource(UUID uuid, size_t handle) : uuid{uuid}, handle{handle} { inc_ref(); }
    Resource(Resource const& other) : uuid{other.uuid}, handle{other.handle} { inc_ref(); }
    Resource(Resource&& other) noexcept : uuid{other.uuid}, handle{other.handle} { other.handle = invalid_handle; }
    ~Resource() { dec_ref(); }

    Resource& operator=(Resource const& other)
    {
        if (this != &other) {
            dec_ref();

            uuid = other.uuid;
            handle = other.handle;

            inc_ref();
        }
        return *this;
    }
    Resource& operator=(Resource&& other) noexcept
    {
        if (this != &other) {
            dec_ref();

            uuid = other.uuid;
            handle = other.handle;

            other.handle = invalid_handle;
        }
        return *this;
    }

    void acquire_ownership() { inc_ref(); }

    [[nodiscard]] bool is_valid() const { return handle != invalid_handle && dereference().uuid == uuid; }

    [[nodiscard]] T* get_unchecked() { return &dereference().value; }

    [[nodiscard]] T const* get_unchecked() const { return &dereference().value; }

    [[nodiscard]] size_t get_handle() const { return handle; }

    [[nodiscard]] const UUID& get_uuid() const { return uuid; }

    [[nodiscard]] T* operator->()
    {
        Log::rt_assert(is_valid());
        return get_unchecked();
    }

    [[nodiscard]] T const* operator->() const
    {
        Log::rt_assert(is_valid());
        return get_unchecked();
    }

    [[nodiscard]] T& operator*()
    {
        Log::rt_assert(is_valid());
        return *get_unchecked();
    }

    [[nodiscard]] T const& operator*() const
    {
        Log::rt_assert(is_valid());
        return *get_unchecked();
    }

    [[nodiscard]] auto operator<=>(Resource const& other) const = default;
};

template <typename T>
class Factory {
    using FactoryPool = PoolVector<ManagedResource<T>>;
    using ThisType = Factory<T>;

    [[nodiscard]] static FactoryPool& instance()
    {
        static FactoryPool pool_instance;
        return pool_instance;
    }

public:
    [[nodiscard]] static FactoryPool& get_pool() { return instance(); }

    static void clear_pool() { instance().clear(); }

    static void destroy(Resource<T, ThisType>& resource)
    {
        if (resource.is_valid()) {
            get_pool().deallocate(resource.get_handle());
        }
    }

    [[nodiscard]] static Resource<T, ThisType> get_handle(ManagedResource<T> const& object)
    {
        auto& pool_ref = get_pool();
        size_t index = pool_ref.index_of(object);
        Log::rt_assert(&pool_ref[index] == &object);
        return Resource<T, ThisType>(pool_ref[index].uuid, index);
    }

    // [[nodiscard]] static Resource<T, ThisType> get_handle(T const& object)
    // {
    //     auto const* ptr = (uint8_t const*)std::addressof(object);
    //     auto resource_ptr = (ManagedResource<T> const*)(ptr - sizeof(UUID));
    //     return Factory<T>::get_handle(*resource_ptr);
    // }

    template <typename... Args>
        requires std::constructible_from<T, Args...>
    [[nodiscard]] static Resource<T, ThisType> create(Args&&... args)
    {
        UUID uuid;
        size_t index = get_pool().allocate(uuid, std::forward<Args>(args)...);
        return Resource<T, ThisType>(uuid, index);
    }
};

#define XEN_MAKE_FACTORY(name)                                                                                         \
    using name##Factory = ::xen::Factory<name>;                                                                        \
    using name##Handle = ::xen::Resource<name, name##Factory>
}