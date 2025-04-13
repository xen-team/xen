#include "entity.hpp"

#include <scene/components/tag.hpp>
#include <scene/components/id.hpp>

namespace xen {
void Entity::set_parent(Entity parent)
{
    parent.add_child(*this);
}
void Entity::remove_parent()
{
    auto& hierarchy = get_component<HierarchyComponent>();
    auto managed_entity = Entity(hierarchy.parent, manager);
    managed_entity.remove_child(*this);
}

Entity Entity::get_parent()
{
    auto& hierarchy = get_component<HierarchyComponent>();
    return {(hierarchy.parent == entt::null ? entt::null : hierarchy.parent), manager};
}

void Entity::add_child(Entity entity)
{
    auto& hierarchy = manager->registry.get<HierarchyComponent>(entity);
    if (hierarchy.parent == this->handle) {
        Log::warning("Attempting to re-add child");
        return;
    }

    entity.remove_parent();
    hierarchy.parent = this->handle;

    auto& parent_hierarchy = get_component<HierarchyComponent>();
    if (parent_hierarchy.childs == 0) {
        parent_hierarchy.first = entity;
    }
    else {
        auto& last_child_hierarchy = manager->registry.get<HierarchyComponent>(parent_hierarchy.first);
        while (last_child_hierarchy.next != entt::null) {
            last_child_hierarchy = manager->registry.get<HierarchyComponent>(last_child_hierarchy.next);
        }
        last_child_hierarchy.next = entity;
    }

    parent_hierarchy.childs++;
}
void Entity::remove_child(Entity entity)
{
    auto& hierarchy = manager->registry.get<HierarchyComponent>(entity);
    if (hierarchy.parent != this->handle) {
        Log::warning("Entity is not a child of this entity");
        return;
    }

    auto& parent_hierarchy = get_component<HierarchyComponent>();
    if (parent_hierarchy.first == entity) {
        parent_hierarchy.first = hierarchy.next;
    }
    else {
        auto& current_child_hierarchy = manager->registry.get<HierarchyComponent>(parent_hierarchy.first);
        while (current_child_hierarchy.next != entt::null && current_child_hierarchy.next != entity) {
            current_child_hierarchy = manager->registry.get<HierarchyComponent>(current_child_hierarchy.next);
        }
        if (current_child_hierarchy.next == entity) {
            current_child_hierarchy.next = hierarchy.next;
        }
    }

    hierarchy.parent = entt::null;
    hierarchy.next = entt::null;
    hierarchy.prev = entt::null;

    parent_hierarchy.childs--;
}
bool Entity::have_child(Entity entity)
{
    auto& hierarchy = entity.get_component<HierarchyComponent>();
    return hierarchy.parent == this->handle;
}
size_t Entity::get_child_count() const
{
    auto& parent_hierarchy = get_component<HierarchyComponent>();
    return parent_hierarchy.childs;
}
void Entity::destroy()
{
    manager->registry.destroy(handle);
}
bool Entity::is_valid() const
{
    return manager && manager->registry.valid(handle);
}
UUID Entity::get_uuid() const
{
    return get_component<IDComponent>().id;
}
std::string Entity::get_name() const
{
    auto* name_component = try_get_component<TagComponent>();
    return name_component ? name_component->tag : "Unnamed2";
}
}