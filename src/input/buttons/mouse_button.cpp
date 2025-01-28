#include "mouse_button.hpp"
#include "system/windows.hpp"

namespace xen {
MouseInputButton::MouseInputButton(MouseButton button) : button(button)
{
    if (Windows::get()->is_empty()) {
        return;
    }

    Windows::get()->get_focused_window()->on_mouse_button.connect(
        [this](MouseButton button, InputAction action, InputMods mods) {
            if (this->button == button) {
                on_button(action, mods);
            }
        }
    );
}

bool MouseInputButton::is_down() const
{
    return Windows::get()->is_empty() ?
               false :
               ((Windows::get()->get_focused_window()->get_mouse_button(button) != InputAction::Release) ^ inverted);
}

InputAxis::ArgumentDescription MouseInputButton::get_argument_desc() const
{
    return {
        {.name = "inverted", .type = "bool", .description = "If the down reading will be inverted"},
        {.name = "key", .type = "int", .description = "The mouse button on the mouse being checked"}
    };
}

void MouseInputButton::save(nlohmann::json& j)
{
    save_base(j);
    save_value(j, "button", button);
}

void MouseInputButton::load(nlohmann::json const& j)
{
    load_base(j);
    load_value(j, "button", button);
}
}