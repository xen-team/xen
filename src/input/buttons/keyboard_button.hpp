#pragma once

#include "input/input_button.hpp"

namespace xen {
class XEN_API KeyboardInputButton : public InputButton::Registrar<KeyboardInputButton> {
    inline static bool const registered = Register("keyboard");

private:
    Key key;

public:
    explicit KeyboardInputButton(Key key = Key::Unknown);

    [[nodiscard]] bool is_down() const override;

    [[nodiscard]] InputAxis::ArgumentDescription get_argument_desc() const override;

    [[nodiscard]] Key get_key() const { return key; }
    void set_key(Key key) { this->key = key; }

    friend void to_json(json& j, KeyboardInputButton const& p)
    {
        to_json(j["inverted"], p.inverted);
        to_json(j["key"], p.key);
    }

    friend void from_json(json const& j, KeyboardInputButton& p)
    {
        from_json(j["inverted"], p.inverted);
        from_json(j["key"], p.key);
    }
};
}