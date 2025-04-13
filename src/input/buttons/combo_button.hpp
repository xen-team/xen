#pragma once

#include "utils/classes.hpp"
#include "input/input_button.hpp"

namespace xen {
class XEN_API ComboInputButton : public InputButton::Registrar<ComboInputButton>, NonCopyable {
    inline static bool const registered = Register("combo");

public:
    using ButtonsCombo = std::vector<std::unique_ptr<InputButton>>;

private:
    ButtonsCombo buttons;
    bool check_all = false;
    bool last_down = false;

public:
    ComboInputButton(ButtonsCombo&& buttons = {}, bool check_all = false);

    template <std::same_as<InputButton>... Buttons>
    ComboInputButton(bool check_all, Buttons&&... args) : check_all(check_all)
    {
        buttons.reserve(sizeof...(Buttons));
        (buttons.emplace_back(std::forward<Buttons>(args)), ...);
        connect_buttons();
    }

    template <std::same_as<InputButton>... Buttons>
    ComboInputButton(Buttons&&... args)
    {
        buttons.reserve(sizeof...(Buttons));
        (buttons.emplace_back(std::forward<Buttons>(args)), ...);
        connect_buttons();
    }

    [[nodiscard]] bool is_down() const override;

    [[nodiscard]] InputAxis::ArgumentDescription get_argument_desc() const override;

    [[nodiscard]] ButtonsCombo const& get_buttons() const { return buttons; }
    InputButton* add_button(std::unique_ptr<InputButton>&& button);
    void remove_button(InputButton* button);

    [[nodiscard]] bool is_check_all() const { return check_all; }
    void set_check_all(bool check_all) { this->check_all = check_all; }

    friend void to_json(json& j, ComboInputButton const& p)
    {
        to_json(j["inverted"], p.inverted);
        to_json(j["buttons"], p.buttons);
        to_json(j["check_all"], p.check_all);
    }

    friend void from_json(json const& j, ComboInputButton& p)
    {
        from_json(j["inverted"], p.inverted);
        from_json(j["buttons"], p.buttons);
        from_json(j["check_all"], p.check_all);
        p.connect_buttons();
    }

private:
    void connect_button(std::unique_ptr<InputButton>& button);
    void connect_buttons();
};
}