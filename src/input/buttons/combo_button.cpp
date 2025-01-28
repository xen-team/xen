#include "combo_button.hpp"
#include "utils/enumerate.hpp"

namespace xen {
ComboInputButton::ComboInputButton(ButtonsCombo&& buttons, bool check_all) :
    buttons{std::move(buttons)}, check_all{check_all}
{
    connect_buttons();
}

bool ComboInputButton::is_down() const
{
    for (auto const& button : buttons) {
        if (check_all && !button->is_down()) {
            return false ^ inverted;
        }

        if (!check_all && button->is_down()) {
            return true ^ inverted;
        }
    }

    return check_all ^ inverted;
}

InputAxis::ArgumentDescription ComboInputButton::get_argument_desc() const
{
    return {
        {.name = "inverted", .type = "bool", .description = "If the down reading will be inverted"},
        {.name = "buttons",
         .type = "button[]",
         .description = "The buttons that will be combined into a compound button"},
        {.name = "useAnd", .type = "bool", .description = "If must be down for a down reading, or just one"}
    };
}

InputButton* ComboInputButton::add_button(std::unique_ptr<InputButton>&& button)
{
    auto& result = buttons.emplace_back(std::move(button));
    connect_button(result);
    return result.get();
}

void ComboInputButton::remove_button(InputButton* button)
{
    // button->OnButton().RemoveObservers(this);
    buttons.erase(
        std::remove_if(buttons.begin(), buttons.end(), [button](auto const& b) { return b.get() == button; }),
        buttons.end()
    );
}

void ComboInputButton::connect_button(std::unique_ptr<InputButton>& button)
{
    button->on_button.connect([this]([[maybe_unused]] InputAction action, [[maybe_unused]] InputMods mods) {
        bool const down = is_down();

        if (!last_down && down) {
            last_down = true;
            on_button(InputAction::Press, 0);
        }
        else if (last_down && !down) {
            last_down = false;
            on_button(InputAction::Release, 0);
        }
        else if (last_down && down) {
            // TODO: This will be sent for every button, only count one per cycle.
            on_button(InputAction::Repeat, 0);
        }
    });
}

void ComboInputButton::connect_buttons()
{
    for (auto& button : buttons) {
        connect_button(button);
    }
}

void ComboInputButton::save(nlohmann::json& j)
{
    save_base(j);
    save_value(j, "check_all", check_all);
    for (auto [i, button] : enumerate(buttons)) {
        button->save(j["buttons"][std::to_string(i)]);
    }
}
void ComboInputButton::load(nlohmann::json const& j)
{
    load_base(j);
    load_value(j, "check_all", check_all);
    if (!j.contains("buttons")) {
        return;
    }

    for (auto const& [i, button] : enumerate(j["buttons"])) {
        buttons.emplace_back(create_by_type(button));
        buttons[i]->load(button);
    }
    connect_buttons();
}
}