#include "engine/engine.hpp"
#include "system/windows.hpp"
#include "input/inputs.hpp"
#include "input/buttons/combo_button.hpp"

using namespace xen;

int main()
{
    Engine engine;

    auto* window0 = Windows::get()->add_window();
    window0->set_title("Game");
    window0->on_close.connect([]() { Engine::get()->request_close(); });

    auto* input_scheme = Inputs::get()->add_scheme("Test", std::make_unique<InputScheme>("./test_scheme.json"), true);
    auto combo_button = std::make_unique<ComboInputButton>();
    combo_button->set_check_all(true);
    auto combo_button2 = std::make_unique<ComboInputButton>();
    combo_button2->set_check_all(false);
    combo_button->add_button(std::move(combo_button2));
    input_scheme->add_button("Combo6", std::move(combo_button));
    input_scheme->write();

    auto exit_code = engine.run();
    return exit_code;
}