#include "entity.hpp"
#include "world.hpp"

namespace xen {
void Entity::destroy()
{
    linked_world.remove_entity(*this);
}
}