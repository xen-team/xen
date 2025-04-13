#pragma once

#include <scene/component.hpp>
#include <utils/uuid.hpp>

namespace xen {
struct IDComponent : public Component {
    inline static bool const registered = register_component<IDComponent>();

    UUID id;

    template <class Archive>
    void save(Archive& ar) const
    {
        ar(id);
    }
    template <class Archive>
    void load(Archive& ar)
    {
        ar(id);
    }
};
}
