#pragma once

#include "color.hpp"

namespace xen {
constexpr bool Color::operator==(Color const& rhs) const
{
    return r == rhs.r && g == rhs.g && b == rhs.b && a == rhs.a;
}

constexpr bool Color::operator!=(Color const& rhs) const
{
    return !operator==(rhs);
}

constexpr Color operator+(Color const& lhs, Color const& rhs)
{
    return {lhs.r + rhs.r, lhs.g + rhs.g, lhs.b + rhs.b, lhs.a + rhs.a};
}

constexpr Color operator-(Color const& lhs, Color const& rhs)
{
    return {lhs.r - rhs.r, lhs.g - rhs.g, lhs.b - rhs.b, lhs.a - rhs.a};
}

constexpr Color operator*(Color const& lhs, Color const& rhs)
{
    return {lhs.r * rhs.r, lhs.g * rhs.g, lhs.b * rhs.b, lhs.a * rhs.a};
}

constexpr Color operator/(Color const& lhs, Color const& rhs)
{
    return {lhs.r / rhs.r, lhs.g / rhs.g, lhs.b / rhs.b, lhs.a / rhs.a};
}

constexpr Color operator+(float lhs, Color const& rhs)
{
    return Color(lhs, lhs, lhs, 0.0f) + rhs;
}

constexpr Color operator-(float lhs, Color const& rhs)
{
    return Color(lhs, lhs, lhs, 0.0f) - rhs;
}

constexpr Color operator*(float lhs, Color const& rhs)
{
    return Color(lhs, lhs, lhs) * rhs;
}

constexpr Color operator/(float lhs, Color const& rhs)
{
    return Color(lhs, lhs, lhs) / rhs;
}

constexpr Color operator+(Color const& lhs, float rhs)
{
    return lhs + Color(rhs, rhs, rhs, 0.0f);
}

constexpr Color operator-(Color const& lhs, float rhs)
{
    return lhs - Color(rhs, rhs, rhs, 0.0f);
}

constexpr Color operator*(Color const& lhs, float rhs)
{
    return lhs * Color(rhs, rhs, rhs);
}

constexpr Color operator/(Color const& lhs, float rhs)
{
    return lhs / Color(rhs, rhs, rhs);
}

constexpr Color& Color::operator+=(Color const& rhs)
{
    return *this = *this + rhs;
}

constexpr Color& Color::operator-=(Color const& rhs)
{
    return *this = *this - rhs;
}

constexpr Color& Color::operator*=(Color const& rhs)
{
    return *this = *this * rhs;
}

constexpr Color& Color::operator/=(Color const& rhs)
{
    return *this = *this / rhs;
}

constexpr Color& Color::operator+=(float rhs)
{
    return *this = *this + rhs;
}

constexpr Color& Color::operator-=(float rhs)
{
    return *this = *this - rhs;
}

constexpr Color& Color::operator*=(float rhs)
{
    return *this = *this * rhs;
}

constexpr Color& Color::operator/=(float rhs)
{
    return *this = *this / rhs;
}

inline std::ostream& operator<<(std::ostream& stream, Color const& color)
{
    return stream << color.r << ", " << color.g << ", " << color.b << ", " << color.a;
}
}