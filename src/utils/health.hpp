#pragma once

#include "audio/sound.hpp"
#include "data/wav_format.hpp"
#include "filepath.hpp"
#include <system.hpp>
#include <component.hpp>
#include <rocket.hpp>

namespace xen {
class Health final : public xen::Component {
public:
    rocket::signal<void()> on_death;
    rocket::signal<void(float)> on_damage;

public:
    Health(float max_health) :
        max_value{max_health}, value{max_value},
        health_sound(std::make_unique<Sound>(WavFormat::load("assets/sounds/heal.wav"))) {};

    [[nodiscard]] float get() const { return value; }
    [[nodiscard]] float get_max() const { return max_value; }

    void apply_damage(float damage)
    {
        float const clamped_damage = value - std::clamp(value - damage, 0.f, max_value);
        on_damage.invoke(clamped_damage);

        value -= clamped_damage;

        if (value <= 0.f) {
            value = 0.f;
            on_death.invoke();
        }
    }
    void apply_heal(float heal)
    {
        health_sound->play();
        value = std::clamp(value + heal, 0.f, max_value);
    }

private:
    float max_value;
    float value;

    std::unique_ptr<Sound> health_sound;
};
}