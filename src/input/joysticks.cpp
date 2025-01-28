#include "joysticks.hpp"

#include "debug/log.hpp"
#include "utils/enumerate.hpp"

#include <GLFW/glfw3.h>

namespace xen {
static_assert(
    GLFW_JOYSTICK_LAST == static_cast<int16_t>(JoystickPort::_16),
    "GLFW joystick port count does not match our joystick port enum count."
);

void callback_joystick(int32_t const id, int32_t const event)
{
    auto* joystick = Joysticks::get()->get_joystick(static_cast<JoystickPort>(id));
    switch (event) {
    case GLFW_CONNECTED: {
        Log::out("Joystick connected: '", glfwGetJoystickName(id), "' to ", id, '\n');
        joystick->connected = true;
        joystick->name = glfwGetJoystickName(id);
        joystick->on_connect(true);
        break;
    }
    case GLFW_DISCONNECTED: {
        Log::out("Joystick disconnected from ", id, '\n');
        joystick->connected = false;
        joystick->on_connect(false);
        break;
    }
    default:
        break;
    }
}
void Joystick::update_axes()
{
    int32_t axes_count = 0;
    float const* axes_ptr = glfwGetJoystickAxes(static_cast<int>(port), &axes_count);
    axes.resize(static_cast<std::size_t>(axes_count));

    for (int32_t i = 0; i < axes_count; i++) {
        if (axes[i] != axes_ptr[i]) {
            axes[i] = axes_ptr[i];
            on_axis(i, axes[i]);
        }
    }
}

void Joystick::update_buttons()
{
    int32_t buttons_count = 0;
    uint8_t const* buttons_ptr = glfwGetJoystickButtons(static_cast<int>(port), &buttons_count);
    buttons.resize(static_cast<std::size_t>(buttons_count));

    for (int32_t i = 0; i < buttons_count; i++) {
        if (buttons_ptr[i] != GLFW_RELEASE && buttons[i] != InputAction::Release) {
            buttons[i] = InputAction::Repeat;
        }
        else if (buttons[i] != static_cast<InputAction>(buttons_ptr[i])) {
            buttons[i] = static_cast<InputAction>(buttons_ptr[i]);
            on_button(i, buttons[i]);
        }
    }
}

void Joystick::update_hats()
{
    int32_t hats_count = 0;
    uint8_t const* hats_ptr = glfwGetJoystickHats(static_cast<int>(port), &hats_count);
    hats.resize(static_cast<std::size_t>(hats_count));

    for (int32_t i = 0; i < hats_count; i++) {
        if (hats[i] != JoystickHatValues(hats_ptr[i])) {
            hats[i] = JoystickHatValues(hats_ptr[i]);
            on_hat(i, hats[i]);
        }
    }
}

void Joystick::update()
{
    if (!connected) {
        return;
    }

    update_axes();
    update_buttons();
    update_hats();
}

float Joystick::get_axis(JoystickAxis axis) const
{
    return (connected && axis <= axes.size()) ? axes[axis] : 0.0f;
}

InputAction Joystick::get_button(JoystickButton button) const
{
    return (connected && button <= buttons.size()) ? buttons[button] : InputAction::Release;
}

JoystickHatValues Joystick::get_hat(JoystickHat hat) const
{
    return (connected && hat <= hats.size()) ? hats[hat] : JoystickHatValue::Centered;
}

Joysticks::Joysticks()
{
    glfwSetJoystickCallback(callback_joystick);

    for (auto [port, joystick] : enumerate(joysticks)) {
        if (glfwJoystickPresent(port)) {
            joystick.port = static_cast<JoystickPort>(port);
            joystick.name = glfwGetJoystickName(port);
            joystick.on_connect(true);
        }
    }
}

void Joysticks::update()
{
    for (auto& joystick : joysticks) {
        joystick.update();
    }
}

Joystick const* Joysticks::get_joystick(JoystickPort port) const
{
    return &joysticks[static_cast<std::size_t>(port)];
}

Joystick* Joysticks::get_joystick(JoystickPort port)
{
    return &joysticks[static_cast<std::size_t>(port)];
}
}