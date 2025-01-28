#pragma once

#include "input/input_button.hpp"
#include "input/input_axis.hpp"

namespace xen {
class XEN_API ButtonInputAxis : public InputAxis::Registrar<ButtonInputAxis> {
    inline static bool const registered = Register("button");

private:
    std::unique_ptr<InputButton> negative;
    std::unique_ptr<InputButton> positive;

public:
    ButtonInputAxis() = default;

    ButtonInputAxis(std::unique_ptr<InputButton>&& negative, std::unique_ptr<InputButton>&& positive);

    [[nodiscard]] float get_amount() const override;

    [[nodiscard]] ArgumentDescription get_argument_desc() const override;

    [[nodiscard]] InputButton const* get_negative() const { return negative.get(); }
    void set_negative(std::unique_ptr<InputButton>&& negative);
    [[nodiscard]] InputButton const* get_positive() const { return positive.get(); }
    void set_positive(std::unique_ptr<InputButton>&& positive);

    void save(nlohmann::json& j) override;
    void load(nlohmann::json const& j) override;
};
}
