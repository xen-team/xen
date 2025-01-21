#pragma once

#include <cmath>

namespace xen {
class Math {
public:
    Math() = delete;

    template<typename T = float>
    static T random(T min, T max);

    template<typename T = float>
    static T random_normal(T standart, T mean);

    template<typename T = float>
    static T random_log(T min, T max);

    template<typename T = float>
    constexpr static T deg_to_rad(T const& degrees) {
        return static_cast<T>(degrees * M_PI / 180);
    }

    template<typename T = float>
    static constexpr T rad_to_deg(T const& radians) {
        return static_cast<T>(radians * 180 / M_PI);
    }

    template<typename T = float>
    static T deg_normal(T const& degrees) {
        auto x = std::fmod(degrees, 360);

        if (x < 0) {
            x += 360;
        }

        return static_cast<T>(x);
    }

    template<typename T = float>
    static T rad_normal(T const& radians) {
        auto x = std::fmod(radians, 2 * M_PI);

        if (x < 0) {
            x += 2 * M_PI;
        }

        return static_cast<T>(x);
    }

    template<typename T = float, typename K = float>
    static bool almost_equal(T const& a, T const& b, const K &eps) {
        return std::fabs(a - b) < eps;
    }

    template<typename T = float, typename K = float>
    static constexpr auto lerp(T const& a, T const& b, const K &factor) {
        return a * (1 - factor) + b * factor;
    }

    template<typename T = float>
    static T deadband(T const& min, T const& value) {
        return std::fabs(value) >= std::fabs(min) ? value : 0.0f;
    }
};
}