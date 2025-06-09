#pragma once

namespace xen {
template <typename T>
struct Degrees;
template <typename T>
struct Radians;

/// Degrees class, handling mathematical angles in degrees.
/// \tparam T Type of the degrees' value.
template <typename T>
struct Degrees {
    static_assert(std::is_floating_point_v<T>, "Error: Degrees' type must be floating point.");

    T value;

    constexpr explicit Degrees(T val) : value{val} {}
    constexpr explicit Degrees(Radians<T> rad) : value{rad.value * 180 / std::numbers::pi_v<T>} {}

    constexpr Degrees(Degrees const&) = default;
    constexpr Degrees(Degrees&&) noexcept = default;

    constexpr Degrees& operator=(Degrees const&) = default;
    constexpr Degrees& operator=(Degrees&&) noexcept = default;

    /// Degrees negation operator.
    /// \return Negated degrees object.
    constexpr Degrees operator-() const { return Degrees(-value); }

    /// Degrees-value addition operator.
    /// \tparam ValT Type of the value to be added.
    /// \param val Value to be added.
    /// \return Result of the degrees object summed with the value.
    template <typename ValT>
    constexpr Degrees operator+(ValT val) const
    {
        return Degrees(value + static_cast<T>(val));
    }

    /// Degrees-value substraction operator.
    /// \tparam ValT Type of the value to be substracted.
    /// \param val Value to be substracted.
    /// \return Result of the degrees object substracted by the value.
    template <typename ValT>
    constexpr Degrees operator-(ValT val) const
    {
        return Degrees(value - static_cast<T>(val));
    }

    /// Degrees-value multiplication operator.
    /// \tparam ValT Type of the value to be multiplied by.
    /// \param val Value to be multiplied by.
    /// \return Result of the degrees object multiplied by the value.
    template <typename ValT>
    constexpr Degrees operator*(ValT val) const
    {
        return Degrees(value * static_cast<T>(val));
    }

    /// Degrees-value division operator.
    /// \tparam ValT Type of the value to be divided by.
    /// \param val Value to be divided by.
    /// \return Result of the degrees object divided by the value.
    template <typename ValT>
    constexpr Degrees operator/(ValT val) const
    {
        return Degrees(value / static_cast<T>(val));
    }

    /// Radians-value addition assignment operator.
    /// \tparam ValT Type of the value to be added.
    /// \param val Value to be added.
    /// \return Reference to the modified original degrees object.
    template <typename ValT>
    constexpr Degrees& operator+=(ValT val)
    {
        value += static_cast<T>(val);
        return *this;
    }

    /// Radians-value substraction assignment operator.
    /// \tparam ValT Type of the value to be substracted.
    /// \param val Value to be substracted.
    /// \return Reference to the modified original degrees object.
    template <typename ValT>
    constexpr Degrees& operator-=(ValT val)
    {
        value -= static_cast<T>(val);
        return *this;
    }

    /// Radians-value multiplication assignment operator.
    /// \tparam ValT Type of the value to be multiplied by.
    /// \param val Value to be multiplied by.
    /// \return Reference to the modified original degrees object.
    template <typename ValT>
    constexpr Degrees& operator*=(ValT val)
    {
        value *= static_cast<T>(val);
        return *this;
    }

    /// Radians-value division assignment operator.
    /// \tparam ValT Type of the value to be divided by.
    /// \param val Value to be divided by.
    /// \return Reference to the modified original degrees object.
    template <typename ValT>
    constexpr Degrees& operator/=(ValT val)
    {
        value /= static_cast<T>(val);
        return *this;
    }

    /// Checks if the current degrees angle is equal to another given one.
    /// Uses a near-equality check to take floating-point errors into account.
    /// \param deg Degrees to be compared with.
    /// \return True if degrees' values are nearly equal, false otherwise.
    constexpr bool operator==(Degrees const& deg) const { return Math::almost_equal(value, deg.value); }

    /// Checks if the current degrees angle is different from another given one.
    /// Uses a near-equality check to take floating-point errors into account.
    /// \param deg Degrees to be compared with.
    /// \return True if degrees' values are different, false otherwise.
    constexpr bool operator!=(Degrees const& deg) const { return !(*this == deg); }

    /// Implicit degrees' value type conversion operator.
    /// \tparam T2 Type to convert the value into.
    /// \return Degrees object of the new type.
    template <typename T2>
    constexpr operator Degrees<T2>() const
    {
        return Degrees<T2>(static_cast<T2>(value));
    }

    /// Implicit degrees to radians conversion operator.
    /// \tparam T2 Type to convert the value into.
    /// \return Radians object of the new type.
    template <typename T2>
    constexpr operator Radians<T2>() const
    {
        return Radians<T2>(Degrees<T2>(static_cast<T2>(value)));
    }
};

// Aliases

using Degreesf = Degrees<float>;
using Degreesd = Degrees<double>;
using Degreesld = Degrees<long double>;

/// Radians class, handling mathematical angles in radians.
/// \tparam T Type of the radians' value.
template <typename T>
struct Radians {
    static_assert(std::is_floating_point_v<T>, "Error: Radians' type must be floating point.");

