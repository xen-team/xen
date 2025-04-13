#pragma once

#include "core.hpp"

#include <cmath>
#include <random>

template <typename T>
concept HasDot = requires(T a, T b) {
    { a.dot(b) };
};

namespace xen {
class XEN_API Math {
public:
    template <typename T>
    constexpr static T PI = static_cast<T>(3.14159265358979323846264338327950288L);

    Math() = delete;

    static std::mt19937 random_generator;

    template <typename T = float>
    [[nodiscard]] static T random(T min, T max)
    {
        if constexpr (std::is_integral_v<T>) {
            std::uniform_int_distribution<T> dist(min, max);
            return dist(random_generator);
        }
        else if constexpr (std::is_floating_point_v<T>) {
            std::uniform_real_distribution<T> dist(min, max);
            return dist(random_generator);
        }
    }

    template <typename T = float>
    [[nodiscard]] static T random_normal(T standart_deviation, T mean)
    {
        std::normal_distribution<T> dist(mean, standart_deviation);
        return dist(random_generator);
    }

    template <typename T = float>
    [[nodiscard]] static T random_log(T min, T max)
    {
        auto const logLower = std::log(min);
        auto const logUpper = std::log(max);
        auto const raw = random(min, max);

        auto result = std::exp((raw * (logUpper - logLower)) + logLower);

        if (result < min) {
            result = min;
        }
        else if (result > max) {
            result = max;
        }

        return result;
    }

    template <HasDot T>
    constexpr static float distance_squared(T const& v1, T const& v2)
    {
        T const diff = v2 - v1;
        return diff.dot(diff);
    }

    template <typename T = float>
    [[nodiscard]] constexpr static T deg_to_rad(T const& degrees)
    {
        return static_cast<T>(degrees * M_PI / 180);
    }

    template <typename T = float>
    [[nodiscard]] static constexpr T rad_to_deg(T const& radians)
    {
        return static_cast<T>(radians * 180 / M_PI);
    }

    template <typename T = float>
    [[nodiscard]] static T deg_normal(T const& degrees)
    {
        auto x = std::fmod(degrees, 360);

        if (x < 0) {
            x += 360;
        }

        return static_cast<T>(x);
    }

    template <typename T = float>
    [[nodiscard]] static T rad_normal(T const& radians)
    {
        auto x = std::fmod(radians, 2 * M_PI);

        if (x < 0) {
            x += 2 * M_PI;
        }

        return static_cast<T>(x);
    }

    template <typename T = float, typename K = float>
    [[nodiscard]] static bool almost_equal(T const& a, T const& b, K const& eps)
    {
        return std::fabs(a - b) < eps;
    }

    template <typename T = float, typename K = float>
    constexpr static auto smooth_damp(T const& current, T const& target, K const& rate)
    {
        return current + ((target - current) * rate);
    }

    template <typename T = float, typename K = float>
    [[nodiscard]] static constexpr auto lerp(T const& a, T const& b, K const& factor)
    {
        return (a * (1 - factor)) + (b * factor);
    }

    template <typename T = float>
    [[nodiscard]] static T deadband(T const& min, T const& value)
    {
        return std::fabs(value) >= std::fabs(min) ? value : 0.0f;
    }

    template <typename T = float, typename K = float>
    static auto cos_from_sin(T const& sin, K const& angle)
    {
        // sin(x)^2 + cos(x)^2 = 1
        auto cos = std::sqrt(1 - sin * sin);
        auto a = angle + (PI<T> / 2);
        auto b = a - static_cast<int32_t>(a / (2 * PI<T>)) * (2 * PI<T>);

        if (b < 0) {
            b = (2 * PI<T>)+b;
        }

        if (b >= PI<T>) {
            return -cos;
        }

        return cos;
    }
};
}