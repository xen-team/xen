#pragma once

#include "input/input_button.hpp"

namespace xen {
class XEN_API MouseInputButton : public InputButton::Registrar<MouseInputButton> {
    inline static bool const registered = Register("mouse");

private:
    MouseButton button;

public:
    explicit MouseInputButton(MouseButton button = MouseButton::_1);

    [[nodiscard]] bool is_down() const override;

    [[nodiscard]] InputAxis::ArgumentDescription get_argument_desc() const override;

    [[nodiscard]] MouseButton get_button() const { return button; }
    void set_button(MouseButton button) { this->button = button; }

    friend void to_json(json& j, MouseInputButton const& p)
    {
        to_json(j["inverted"], p.inverted);
        to_json(j["button"], p.button);
    }

    friend void from_json(json const& j, MouseInputButton& p)
    {
        from_json(j["inverted"], p.inverted);
        from_json(j["button"], p.button);
    }
};
}