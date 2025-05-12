#pragma once

#include <component.hpp>
#include <utils/shape.hpp>

namespace xen {
/// Triggerer component, representing an entity that can interact with triggerable entities.
/// \see TriggerVolume
class Triggerer final : public Component {};

/// TriggerVolume component, holding a volume that can be triggered and actions that can be executed accordingly.
/// \see Triggerer, TriggerSystem
class TriggerVolume final : public Component {
    friend class TriggerSystem;

public:
    template <typename VolumeT>
    explicit TriggerVolume(VolumeT&& volume) : volume{std::forward<VolumeT>(volume)}
    {
        // TODO: the OBB's point containment check isn't implemented yet
        static_assert(std::is_same_v<std::decay_t<VolumeT>, AABB> || std::is_same_v<std::decay_t<VolumeT>, Sphere>);
    }

    void set_enter_action(std::function<void()> enter_action) { this->enter_action = std::move(enter_action); }

    void set_stay_action(std::function<void()> stay_action) { this->stay_action = std::move(stay_action); }

    void set_leave_action(std::function<void()> leave_action) { this->leave_action = std::move(leave_action); }

    /// Changes the trigger volume's state.
    /// \param enabled True if the trigger volume should be enabled (triggerable), false otherwise.
    void enable(bool enabled = true) { this->enabled = enabled; }

    /// Disables the trigger volume, making it non-triggerable.
    void disable() { enable(false); }

    void reset_enter_action() { set_enter_action(nullptr); }

    void reset_stay_action() { set_stay_action(nullptr); }

    void reset_leave_action() { set_leave_action(nullptr); }

private:
    bool enabled = true;

    std::variant<AABB, Sphere> volume;
    std::function<void()> enter_action;
    std::function<void()> stay_action;
    std::function<void()> leave_action;

    bool currently_triggered = false;
};
}