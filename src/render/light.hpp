#pragma once

#include <math/angle.hpp>

namespace xen {
enum class LightType { POINT = 0, DIRECTIONAL, SPOT };

class Light final : public Component {
public:
    constexpr Light(LightType type, float energy, Color const& color = Color::White) :
        type{type}, energy{energy}, color{color}
    {
    }
    constexpr Light(LightType type, Vector3f const& direction, float energy, Color const& color = Color::White) :
        type{type}, direction{direction}, energy{energy}, color{color}
    {
    }
    constexpr Light(
        LightType type, Vector3f const& direction, float energy, Radiansf angle, Color const& color = Color::White
    ) : type{type}, direction{direction}, energy{energy}, color{color}, angle{angle}
    {
    }

    constexpr LightType get_type() const { return type; }
    constexpr Vector3f const& get_direction() const { return direction; }
    constexpr float get_energy() const { return energy; }
    constexpr Color const& get_color() const { return color; }
    constexpr Radiansf get_angle() const { return angle; }

    constexpr void set_type(LightType type) { this->type = type; }
    constexpr void set_direction(Vector3f const& direction) { this->direction = direction; }
    constexpr void set_energy(float energy) { this->energy = energy; }
    constexpr void set_color(Color const& color) { this->color = color; }
    constexpr void set_angle(Radiansf angle) { this->angle = angle; }

private:
    LightType type{};
    Vector3f direction{};
    float energy = 1.f;
    Color color{};
    Radiansf angle = Radiansf(1.f);
};

}