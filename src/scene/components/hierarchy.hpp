#pragma once

#include <scene/component.hpp>
#include <entt.hpp>

namespace xen {
struct HierarchyComponent : public Component {
    inline static bool const registered = register_component<HierarchyComponent>();

    std::size_t childs{};
    entt::entity first{entt::null};
    entt::entity prev{entt::null};
    entt::entity next{entt::null};
    entt::entity parent{entt::null};

    template <class Archive>
    void save(Archive& ar) const
    {
        ar(childs, first, prev, next, parent);
    }
    template <class Archive>
    void load(Archive& ar)
    {
        ar(childs, first, prev, next, parent);
    }
};
}