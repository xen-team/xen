#pragma once

#include <system.hpp>

namespace xen {
class ScriptSystem final : public System {
public:
    /// Default constructor.
    ScriptSystem();

    bool update(FrameTimeInfo const& time_info) override;

private:
    /// Links the entity to the system, registering the entity to the script and calling the script's setup function.
    /// \param entity Entity to be linked.
    void link_entity(EntityPtr const& entity) override;
};
}