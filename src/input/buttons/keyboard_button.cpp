#include "keyboard_button.hpp"
#include "system/windows.hpp"

namespace xen {
KeyboardInputButton::KeyboardInputButton(Key key) : key(key)
{
    if (Windows::get()->is_empty()) {
        return;
    }

    Windows::get()->get_focused_window()->on_key.connect([this](Key key, InputAction action, InputMods mods) {
        if (this->key == key) {
            on_button(action, mods);
        }
    });
}

bool KeyboardInputButton::is_down() const
{
    return Windows::get()->is_empty() ?
               false :
               ((Windows::get()->get_focused_window()->get_key(key) != InputAction::Release) ^ inverted);
}

InputAxis::ArgumentDescription KeyboardInputButton::get_argument_desc() const
{
    return {
        {.name = "inverted", .type = "bool", .description = "If the down reading will be inverted"},
        {.name = "key", .type = "int", .description = "The key on the keyboard being checked"}
    };
}

void KeyboardInputButton::save(nlohmann::json& j)
{
    save_base(j);
    save_value(j, "key", key);
}
void KeyboardInputButton::load(nlohmann::json const& j)
{
    load_base(j);
    load_value(j, "key", key);
}
}