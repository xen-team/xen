#pragma once

#include <system.hpp>

namespace xen {
class Transform;
class TriggerVolume;

class TriggerSystem final : public System {
public:
    TriggerSystem();

    bool update(FrameTimeInfo const& time_info) override;

private:
    static void process_trigger(TriggerVolume& trigger_volume, Transform const& triggerer_transform);
};
}