#include "combo_axis.hpp"
#include "utils/enumerate.hpp"

namespace xen {
ComboInputAxis::ComboInputAxis(std::vector<std::unique_ptr<InputAxis>>&& axes) : axes(std::move(axes))
{
    connect_axes();
}

float ComboInputAxis::get_amount() const
{
    float result = 0.0f;
    for (auto const& axis : axes) {
        result += axis->get_amount();
    }

    return (scale * std::clamp(result, -1.0f, 1.0f)) + offset;
}

InputAxis::ArgumentDescription ComboInputAxis::get_argument_desc() const
{
    return {
        {.name = "scale", .type = "float", .description = "Output amount scalar"},
        {.name = "axes", .type = "axis[]", .description = "The axes that will be combined into a compound axis"}
    };
}

InputAxis* ComboInputAxis::add_axis(std::unique_ptr<InputAxis>&& axis)
{
    auto& result = axes.emplace_back(std::move(axis));
    connect_axis(result);
    return result.get();
}

void ComboInputAxis::remove_axis(InputAxis* axis)
{
    // axis->OnAxis().RemoveObservers(this);
    axes.erase(std::remove_if(axes.begin(), axes.end(), [axis](auto const& a) { return a.get() == axis; }), axes.end());
}

void ComboInputAxis::connect_axis(std::unique_ptr<InputAxis>& axis)
{
    axis->on_axis.connect([this]([[maybe_unused]] float value) { on_axis(get_amount()); });
}

void ComboInputAxis::connect_axes()
{
    for (auto& axis : axes) {
        connect_axis(axis);
    }
}
}
