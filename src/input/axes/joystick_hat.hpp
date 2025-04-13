#pragma once

#include "input/joysticks.hpp"
#include "input/input_button.hpp"
#include "input/input_axis.hpp"

namespace xen {
class XEN_API JoystickHatInput :
    public InputAxis::Registrar<JoystickHatInput>,
    public InputButton::Registrar<JoystickHatInput> {
    inline static bool const registered = InputAxis::Registrar<JoystickHatInput>::Register("joystick_hat") &&
                                          InputButton::Registrar<JoystickHatInput>::Register("joystick_hat");

private:
    Joystick* joystick;
    JoystickHat hat;
    JoystickHatValues hat_flags;
    bool last_down = false;

public:
    explicit JoystickHatInput(
        JoystickPort port = JoystickPort::_1, JoystickHat hat = 0,
        JoystickHatValues const& hat_flags = JoystickHatValue::Centered
    );

    [[nodiscard]] ArgumentDescription get_argument_desc() const override;

    [[nodiscard]] float get_amount() const override;
    [[nodiscard]] bool is_down() const override;

    [[nodiscard]] bool is_connected() const { return joystick->is_connected(); }

    [[nodiscard]] JoystickPort get_port() const { return joystick->get_port(); }
    void set_port(JoystickPort port);

    [[nodiscard]] JoystickHat get_hat() const { return hat; }
    void set_hat(JoystickHat hat) { this->hat = hat; }

    [[nodiscard]] JoystickHatValues const& get_hat_flags() const { return hat_flags; }
    void set_hat_flags(JoystickHatValues hat_flags) { this->hat_flags = hat_flags; }

    friend void to_json(json& j, JoystickHatInput const& p)
    {
        to_json(j["scale"], p.scale);
        to_json(j["inverted"], p.inverted);
        to_json(j["port"], p.get_port());
        to_json(j["hat"], p.hat);
        to_json(j["hat_flags"], p.hat_flags);
    }

    friend void from_json(json const& j, JoystickHatInput& p)
    {
        from_json(j["scale"], p.scale);
        from_json(j["inverted"], p.inverted);
        p.set_port(j["port"].get<JoystickPort>());
        from_json(j["hat"], p.hat);
        from_json(j["hat_flags"], p.hat_flags);
    }
};

using JoystickHatInputAxis = JoystickHatInput;
using JoystickHatInputButton = JoystickHatInput;
}