    T value;

    constexpr explicit Radians(T val) : value{val} {}
    constexpr explicit Radians(Degrees<T> deg) : value{deg.value * std::numbers::pi_v<T> / 180} {}

    Radians(Radians const&) = default;
    Radians(Radians&&) noexcept = default;

    Radians& operator=(Radians const&) = default;
    Radians& operator=(Radians&&) noexcept = default;

    /// Radians negation operator.
    /// \return Negated radians object.
    constexpr Radians operator-() const { return Radians(-value); }

    /// Radians-value addition operator.
    /// \tparam ValT Type of the value to be added.
    /// \param val Value to be added.
    /// \return Result of the radians object summed with the value.
    template <typename ValT>
    constexpr Radians operator+(ValT val) const
    {
        return Radians(value + static_cast<T>(val));
    }

    /// Radians-value substraction operator.
    /// \tparam ValT Type of the value to be substracted.
    /// \param val Value to be substracted.
    /// \return Result of the radians object substracted by the value.
    template <typename ValT>
    constexpr Radians operator-(ValT val) const
    {
        return Radians(value - static_cast<T>(val));
    }

    /// Radians-value multiplication operator.
    /// \tparam ValT Type of the value to be multiplied by.
    /// \param val Value to be multiplied by.
    /// \return Result of the radians object multiplied by the value.
    template <typename ValT>
    constexpr Radians operator*(ValT val) const
    {
        return Radians(value * static_cast<T>(val));
    }

    /// Radians-value division operator.
    /// \tparam ValT Type of the value to be divided by.
    /// \param val Value to be divided by.
    /// \return Result of the radians object divided by the value.
    template <typename ValT>
    constexpr Radians operator/(ValT val) const
    {
        return Radians(value / static_cast<T>(val));
    }

    /// Radians-value addition assignment operator.
    /// \tparam ValT Type of the value to be added.
    /// \param val Value to be added.
    /// \return Reference to the modified original radians object.
    template <typename ValT>
    constexpr Radians& operator+=(ValT val)
    {
        value += static_cast<T>(val);
        return *this;
    }

    /// Radians-value substraction assignment operator.
    /// \tparam ValT Type of the value to be substracted.
    /// \param val Value to be substracted.
    /// \return Reference to the modified original radians object.
    template <typename ValT>
    constexpr Radians& operator-=(ValT val)
    {
        value -= static_cast<T>(val);
        return *this;
    }

    /// Radians-value multiplication assignment operator.
    /// \tparam ValT Type of the value to be multiplied by.
    /// \param val Value to be multiplied by.
    /// \return Reference to the modified original radians object.
    template <typename ValT>
    constexpr Radians& operator*=(ValT val)
    {
        value *= static_cast<T>(val);
        return *this;
    }

    /// Radians-value division assignment operator.
    /// \tparam ValT Type of the value to be divided by.
    /// \param val Value to be divided by.
    /// \return Reference to the modified original radians object.
    template <typename ValT>
    constexpr Radians& operator/=(ValT val)
    {
        value /= static_cast<T>(val);
        return *this;
    }

    /// Checks if the current radians angle is equal to another given one.
    /// Uses a near-equality check to take floating-point errors into account.
    /// \param rad Radians to be compared with.
    /// \return True if radians' values are nearly equal, false otherwise.
    constexpr bool operator==(Radians const& rad) const { return Math::almost_equal(value, rad.value); }

    /// Checks if the current radians angle is different from another given one.
    /// Uses a near-equality check to take floating-point errors into account.
    /// \param rad Radians to be compared with.
    /// \return True if radians' values are different, false otherwise.
    constexpr bool operator!=(Radians const& rad) const { return !(*this == rad); }

    /// Implicit radians' value type conversion operator.
    /// \tparam T2 Type to convert the value into.
    /// \return Radians object of the new type.
    template <typename T2>
    constexpr operator Radians<T2>() const
    {
        return Radians<T2>(static_cast<T2>(value));
    }

    /// Implicit radians to degrees conversion operator.
    /// \tparam T2 Type to convert the value into.
    /// \return Degrees object of the new type.
    template <typename T2>
    constexpr operator Degrees<T2>() const
    {
        return Degrees<T2>(Radians<T2>(static_cast<T2>(value)));
    }
};

// Aliases

using Radiansf = Radians<float>;
using Radiansd = Radians<double>;

namespace Literals {
constexpr Degreesld operator""_deg(unsigned long long val) noexcept
{
    return Degreesld(static_cast<long double>(val));
}
/// Degrees user-defined literal.
/// \param val Value in degrees.
/// \return Degrees object containing the given value.
constexpr Degreesld operator""_deg(long double val) noexcept
{
    return Degreesld(val);
}

}
}