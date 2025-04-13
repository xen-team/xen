#pragma once

#include <scene/entity_manager.hpp>
#include <scene/component.hpp>
#include <scene/components/hierarchy.hpp>
#include <scene/components/tag.hpp>
#include <debug/log.hpp>
#include <utils/uuid.hpp>
#include <entt.hpp>
#include <ranges>
#include <cereal/cereal.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/memory.hpp>

namespace xen {
class Entity {
private:
    EntityManager* manager = nullptr;
    entt::entity handle{entt::null};

    class HierarchyIterator {
    public:
        using value_type = Entity;
        using pointer = Entity*;
        using reference = Entity&;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::forward_iterator_tag;

        HierarchyIterator(EntityManager* manager, entt::entity entity) : manager(manager), entity(entity) {}

        Entity operator*() const { return {entity, manager}; }

        HierarchyIterator& operator++()
        {
            auto hierarchy = manager->registry.get<HierarchyComponent>(entity);
            if (hierarchy.next != entt::null) {
                entity = hierarchy.next;
            }
            else {
                entity = entt::null;
            }
            return *this;
        }

        bool operator==(HierarchyIterator const& other) const { return entity == other.entity; }

        bool operator!=(HierarchyIterator const& other) const { return !(*this == other); }

        HierarchyIterator begin() { return {manager, entity}; }

        HierarchyIterator end() { return {manager, entt::null}; }

    private:
        EntityManager* manager = nullptr;
        entt::entity entity = entt::null;
    };

    class RecursiveHierarchyIterator {
    public:
        using value_type = Entity;
        using pointer = Entity*;
        using reference = Entity&;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::forward_iterator_tag;

        RecursiveHierarchyIterator(EntityManager* manager, entt::entity root) : manager(manager)
        {
            if (root != entt::null) {
                stack.push_back(root);
            }
        }

        RecursiveHierarchyIterator(EntityManager* manager) : manager(manager) {}

        value_type operator*() const { return {stack.back(), manager}; }

        RecursiveHierarchyIterator& operator++()
        {
            if (stack.empty()) {
                return *this;
            }

            auto const current = stack.back();
            stack.pop_back();

            auto const hierarchy = manager->registry.get<HierarchyComponent>(current);

            if (hierarchy.first != entt::null) {
                std::vector<entt::entity> children;
                for (entt::entity child = hierarchy.first; child != entt::null;) {
                    children.push_back(child);

                    auto& childHierarchy = manager->registry.get<HierarchyComponent>(child);
                    child = childHierarchy.next;
                }

                for (auto& it : std::ranges::reverse_view(children)) {
                    stack.push_back(it);
                }
            }

            return *this;
        }

        RecursiveHierarchyIterator operator++(int)
        {
            RecursiveHierarchyIterator tmp = *this;
            ++(*this);
            return tmp;
        }

        bool operator==(RecursiveHierarchyIterator const& other) const
        {
            if (stack.empty() && other.stack.empty()) {
                return true;
            }
            if (stack.empty() || other.stack.empty()) {
                return false;
            }
            return stack.back() == other.stack.back();
        }

        bool operator!=(RecursiveHierarchyIterator const& other) const { return !(*this == other); }

        RecursiveHierarchyIterator begin() { return {manager, stack.front()}; }

        RecursiveHierarchyIterator end() { return {manager, entt::null}; }

    private:
        EntityManager* manager = nullptr;
        std::vector<entt::entity> stack;
    };

public:
    Entity() = default;
    Entity(entt::null_t) : handle{entt::null} {};
    Entity(entt::entity entity, EntityManager* manager) : manager{manager}, handle{entity} {};

    void update();

    template <typename Component>
    [[nodiscard]] bool has_component()
    {
        return manager->registry.all_of<Component>(handle);
    }

    template <typename Component>
    [[nodiscard]] Component& get_component() const
    {
        return manager->registry.get<Component>(handle);
    }

    template <typename Component>
    [[nodiscard]] Component* try_get_component() const
    {
        return manager->registry.try_get<Component>(handle);
    }

    template <typename Component, typename... Args>
    Component& add_component(Args&&... args)
    {
        if (has_component<Component>()) {
            Log::warning("Attempting to add component twice");
        }

        return manager->registry.emplace<Component>(handle, std::forward<Args>(args)...);
    }

    template <typename Component, typename... Args>
    Component& get_or_add_component(Args&&... args)
    {
        return manager->registry.get_or_emplace<Component>(handle, std::forward<Args>(args)...);
    }

    template <typename Component, typename... Args>
    Component& add_or_replace(Args&&... args)
    {
        return manager->registry.emplace_or_replace<Component>(handle, std::forward<Args>(args)...);
    }

    template <typename Component>
    void remove_component()
    {
        return manager->registry.remove<Component>(handle);
    }

    template <typename Component>
    void try_remove_component()
    {
        if (has_component<Component>()) {
            manager->registry.remove<Component>(handle);
        }
    }

    void set_parent(Entity parent);
    void remove_parent();
    [[nodiscard]] Entity get_parent();

    void add_child(Entity entity);
    void remove_child(Entity entity);
    [[nodiscard]] bool have_child(Entity entity);
    [[nodiscard]] size_t get_child_count() const;

    [[nodiscard]] HierarchyIterator hierarchy()
    {
        auto& hierarchy = manager->registry.get<HierarchyComponent>(handle);
        if (hierarchy.first != entt::null) {
            return {manager, hierarchy.first};
        }

        return HierarchyIterator{nullptr, entt::null};
    }

    [[nodiscard]] RecursiveHierarchyIterator recursive_hierarchy() { return {manager, handle}; }

    [[nodiscard]] HierarchyIterator hierarchy() const
    {
        auto& hierarchy = manager->registry.get<HierarchyComponent>(handle);
        if (hierarchy.first != entt::null) {
            return {manager, hierarchy.first};
        }

        return HierarchyIterator{nullptr, entt::null};
    }

    [[nodiscard]] RecursiveHierarchyIterator recursive_hierarchy() const { return {manager, handle}; }

    void destroy();
    [[nodiscard]] bool is_valid() const;

    [[nodiscard]] EntityManager* get_manager() { return manager; }
    [[nodiscard]] UUID get_uuid() const;
    [[nodiscard]] std::string get_name() const;

    operator bool() const { return handle != entt::null; }
    operator entt::entity() const { return handle; }
    operator uint32_t() const { return (uint32_t)handle; }

    bool operator==(Entity const& other) const { return handle == other.handle; }
    bool operator!=(Entity const& other) const { return handle != other.handle; }

    template <class Archive>
    friend void save(Archive& archive, xen::Entity const& ent)
    {
        archive(static_cast<entt::id_type>(ent.handle));
    }

    template <class Archive>
    friend void load(Archive& archive, xen::Entity& ent)
    {
        entt::id_type e;
        archive(e);
        ent.handle = static_cast<entt::entity>(e);
    }
};
}