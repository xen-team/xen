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

    void save(nlohmann::json& j) override;
    void load(nlohmann::json const& j) override;
};
}
