#include "scroll_axis.hpp"
#include "system/windows.hpp"

namespace xen {
ScrollInputAxis::ScrollInputAxis(Axis axis) : axis(axis)
{
    if (Windows::get()->is_empty()) {
        return;
    }

    Windows::get()->get_focused_window()->on_mouse_scroll.connect([this]([[maybe_unused]] Vector2d) {
        on_axis(get_amount());
    });
}

float ScrollInputAxis::get_amount() const
{
    return Windows::get()->is_empty() ?
               0.f :
               (scale * static_cast<float>(Windows::get()->get_focused_window()->get_mouse_scroll_delta(
                        )[static_cast<uint8_t>(axis)])) +
                   offset;
}

InputAxis::ArgumentDescription ScrollInputAxis::get_argument_desc() const
{
    return {
        {.name = "scale", .type = "float", .description = "Output amount scalar"},
        {.name = "axis", .type = "axis", .description = "The axis on the mouse delta is being checked"}
    };
}
}
