#pragma once

#include "input/input_axis.hpp"

namespace xen {
class XEN_API MouseInputAxis : public InputAxis::Registrar<MouseInputAxis> {
    inline static bool const registered = Register("mouse");

private:
    uint8_t axis;

public:
    explicit MouseInputAxis(uint8_t axis = 0);

    [[nodiscard]] float get_amount() const override;

    [[nodiscard]] ArgumentDescription get_argument_desc() const override;

    [[nodiscard]] uint8_t get_axis() const { return axis; }
    void set_axis(uint8_t axis) { this->axis = axis; }

    void save(nlohmann::json& j) override;
    void load(nlohmann::json const& j) override;
};
}
