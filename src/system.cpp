#include "system.hpp"

namespace xen {
bool System::contains_entity(Entity const& entity) const
{
    for (Entity const* entity_ptr : entities) {
        if (entity_ptr->get_id() == entity.get_id()) {
            return true;
        }
    }

    return false;
}

void System::link_entity(EntityPtr const& entity)
{
    entities.emplace_back(entity.get());
}

void System::unlink_entity(EntityPtr const& entity)
{
    for (size_t entity_index = 0; entity_index < entities.size(); ++entity_index) {
        if (entities[entity_index]->get_id() == entity->get_id()) {
            entities.erase(entities.begin() + static_cast<std::ptrdiff_t>(entity_index));
            break;
        }
    }
}
}