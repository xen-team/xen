#pragma once
#include "physics/collision_object.hpp"

#include "entity.hpp"

namespace xen {
class CollisionObject;

template <typename CompT, typename... Args>
CompT& Entity::add_component(Args&&... args)
{

    static_assert(std::is_base_of_v<Component, CompT>, "Error: The added component must be derived from Component.");

    size_t const comp_id = Component::get_id<CompT>();

    if (comp_id >= components.size()) {
        components.resize(comp_id + 1);
    }

    components[comp_id] = std::make_unique<CompT>(std::forward<Args>(args)...);
    enabled_components.set_bit(comp_id);

    auto& new_comp_ref = static_cast<CompT&>(*components[comp_id]);

    if constexpr (std::is_base_of_v<CollisionObject, CompT>) {
        new_comp_ref.set_entity_owner(this);
    }

    return new_comp_ref;
}

template <typename CompT>
std::tuple<CompT&> Entity::add_components()
{
    static_assert(std::is_base_of_v<Component, CompT>, "Error: The added component must be derived from Component.");

    return std::forward_as_tuple(add_component<CompT>());
}

template <typename CompT1, typename CompT2, typename... C>
std::tuple<CompT1&, CompT2&, C...> Entity::add_components()
{
    static_assert(std::is_base_of_v<Component, CompT1>, "Error: The added component must be derived from Component.");

    return std::tuple_cat(std::forward_as_tuple(add_component<CompT1>()), add_components<CompT2, C...>());
}

template <typename CompT>
bool Entity::has_component() const
{
    static_assert(std::is_base_of_v<Component, CompT>, "Error: The checked component must be derived from Component.");

    size_t const comp_id = Component::get_id<CompT>();
    return ((comp_id < components.size()) && enabled_components[comp_id]);
}

template <typename CompT>
CompT const& Entity::get_component() const
{
    static_assert(std::is_base_of_v<Component, CompT>, "Error: The fetched component must be derived from Component.");

    if (has_component<CompT>()) {
        return static_cast<CompT const&>(*components[Component::get_id<CompT>()]);
    }

    throw std::runtime_error("Error: No component available of specified type");
}

template <typename CompT>
void Entity::remove_component()
{
    static_assert(std::is_base_of_v<Component, CompT>, "Error: The removed component must be derived from Component.");

    if (has_component<CompT>()) {
        size_t const comp_id = Component::get_id<CompT>();

        components[comp_id].reset();
        enabled_components.set_bit(comp_id, false);
    }
}
}