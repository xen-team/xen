#include "axis_button.hpp"

namespace xen {
AxisInputButton::AxisInputButton(std::unique_ptr<InputAxis>&& axis, float min, float max) :
    axis(std::move(axis)), min(min), max(max)
{
    /*if (this->axis) {
            this->axis->OnAxis().connect(this, [this](float value) {
                    onButton(IsDown());
            });
    }*/
}

bool AxisInputButton::is_down() const
{
    float const amount = axis->get_amount();
    return min < amount && amount < max;
}

InputAxis::ArgumentDescription AxisInputButton::get_argument_desc() const
{
    return {
        {.name = "inverted", .type = "bool", .description = "If the down reading will be inverted"},
        {.name = "axis", .type = "axis", .description = "The axis to sample"},
        {.name = "min", .type = "float", .description = "Lower axis value bound"},
        {.name = "max", .type = "float", .description = "Upper axis value bound"}
    };
}

void AxisInputButton::set_axis(std::unique_ptr<InputAxis>&& axis)
{
    this->axis = std::move(axis);
}
}