#pragma once

#include "input/input_axis.hpp"
#include "input/input_button.hpp"

namespace xen {
class XEN_API AxisInputButton : public InputButton::Registrar<AxisInputButton> {
    inline static bool const registered = Register("axis");

private:
    std::unique_ptr<InputAxis> axis;
    float min, max;

public:
    AxisInputButton(std::unique_ptr<InputAxis>&& axis = nullptr, float min = 0.1f, float max = 1.0f);

    [[nodiscard]] bool is_down() const override;

    [[nodiscard]] InputAxis::ArgumentDescription get_argument_desc() const override;

    [[nodiscard]] InputAxis const* get_axis() const { return axis.get(); }
    void set_axis(std::unique_ptr<InputAxis>&& axis);

    [[nodiscard]] float get_min() const { return min; }
    void set_min(float min) { this->min = min; }
    [[nodiscard]] float get_max() const { return max; }
    void set_max(float max) { this->max = max; }

    void save(nlohmann::json& j) override;
    void load(nlohmann::json const& j) override;
};
}