#include "trigger_system.hpp"

#include <entity.hpp>
#include <math/transform/transform.hpp>
#include <utils/trigger_volume.hpp>

namespace xen {
TriggerSystem::TriggerSystem()
{
    register_components<Triggerer, TriggerVolume>();
}

bool TriggerSystem::update(FrameTimeInfo const&)
{
    for (Entity const* triggerer_entity : entities) {
        if (!triggerer_entity->has_component<Triggerer>() || !triggerer_entity->has_component<Transform>()) {
            continue;
        }

        auto const& triggerer_transform = triggerer_entity->get_component<Transform>();

        for (Entity* trigger_volume_entity : entities) {
            if (!trigger_volume_entity->has_component<TriggerVolume>()) {
                continue;
            }

            auto& trigger_volume = trigger_volume_entity->get_component<TriggerVolume>();

            if (!trigger_volume.enabled) {
                continue;
            }

            process_trigger(trigger_volume, triggerer_transform);
        }
    }

    return true;
}

void TriggerSystem::process_trigger(TriggerVolume& trigger_volume, Transform const& triggerer_transform)
{
    bool const was_being_triggered = trigger_volume.currently_triggered;

    trigger_volume.currently_triggered = std::visit(
        [&triggerer_transform](auto const& volume) {
            // TODO: handle all transform info for both the triggerer & the volume
            return volume.contains(triggerer_transform.get_position());
        },
        trigger_volume.volume
    );

    if (!was_being_triggered && !trigger_volume.currently_triggered) {
        return;
    }

    std::function<void()> const& action =
        (!was_being_triggered && trigger_volume.currently_triggered ?
             trigger_volume.enter_action :
             (was_being_triggered && trigger_volume.currently_triggered ? trigger_volume.stay_action :
                                                                          trigger_volume.leave_action));

    if (action) {
        action();
    }
}
}