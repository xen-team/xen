#pragma once

#include "core.hpp"

#include <chrono>
#include <iomanip>
#include <sstream>

namespace xen {
using namespace std::chrono_literals;

class XEN_API Time {
private:
    std::chrono::microseconds value{};

public:
    using internal_type = uint32_t;

public:
    Time() = default;

    template <typename Rep, typename Period>
    constexpr Time(std::chrono::duration<Rep, Period> const& duration) :
        value(std::chrono::duration_cast<std::chrono::microseconds>(duration).count())
    {
    }

    template <typename T = internal_type>
    [[nodiscard]] constexpr static Time seconds(T seconds)
    {
        return Time(std::chrono::duration<T>(seconds));
    }

    template <typename T = internal_type>
    [[nodiscard]] constexpr static Time milliseconds(T milliseconds)
    {
        return Time(std::chrono::duration<T>(milliseconds));
    }

    template <typename T = internal_type>
    [[nodiscard]] constexpr static Time microseconds(T microseconds)
    {
        return Time(std::chrono::duration<T>(microseconds));
    }

    template <typename T = float>
    [[nodiscard]] constexpr T as_seconds() const
    {
        return static_cast<T>(value.count()) / static_cast<T>(1'000'000);
    }

    template <typename T = float>
    [[nodiscard]] constexpr T as_milliseconds() const
    {
        return static_cast<T>(value.count()) / static_cast<T>(1'000);
    }

    template <typename T = internal_type>
    [[nodiscard]] constexpr T as_microseconds() const
    {
        return static_cast<T>(value.count());
    }

    [[nodiscard]] static Time now()
    {
        static auto const local_epoch = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::high_resolution_clock::now() - local_epoch
        );
    }

    [[nodiscard]] static std::string get_date_time(std::string_view format = "%Y-%m-%d %H:%M:%S")
    {
        auto const now = std::chrono::system_clock::now();
        auto const now_time_t = std::chrono::system_clock::to_time_t(now);

        std::stringstream ss;
        ss << std::put_time(std::localtime(&now_time_t), format.data());
        return ss.str();
    }

    template <typename Rep, typename Period>
    constexpr operator std::chrono::duration<Rep, Period>() const
    {
        return std::chrono::duration_cast<std::chrono::duration<Rep, Period>>(value);
    }

    constexpr bool operator==(Time const& rhs) const;
    constexpr bool operator!=(Time const& rhs) const;
    constexpr bool operator<(Time const& rhs) const;
    constexpr bool operator<=(Time const& rhs) const;
    constexpr bool operator>(Time const& rhs) const;
    constexpr bool operator>=(Time const& rhs) const;

    constexpr Time operator-() const;

    constexpr friend Time operator+(Time const& lhs, Time const& rhs);
    constexpr friend Time operator-(Time const& lhs, Time const& rhs);
    constexpr friend Time operator*(Time const& lhs, float rhs);
    constexpr friend Time operator*(Time const& lhs, internal_type rhs);
    constexpr friend Time operator*(float lhs, Time const& rhs);
    constexpr friend Time operator*(internal_type lhs, Time const& rhs);
    constexpr friend Time operator/(Time const& lhs, float rhs);
    constexpr friend Time operator/(Time const& lhs, internal_type rhs);
    constexpr friend double operator/(Time const& lhs, Time const& rhs);

    constexpr Time& operator+=(Time const& rhs);
    constexpr Time& operator-=(Time const& rhs);
    constexpr Time& operator*=(float rhs);
    constexpr Time& operator*=(internal_type rhs);
    constexpr Time& operator/=(float rhs);
    constexpr Time& operator/=(internal_type rhs);
};
}

#include "time.inl"