#include "mouse_axis.hpp"
#include "system/windows.hpp"

namespace xen {
MouseInputAxis::MouseInputAxis(Axis axis) : axis(axis)
{
    if (Windows::get()->is_empty()) {
        return;
    }

    Windows::get()->get_focused_window()->on_mouse_pos.connect([this]([[maybe_unused]] Vector2d value) {
        on_axis(get_amount());
    });
}

float MouseInputAxis::get_amount() const
{
    return Windows::get()->is_empty() ?
               0.f :
               (scale * static_cast<float>(Windows::get()->get_focused_window()->get_mouse_pos_delta(
                        )[static_cast<uint8_t>(axis)])) +
                   offset;
}

InputAxis::ArgumentDescription MouseInputAxis::get_argument_desc() const
{
    return {
        {.name = "scale", .type = "float", .description = "Output amount scalar"},
        {.name = "axis", .type = "axis", .description = "The axis on the mouse delta is being checked"}
    };
}
}
