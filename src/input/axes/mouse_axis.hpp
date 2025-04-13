#pragma once

#include "input/input_axis.hpp"

namespace xen {
class XEN_API MouseInputAxis : public InputAxis::Registrar<MouseInputAxis> {
    inline static bool const registered = Register("mouse");

private:
    Axis axis;

public:
    explicit MouseInputAxis(Axis axis = Axis::X);

    [[nodiscard]] float get_amount() const override;

    [[nodiscard]] ArgumentDescription get_argument_desc() const override;

    [[nodiscard]] Axis get_axis() const { return axis; }
    void set_axis(Axis axis) { this->axis = axis; }

    friend void to_json(json& j, MouseInputAxis const& p)
    {
        to_json(j["scale"], p.scale);
        to_json(j["axis"], p.axis);
    }

    friend void from_json(json const& j, MouseInputAxis& p)
    {
        from_json(j["scale"], p.scale);
        from_json(j["axis"], p.axis);
    }
};
}
