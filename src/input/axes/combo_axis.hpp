#pragma once

#include "utils/classes.hpp"
#include "input/input_axis.hpp"

namespace xen {
class XEN_API ComboInputAxis : public InputAxis::Registrar<ComboInputAxis>, NonCopyable {
    inline static bool const registered = Register("combo");

private:
    std::vector<std::unique_ptr<InputAxis>> axes;

public:
    explicit ComboInputAxis(std::vector<std::unique_ptr<InputAxis>>&& axes = {});

    template <typename... Args>
    explicit ComboInputAxis(Args&&... args)
    {
        axes.reserve(sizeof...(Args));
        (axes.emplace_back(std::forward<Args>(args)), ...);
        connect_axes();
    }

    [[nodiscard]] float get_amount() const override;

    [[nodiscard]] ArgumentDescription get_argument_desc() const override;

    [[nodiscard]] std::vector<std::unique_ptr<InputAxis>> const& get_axes() const { return axes; }
    InputAxis* add_axis(std::unique_ptr<InputAxis>&& axis);
    void remove_axis(InputAxis* axis);

    friend void to_json(json& j, ComboInputAxis const& p)
    {
        to_json(j["scale"], p.scale);
        to_json(j["axes"], p.axes);
    }

    friend void from_json(json const& j, ComboInputAxis& p)
    {
        from_json(j["scale"], p.scale);
        from_json(j["axes"], p.axes);
        p.connect_axes();
    }

private:
    void connect_axis(std::unique_ptr<InputAxis>& axis);
    void connect_axes();
};
}
