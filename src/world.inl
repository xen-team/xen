namespace xen {
template <typename SysT, typename... Args>
SysT& World::add_system(Args&&... args)
{
    static_assert(std::is_base_of_v<System, SysT>, "Error: The added system must be derived from System.");

    size_t const system_id = System::get_id<SysT>();

    if (system_id >= systems.size()) {
        systems.resize(system_id + 1);
    }

    systems[system_id] = std::make_unique<SysT>(std::forward<Args>(args)...);
    active_systems.set_bit(system_id);

    return static_cast<SysT&>(*systems[system_id]);
}

template <typename SysT>
bool World::has_system() const
{
    static_assert(std::is_base_of_v<System, SysT>, "Error: The checked system must be derived from System.");

    size_t const system_id = System::get_id<SysT>();
    return ((system_id < systems.size()) && systems[system_id]);
}

template <typename SysT>
SysT const& World::get_system() const
{
    static_assert(std::is_base_of_v<System, SysT>, "Error: The fetched system must be derived from System.");

    if (has_system<SysT>()) {
        return static_cast<SysT const&>(*systems[System::get_id<SysT>()]);
    }

    throw std::runtime_error("Error: No system available of specified type");
}

template <typename SysT>
void World::remove_system()
{
    static_assert(std::is_base_of_v<System, SysT>, "Error: The removed system must be derived from System.");

    if (has_system<SysT>()) {
        systems[System::get_id<SysT>()].reset();
    }
}

template <typename CompT, typename... Args>
Entity& World::add_entity_with_component(Args&&... args)
{
    Entity& entity = add_entity();
    entity.add_component<CompT>(std::forward<Args>(args)...);

    return entity;
}

template <typename... CompsTs>
Entity& World::add_entity_with_components(bool enabled)
{
    Entity& entity = add_entity(enabled);
    entity.add_components<CompsTs...>();

    return entity;
}

template <typename... CompsTs>
std::vector<Entity*> World::recover_entities_with_components()
{
    static_assert(
        (std::is_base_of_v<Component, CompsTs> && ...),
        "Error: The components to query the entity with must all be derived from Component."
    );

    std::vector<Entity*> entities;

    for (EntityPtr const& entity : entities) {
        if ((entity->has_component<CompsTs>() && ...)) {
            entities.emplace_back(entity.get());
        }
    }

    return entities;
}
}