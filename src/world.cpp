#include "world.hpp"

#include <tracy/Tracy.hpp>

namespace xen {
Entity& World::add_entity(bool enabled)
{
    entities.emplace_back(Entity::create(max_entity_index++, enabled));
    active_entity_count += enabled;

    return *entities.back();
}

void World::remove_entity(Entity const& entity)
{
    auto iter = std::ranges::find_if(entities, [&entity](EntityPtr const& entity_ptr) {
        return (&entity == entity_ptr.get());
    });

    if (iter == entities.end()) {
        throw std::invalid_argument("Error: The entity isn't owned by this world");
    }

    for (SystemPtr const& system : systems) {
        system->unlink_entity(*iter);
    }

    entities.erase(iter);
}

bool World::update(FrameTimeInfo const& time_info)
{
    ZoneScopedN("World::update");

    refresh();

    for (size_t system_index = 0; system_index < systems.size(); ++system_index) {
        if (!active_systems[system_index]) {
            continue;
        }

        bool const system_active = systems[system_index]->update(time_info);

        if (!system_active) {
            active_systems.set_bit(system_index, false);
        }
    }

    return !active_systems.empty();
}

void World::refresh()
{
    ZoneScopedN("World::refresh");

    if (entities.empty()) {
        return;
    }

    sort_entities();

    for (size_t entity_index = 0; entity_index < active_entity_count; ++entity_index) {
        EntityPtr const& entity = entities[entity_index];

        if (!entity->is_enabled()) {
            continue;
        }

        for (size_t system_index = 0; system_index < systems.size(); ++system_index) {
            SystemPtr const& system = systems[system_index];

            if (system == nullptr || !active_systems[system_index]) {
                continue;
            }

            Bitset const matching_components = system->get_accepted_components() & entity->get_enabled_components();

            // If the system does not contain the entity, check if it should (if it possesses the accepted components);
            // if yes, link it Else, if the system contains the entity but should not, unlink it
            if (!system->contains_entity(*entity)) {
                if (!matching_components.empty()) {
                    system->link_entity(entity);
                }
            }
            else {
                if (matching_components.empty()) {
                    system->unlink_entity(entity);
                }
            }
        }
    }
}

void World::destroy()
{
    ZoneScopedN("World::destroy");

    // Entities must be released before the systems, since their destruction may depend on those
    entities.clear();
    active_entity_count = 0;
    max_entity_index = 0;

    // This means that no entity must be used in any system destructor, since they will all be invalid
    // Their list is thus cleared to avoid any invalid usage
    for (SystemPtr const& system : systems) {
        if (system) {
            system->entities.clear();
        }
    }

    systems.clear();
    active_systems.clear();
}

void World::sort_entities()
{
    ZoneScopedN("World::sort_entities");

    // Reorganizing the entites, swapping enabled & disabled ones so that the enabled ones are in front
    auto first_entity = entities.begin();
    auto last_entity = entities.end() - 1;

    while (first_entity != last_entity) {
        // Iterating from the beginning to the end, trying to find a disabled entity
        if ((*first_entity)->is_enabled()) {
            ++first_entity;
            continue;
        }

        // Iterating from the end to the beginning, trying to find an enabled entity
        while (first_entity != last_entity && (*last_entity == nullptr || !(*last_entity)->is_enabled())) {
            --last_entity;
        }

        // If both iterators are equal to each other, the list is sorted
        if (first_entity == last_entity) {
            break;
        }

        std::swap(*first_entity, *last_entity);
        --last_entity;
    }

    active_entity_count = static_cast<size_t>(std::distance(entities.begin(), last_entity) + 1);
}
}