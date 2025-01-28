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

    void save(nlohmann::json& j) override;
    void load(nlohmann::json const& j) override;
};

using JoystickHatInputAxis = JoystickHatInput;
using JoystickHatInputButton = JoystickHatInput;
}
