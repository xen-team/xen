#include "joystick_hat.hpp"

namespace xen {
JoystickHatInput::JoystickHatInput(JoystickPort port, JoystickHat hat, JoystickHatValues const& hat_flags) :
    hat(hat), hat_flags(hat_flags)
{
    set_port(port);
}

InputAxis::ArgumentDescription JoystickHatInput::get_argument_desc() const
{
    return {
        {.name = "scale", .type = "float", .description = "Output amount scalar"},
        {.name = "inverted", .type = "bool", .description = "If the down reading will be inverted"},
        {.name = "axis", .type = "axis", .description = "The axis to sample"},
        {.name = "min", .type = "float", .description = "Lower axis value bound"},
        {.name = "max", .type = "float", .description = "Upper axis value bound"}
    };
}

float JoystickHatInput::get_amount() const
{
    constexpr std::array<float, 9> const hat_values = {
        0.0f,   // None
        1.0f,   // Up
        0.5f,   // Down
        0.25f,  // Right
        0.75f,  // Left
        0.125f, // Up | Right
        0.875f, // Up | Left
        0.375f, // Down | Right
        0.625f, // Down | Left
    };

    JoystickHatValues const hat_value = joystick->get_hat(hat);
    return (scale * hat_values[hat_value]) + offset;
}

bool JoystickHatInput::is_down() const
{
    return (joystick->get_hat(hat) & hat_flags) ^ inverted;
}

void JoystickHatInput::set_port(JoystickPort port)
{
    joystick = Joysticks::get()->get_joystick(port);
    disconnect_tracked_connections();
    joystick->on_hat.connect([this](JoystickHat hat, [[maybe_unused]] JoystickHatValues value) {
        if (this->hat == hat) {
            on_axis(get_amount());
            bool const down = is_down();

            if (down != last_down) {
                last_down = down;
                on_button(down ? InputAction::Press : InputAction::Release, 0);
            }
            else if (down) {
                on_button(InputAction::Repeat, 0);
            }
        }
    });
}
}
