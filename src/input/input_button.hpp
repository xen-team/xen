#pragma once

#include "core.hpp"
#include "input_axis.hpp"
#include "system/window.hpp"
#include "utils/json_factory.hpp"
#include <json.hpp>

namespace xen {
class XEN_API InputButton : public JsonFactory<InputButton>, public virtual rocket::trackable {
protected:
    bool inverted = false;

private:
    bool was_downed = false;

public:
    rocket::signal<void(InputAction, InputMods)> on_button;

public:
    virtual ~InputButton() = default;

    [[nodiscard]] virtual bool is_down() const { return false; }

    [[nodiscard]] bool was_down()
    {
        bool const still_down = was_downed && is_down();
        was_downed = is_down();
        return was_downed == !still_down;
    }

    [[nodiscard]] virtual InputAxis::ArgumentDescription get_argument_desc() const { return {}; }

    [[nodiscard]] bool is_inverted() const { return inverted; }
    void set_inverted(bool inverted) { this->inverted = inverted; }

protected:
    void base_save(nlohmann::json& j) override { save_value(j, "inverted", inverted); }

    void base_load(nlohmann::json const& j) override { load_value(j, "inverted", inverted); }
};
}