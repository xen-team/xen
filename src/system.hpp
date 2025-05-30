#pragma once

#include <entity.hpp>
#include <data/bitset.hpp>

namespace xen {
struct FrameTimeInfo;
class System;
using SystemPtr = std::unique_ptr<System>;

/// System class representing a base System to be inherited.
class System {
    friend class World;

public:
    System(System const&) = delete;
    System(System&&) = delete;

    System& operator=(System const&) = delete;
    System& operator=(System&&) = delete;

    virtual ~System() = default;

    Bitset const& get_accepted_components() const { return accepted_components; }

    void pause() { paused = true; }
    void unpause() { paused = false; }

    /// Gets the ID of the given system type.
    /// It uses CRTP to assign a different ID to each system type it is called with.
    /// This function will be instantiated every time it is called with a different type, incrementing the assigned
    /// index. Note that it must be called directly from System, and a derived class must be given
    /// (System::get_id<DerivedSystem>()).
    /// \tparam SysT Type of the system to get the ID of.
    /// \return Given system's ID.
    template <typename SysT>
    static size_t get_id();

    /// Checks if the system contains the given entity.
    /// \param entity Entity to be checked.
    /// \return True if the system contains the entity, false otherwise.
    bool contains_entity(Entity const& entity) const;

    /// Updates the system.
    /// \param time_info Time-related frame information.
    /// \return True if the system is still active, false otherwise.
    virtual bool update([[maybe_unused]] FrameTimeInfo const& time_info) { return true; }

    /// Destroys the system.
    virtual void destroy() {}

protected:
    std::vector<Entity*> entities{};
    Bitset accepted_components{};
    bool paused = false;

protected:
    System() = default;

    /// Adds the given component types as accepted by the current system.
    /// \tparam CompTs Types of the components to accept.
    template <typename... CompTs>
    void register_components()
    {
        (accepted_components.set_bit(Component::get_id<CompTs>()), ...);
    }

    /// Removes the given component types as accepted by the current system.
    /// \tparam CompTs Types of the components to deny.
    template <typename... CompTs>
    void unregister_components()
    {
        (accepted_components.set_bit(Component::get_id<CompTs>(), false), ...);
    }

    /// Links the entity to the system.
    /// \param entity Entity to be linked.
    virtual void link_entity(EntityPtr const& entity);

    /// Unlinks the entity from the system.
    /// \param entity Entity to be unlinked.
    virtual void unlink_entity(EntityPtr const& entity);

private:
    static inline size_t max_id = 0;
};
}

#include "system.inl"