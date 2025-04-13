#pragma once

#include "core.hpp"
#include "utils/json_factory.hpp"

#include <rocket.hpp>

namespace xen {
enum class Axis : uint8_t { X, Y };

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
};
}