#pragma once

#include "core.hpp"
#include "utils/json_factory.hpp"

#include <rocket.hpp>
#include <nlohmann/json.hpp>

namespace xen {
class XEN_API InputAxis : public JsonFactory<InputAxis>, public virtual rocket::trackable {
protected:
    float scale = 1.0f;
    float offset = 0.0f;

public:
    rocket::signal<void(float)> on_axis;

    struct Argument {
        std::string name;
        std::string type;
        std::string description;
    };
    using ArgumentDescription = std::vector<Argument>;

public:
    virtual ~InputAxis() = default;

    [[nodiscard]] virtual float get_amount() const { return 0.0f; }

    [[nodiscard]] virtual ArgumentDescription get_argument_desc() const { return {}; }

    [[nodiscard]] float get_scale() const { return scale; }
    void set_scale(float scale) { this->scale = scale; }

    [[nodiscard]] float get_offset() const { return offset; }
    void set_offset(float offset) { this->offset = offset; }

protected:
    void base_save(nlohmann::json& j) override
    {
        save_value(j, "scale", scale);
        save_value(j, "offset", offset);
    }

    void base_load(nlohmann::json const& j) override
    {
        load_value(j, "scale", scale);
        load_value(j, "offset", offset);
    }
};
}