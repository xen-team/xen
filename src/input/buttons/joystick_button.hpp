#pragma once

#include "input/joysticks.hpp"
#include "input/input_button.hpp"

namespace xen {
class XEN_API JoystickInputButton : public InputButton::Registrar<JoystickInputButton> {
    inline static bool const registered = Register("joystick");

private:
    Joystick* joystick;
    JoystickButton button;

public:
    explicit JoystickInputButton(JoystickPort port = JoystickPort::_1, JoystickButton button = 0);

    [[nodiscard]] bool is_down() const override;

    [[nodiscard]] InputAxis::ArgumentDescription get_argument_desc() const override;

    [[nodiscard]] bool is_connected() const { return joystick->is_connected(); }

    [[nodiscard]] JoystickPort get_port() const { return joystick->get_port(); }
    void set_port(JoystickPort port);

    [[nodiscard]] JoystickButton get_button() const { return button; }
    void set_button(JoystickButton button) { this->button = button; }

    friend void to_json(json& j, JoystickInputButton const& p)
    {
        to_json(j["inverted"], p.inverted);
        to_json(j["port"], p.get_port());
        to_json(j["button"], p.button);
    }

    friend void from_json(json const& j, JoystickInputButton& p)
    {
        from_json(j["inverted"], p.inverted);
        p.set_port(j["port"].get<JoystickPort>());
        from_json(j["button"], p.button);
    }
};
}