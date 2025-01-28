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

    void save(nlohmann::json& j) override;
    void load(nlohmann::json const& j) override;
};
}