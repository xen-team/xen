#pragma once

#include <scene/component.hpp>
#include <cereal/types/string.hpp>

namespace xen {
struct TagComponent : public Component {
    inline static bool const registered = register_component<TagComponent>();

    std::string tag = "Unnamed";

    TagComponent() = default;
    TagComponent(std::string tag) : tag{tag} {};
    ~TagComponent() = default;

    template <class Archive>
    void save(Archive& ar) const
    {
        // ar(tag);
    }
    template <class Archive>
    void load(Archive& ar)
    {
        // ar(tag);
    }
};
}