#pragma once

#include "time.hpp"

namespace xen {
constexpr bool Time::operator==(Time const& rhs) const
{
    return value == rhs.value;
}

constexpr bool Time::operator!=(Time const& rhs) const
{
    return value != rhs.value;
}

constexpr bool Time::operator<(Time const& rhs) const
{
    return value < rhs.value;
}

constexpr bool Time::operator<=(Time const& rhs) const
{
    return value <= rhs.value;
}

constexpr bool Time::operator>(Time const& rhs) const
{
    return value > rhs.value;
}

constexpr bool Time::operator>=(Time const& rhs) const
{
    return value >= rhs.value;
}

constexpr Time Time::operator-() const
{
    return Time(-value);
}

constexpr Time operator+(Time const& lhs, Time const& rhs)
{
    return lhs.value + rhs.value;
}

constexpr Time operator-(Time const& lhs, Time const& rhs)
{
    return lhs.value - rhs.value;
}

constexpr Time operator*(Time const& lhs, float rhs)
{
    return lhs.value * rhs;
}

constexpr Time operator*(Time const& lhs, Time::internal_type rhs)
{
    return lhs.value * rhs;
}

constexpr Time operator*(float lhs, Time const& rhs)
{
    return rhs * lhs;
}

constexpr Time operator*(Time::internal_type lhs, Time const& rhs)
{
    return rhs * lhs;
}

constexpr Time operator/(Time const& lhs, float rhs)
{
    return lhs.value / rhs;
}

constexpr Time operator/(Time const& lhs, Time::internal_type rhs)
{
    return lhs.value / rhs;
}

constexpr double operator/(Time const& lhs, Time const& rhs)
{
    return static_cast<double>(lhs.value.count()) / static_cast<double>(rhs.value.count());
}

constexpr Time& Time::operator+=(Time const& rhs)
{
    return *this = *this + rhs;
}

constexpr Time& Time::operator-=(Time const& rhs)
{
    return *this = *this - rhs;
}

constexpr Time& Time::operator*=(float rhs)
{
    return *this = *this * rhs;
}

constexpr Time& Time::operator*=(Time::internal_type rhs)
{
    return *this = *this * rhs;
}

constexpr Time& Time::operator/=(float rhs)
{
    return *this = *this / rhs;
}

constexpr Time& Time::operator/=(Time::internal_type rhs)
{
    return *this = *this / rhs;
}
}