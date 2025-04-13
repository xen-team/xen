#pragma once

#include <scene/component.hpp>
#include <utils/uuid.hpp>

namespace xen {
struct FpsPlayer : public Component {
    inline static bool const registered = register_component<FpsPlayer>();

    bool noclip = false;

    FpsPlayer() = default;

    void start() override;
    void update() override;

    template <class Archive>
    void save(Archive& ar) const
    {
        ar(noclip);
    }
    template <class Archive>
    void load(Archive& ar)
    {
        ar(noclip);
    }
};
}
