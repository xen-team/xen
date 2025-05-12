#pragma once

#include <component.hpp>
#include <data/bitset.hpp>

namespace xen {
class Entity;
using EntityPtr = std::unique_ptr<Entity>;

/// Entity class representing an aggregate of Component objects.
class Entity {
public:
    explicit Entity(size_t index, bool enabled = true) : id{index}, enabled{enabled} {}

    Entity(Entity const&) = delete;
    Entity(Entity&&) = delete;
    Entity& operator=(Entity const&) = delete;
    Entity& operator=(Entity&&) = delete;

    size_t get_id() const { return id; }

    bool is_enabled() const { return enabled; }

    std::vector<ComponentPtr> const& get_components() const { return components; }

    Bitset const& get_enabled_components() const { return enabled_components; }

    template <typename... Args>
    static EntityPtr create(Args&&... args)
    {
        return std::make_unique<Entity>(std::forward<Args>(args)...);
    }

    /// Changes the entity's enabled state.
    /// Enables or disables the entity according to the given parameter.
    /// \param enabled True if the entity should be enabled, false if it should be disabled.
    void enable(bool enabled = true) { this->enabled = enabled; }

    /// Disables the entity.
    void disable() { enable(false); }

    /// Adds a component to be held by the entity.
    /// \tparam CompT Type of the component to be added.
    /// \tparam Args Types of the arguments to be forwarded to the given component.
    /// \param args Arguments to be forwarded to the given component.
    /// \return Reference to the newly added component.
    template <typename CompT, typename... Args>
    CompT& add_component(Args&&... args);

    /// Adds a last component to be held by the entity.
    /// \tparam CompT Type of the last component to be added.
    /// \return A tuple containing a reference to the last newly added component.
    template <typename CompT>
    std::tuple<CompT&> add_components();

    /// Adds several components at once to be held by the entity.
    /// \tparam Comp1 Type of the first component to be added.
    /// \tparam Comp2 Type of the second component to be added.
    /// \tparam C Types of the other components to be added.
    /// \return A tuple containing references to all the newly added components.
    template <typename Comp1, typename Comp2, typename... C>
    std::tuple<Comp1&, Comp2&, C...> add_components();

    /// Tells if a given component is held by the entity.
    /// \tparam CompT Type of the component to be checked.
    /// \return True if the entity holds the given component, false otherwise.
    template <typename CompT>
    bool has_component() const;

    /// Gets a given component held by the entity.
    /// The entity must have this component. If not, an exception is thrown.
    /// \tparam CompT Type of the component to be fetched.
    /// \return Reference to the found component.
    template <typename CompT>
    CompT const& get_component() const;

    /// Gets a given component held by the entity.
    /// The entity must have this component. If not, an exception is thrown.
    /// \tparam CompT Type of the component to be fetched.
    /// \return Reference to the found component.
    template <typename CompT>
    CompT& get_component()
    {
        return const_cast<CompT&>(static_cast<Entity const*>(this)->get_component<CompT>());
    }

    /// Removes the given component from the entity.
    /// \tparam CompT Type of the component to be removed.
    template <typename CompT>
    void remove_component();

private:
    size_t id{};
    bool enabled{};
    std::vector<ComponentPtr> components{};
    Bitset enabled_components{};
};
}

#include "entity.inl"