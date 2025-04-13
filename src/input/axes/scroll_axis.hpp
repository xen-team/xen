#pragma once

#include "input/input_axis.hpp"

namespace xen {
class XEN_API ScrollInputAxis : public InputAxis::Registrar<ScrollInputAxis> {
    inline static bool const registered = Register("scroll");

private:
    Axis axis;

public:
    explicit ScrollInputAxis(Axis axis = Axis::X);

    [[nodiscard]] float get_amount() const override;

    [[nodiscard]] ArgumentDescription get_argument_desc() const override;

    [[nodiscard]] Axis get_axis() const { return axis; }
    void set_axis(Axis axis) { this->axis = axis; }

    friend void to_json(json& j, ScrollInputAxis const& p)
    {
        to_json(j["scale"], p.scale);
        to_json(j["axis"], p.axis);
    }

    friend void from_json(json const& j, ScrollInputAxis& p)
    {
        from_json(j["scale"], p.scale);
        from_json(j["axis"], p.axis);
    }
};
}
