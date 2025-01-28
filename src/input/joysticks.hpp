#pragma once

#include "system/windows.hpp"

#include <magic_enum.hpp>

namespace xen {
enum JoystickHatValue : uint8_t { Centered = 0, Up = 1 << 0, Right = 1 << 1, Down = 1 << 2, Left = 1 << 3 };
using JoystickHatValues = uint8_t;

enum class JoystickPort : uint8_t {
    _1 = 0,
    _2 = 1,
    _3 = 2,
    _4 = 3,
    _5 = 4,
    _6 = 5,
    _7 = 6,
    _8 = 7,
    _9 = 8,
    _10 = 9,
    _11 = 10,
    _12 = 11,
    _13 = 12,
    _14 = 13,
    _15 = 14,
    _16 = 15
};

using JoystickAxis = uint8_t;
using JoystickButton = uint8_t;
using JoystickHat = uint8_t;

class XEN_API Joystick {
    friend class Joysticks;

private:
    friend void callback_joystick(int32_t id, int32_t event);

    JoystickPort port;
    bool connected = false;
    std::string name;
    std::vector<float> axes;
    std::vector<InputAction> buttons;
    std::vector<JoystickHatValues> hats;

public:
    rocket::signal<void(bool)> on_connect;
    rocket::signal<void(uint8_t, InputAction)> on_button;
    rocket::signal<void(uint8_t, float)> on_axis;
    rocket::signal<void(uint8_t, JoystickHatValues)> on_hat;

public:
    void update();

    [[nodiscard]] JoystickPort get_port() const { return port; }

    bool is_connected() const { return connected; }

    [[nodiscard]] std::string get_name() const { return name; }

    [[nodiscard]] std::size_t get_axis_count() const { return axes.size(); }

    [[nodiscard]] std::size_t get_button_count() const { return buttons.size(); }

    [[nodiscard]] std::size_t get_hat_count() const { return hats.size(); }

    [[nodiscard]] float get_axis(uint8_t axis) const;

    [[nodiscard]] InputAction get_button(uint8_t button) const;

    [[nodiscard]] JoystickHatValues get_hat(uint8_t hat) const;

private:
    void update_axes();
    void update_buttons();
    void update_hats();
};

class XEN_API Joysticks : public Module::Registrar<Joysticks> {
    inline static bool const registered = Register(Stage::Pre, Depends<Windows>());

private:
    std::array<Joystick, magic_enum::enum_count<JoystickPort>()> joysticks;

public:
    Joysticks();

    void update() override;

    [[nodiscard]] Joystick const* get_joystick(JoystickPort port) const;
    [[nodiscard]] Joystick* get_joystick(JoystickPort port);
};
}