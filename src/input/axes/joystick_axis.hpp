#pragma once

#include "input/joysticks.hpp"
#include "input/input_axis.hpp"

namespace xen {
class XEN_API JoystickInputAxis : public InputAxis::Registrar<JoystickInputAxis> {
    inline static bool const registered = Register("joystick");

private:
    Joystick* joystick;
    JoystickAxis axis;

public:
    explicit JoystickInputAxis(JoystickPort port = JoystickPort::_1, JoystickAxis axis = 0);

    [[nodiscard]] float get_amount() const override;

    [[nodiscard]] ArgumentDescription get_argument_desc() const override;

    [[nodiscard]] bool is_connected() const { return joystick->is_connected(); }

    [[nodiscard]] JoystickPort get_port() const { return joystick->get_port(); }
    void set_port(JoystickPort port);

    [[nodiscard]] JoystickAxis get_axis() const { return axis; }
    void set_axis(JoystickAxis axis) { this->axis = axis; }

    friend void to_json(json& j, JoystickInputAxis const& p)
    {
        to_json(j["scale"], p.scale);
        to_json(j["port"], p.get_port());
        to_json(j["axis"], p.axis);
    }

    friend void from_json(json const& j, JoystickInputAxis& p)
    {
        from_json(j["scale"], p.scale);
        p.set_port(j["port"].get<JoystickPort>());
        from_json(j["axis"], p.axis);
    }
};
}
