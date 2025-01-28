#include "joystick_button.hpp"

namespace xen {
JoystickInputButton::JoystickInputButton(JoystickPort port, JoystickButton button) : button(button)
{
    set_port(port);
}

bool JoystickInputButton::is_down() const
{
    return (joystick->get_button(button) != InputAction::Release) ^ inverted;
}

void JoystickInputButton::set_port(JoystickPort port)
{
    joystick = Joysticks::get()->get_joystick(port);
    disconnect_tracked_connections();
    joystick->on_button.connect([this](JoystickButton button, InputAction action) {
        if (this->button == button) {
            on_button(action, 0);
        }
    });
}

InputAxis::ArgumentDescription JoystickInputButton::get_argument_desc() const
{
    return {
        {.name = "inverted", .type = "bool", .description = "If the down reading will be inverted"},
        {.name = "port", .type = "int", .description = "The joystick port name"},
        {.name = "axis", .type = "bool", .description = "The button on the joystick being checked"}
    };
}

void JoystickInputButton::save(nlohmann::json& j)
{
    save_base(j);
    save_value(j, "port", get_port());
    save_value(j, "button", button);
}
void JoystickInputButton::load(nlohmann::json const& j)
{
    load_base(j);
    set_port(get_value<JoystickPort>(j, "port"));
    load_value(j, "button", button);
}
}