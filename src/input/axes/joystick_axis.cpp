#include "joystick_axis.hpp"

namespace xen {
JoystickInputAxis::JoystickInputAxis(JoystickPort port, JoystickAxis axis) : axis(axis)
{
    set_port(port);
}

float JoystickInputAxis::get_amount() const
{
    return (scale * joystick->get_axis(axis)) + offset;
}

void JoystickInputAxis::set_port(JoystickPort port)
{
    joystick = Joysticks::get()->get_joystick(port);
    disconnect_tracked_connections();
    joystick->on_axis.connect([this](JoystickAxis axis, [[maybe_unused]] float value) {
        if (this->axis == axis) {
            on_axis(get_amount());
        }
    });
}

InputAxis::ArgumentDescription JoystickInputAxis::get_argument_desc() const
{
    return {
        {.name = "scale", .type = "float", .description = "Output amount scalar"},
        {.name = "port", .type = "int", .description = "The joystick port name"},
        {.name = "axis", .type = "int", .description = "The axis on the joystick being checked"}
    };
}
}
