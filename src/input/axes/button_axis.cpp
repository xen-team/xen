#include "button_axis.hpp"

namespace xen {
ButtonInputAxis::ButtonInputAxis(std::unique_ptr<InputButton>&& negative, std::unique_ptr<InputButton>&& positive) :
    negative(std::move(negative)), positive(std::move(positive))
{
    this->negative->on_button.connect([this](InputAction, InputMods) {
        on_axis(get_amount());
    });
    this->positive->on_button.connect([this](InputAction, InputMods) {
        on_axis(get_amount());
    });
}

float ButtonInputAxis::get_amount() const
{
    return (scale * static_cast<float>(positive->is_down() - negative->is_down())) + offset;
}

InputAxis::ArgumentDescription ButtonInputAxis::get_argument_desc() const
{
    return {
        {.name = "scale", .type = "float", .description = "Output amount scalar"},
        {.name = "negative", .type = "button", .description = "InputButton when pressed makes the amount read negative"
        },
        {.name = "positive", .type = "button", .description = "InputButton when pressed makes the amount read positive"}
    };
}

void ButtonInputAxis::set_negative(std::unique_ptr<InputButton>&& negative)
{
    this->negative = std::move(negative);
    this->negative->on_button.connect([this](InputAction, InputMods) {
        on_axis(get_amount());
    });
}

void ButtonInputAxis::set_positive(std::unique_ptr<InputButton>&& positive)
{
    this->positive = std::move(positive);
    this->positive->on_button.connect([this](InputAction, InputMods) {
        on_axis(get_amount());
    });
}
}
