#pragma once

#include <entity.hpp>
#include <system.hpp>

namespace xen {
struct FrameTimeInfo;
class World;
using WorldPtr = std::unique_ptr<World>;

/// World class handling systems & entities.
class World {
public:
    explicit World(size_t entity_count) { entities.reserve(entity_count); }

    World() = default;
    World(World const&) = delete;
    World(World&&) noexcept = default;
    World& operator=(World const&) = delete;
    World& operator=(World&&) noexcept = default;

    [[nodiscard]] static World& get()
    {
        static World world;
        return world;
    }

    ~World() { destroy(); }

    std::vector<SystemPtr> const& get_systems() const { return systems; }

    std::vector<EntityPtr> const& get_entities() const { return entities; }

    /// Adds a given system to the world.
    /// \tparam SysT Type of the system to be added.
    /// \tparam Args Types of the arguments to be forwarded to the given system.
    /// \param args Arguments to be forwarded to the given system.
    /// \return Reference to the newly added system.
    template <typename SysT, typename... Args>
    SysT& add_system(Args&&... args);

    /// Tells if a given system exists within the world.
    /// \tparam SysT Type of the system to be checked.
    /// \return True if the given system is present, false otherwise.
    template <typename SysT>
    bool has_system() const;

    /// Gets a given system contained by the world.
    /// This system must be present within the world. If not, an exception is thrown.
    /// \tparam SysT Type of the system to be fetched.
    /// \return Constant reference to the found system.
    template <typename SysT>
    SysT const& get_system() const;

    /// Gets a given system contained by the world.
    /// This system must be present within the world. If not, an exception is thrown.
    /// \tparam SysT Type of the system to be fetched.
    /// \return Reference to the found system.
    template <typename SysT>
    SysT& get_system()
    {
        return const_cast<SysT&>(static_cast<World const*>(this)->get_system<SysT>());
    }

    /// Removes the given system from the world.
    /// \tparam SysT Type of the system to be removed.
    template <typename SysT>
    void remove_system();

    /// Adds an entity into the world.
    /// \param enabled True if the entity should be active immediately, false otherwise.
    /// \return Reference to the newly created entity.
    Entity& add_entity(bool enabled = true);

    /// Adds an entity into the world with a given component. This entity will be automatically enabled.
    /// \tparam CompT Type of the component to be added into the entity.
    /// \tparam Args Types of the arguments to be forwarded to the given component.
    /// \param args Arguments to be forwarded to the given component.
    /// \return Reference to the newly added entity.
    template <typename CompT, typename... Args>
    Entity& add_entity_with_component(Args&&... args);

    /// Adds an entity into the world with several components at once.
    /// \tparam CompsTs Types of the components to be added into the entity.
    /// \param enabled True if the entity should be active immediately, false otherwise.
    /// \return Reference to the newly added entity.
    template <typename... CompsTs>
    Entity& add_entity_with_components(bool enabled = true);

    void set_player(Entity& player) { this->player = &player; }
    Entity& get_player() { return *player; }

    /// Fetches entities which contain specific component(s).
    /// \tparam CompsTs Types of the components to query.
    /// \return List of entities containing all given components.
    template <typename... CompsTs>
    std::vector<Entity*> recover_entities_with_components();

    /// Removes an entity from the world. It *must* be an entity created by this world.
    /// \param entity Entity to be removed.
    void remove_entity(Entity const& entity);

    /// Updates the world, updating all the systems it contains.
    /// \param time_info Time-related frame information.
    /// \return True if the world still has active systems, false otherwise.
    bool update(FrameTimeInfo const& time_info);

    /// Refreshes the world, optimizing the entities & linking/unlinking entities to systems if needed.
    void refresh();

    /// Destroys the world, releasing all its entities & systems.
    void destroy();

private:
    std::vector<SystemPtr> systems{};
    Bitset active_systems{};

    std::vector<EntityPtr> entities{};
    size_t active_entity_count = 0;
    size_t max_entity_index = 0;

    Entity* player;

private:
    /// Sorts entities so that the disabled ones are packed to the end of the list.
    void sort_entities();
};
}

#include "world.inl"