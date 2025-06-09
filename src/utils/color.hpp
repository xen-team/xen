#pragma once

#include <math/math.hpp>

#include <cassert>
#include <sstream>
#include <iomanip>

namespace xen {
class XEN_API Color {
public:
    float r = 0.f;
    float g = 0.f;
    float b = 0.f;
    float a = 1.f;

    static Color const Clear;
    static Color const Black;
    static Color const Grey;
    static Color const Silver;
    static Color const White;
    static Color const Maroon;
    static Color const Red;
    static Color const Olive;
    static Color const Yellow;
    static Color const Green;
    static Color const Lime;
    static Color const Teal;
    static Color const Aqua;
    static Color const Navy;
    static Color const Blue;
    static Color const Purple;
    static Color const Fuchsia;

public:
    /// In order of how bits are mapped [24, 16, 8, 0xFF].
    enum class Type { RGBA, ARGB, RGB };

    Color() = default;

    constexpr Color(float r, float g, float b, float a = 1.0f) : r(r), g(g), b(b), a(a) {}

    constexpr Color(uint32_t i, Type type = Type::RGB)
    {
        switch (type) {
        case Type::RGBA:
            r = static_cast<float>((uint8_t)(i >> 24 & 0xFF)) / 255.0f;
            g = static_cast<float>((uint8_t)(i >> 16 & 0xFF)) / 255.0f;
            b = static_cast<float>((uint8_t)(i >> 8 & 0xFF)) / 255.0f;
            a = static_cast<float>((uint8_t)(i & 0xFF)) / 255.0f;
            break;
        case Type::ARGB:
            r = static_cast<float>((uint8_t)(i >> 16)) / 255.0f;
            g = static_cast<float>((uint8_t)(i >> 8)) / 255.0f;
            b = static_cast<float>((uint8_t)(i & 0xFF)) / 255.0f;
            a = static_cast<float>((uint8_t)(i >> 24)) / 255.0f;
            break;
        case Type::RGB:
            r = static_cast<float>((uint8_t)(i >> 16)) / 255.0f;
            g = static_cast<float>((uint8_t)(i >> 8)) / 255.0f;
            b = static_cast<float>((uint8_t)(i & 0xFF)) / 255.0f;
            a = 1.0f;
            break;
        default:
            throw std::runtime_error("Unknown Color type");
        }
    }

    constexpr Color(std::string hex, float a = 1.0f) : a(a)
    {
        if (hex[0] == '#') {
            hex.erase(0, 1);
        }

        assert(hex.size() == 6);
        ulong const hex_value = std::stoul(hex, nullptr, 16);

        r = static_cast<float>((hex_value >> 16) & 0xff) / 255.0f;
        g = static_cast<float>((hex_value >> 8) & 0xff) / 255.0f;
        b = static_cast<float>((hex_value >> 0) & 0xff) / 255.0f;
    }

    [[nodiscard]] constexpr Color lerp(Color const& other, float progression) const
    {
        Color const ta = *this * (1.0f - progression);
        Color const tb = other * progression;
        return ta + tb;
    }

    [[nodiscard]] Color normalize() const
    {
        float const l = length();

        if (l == 0.0f) {
            throw std::runtime_error("Can't normalize a zero length vector");
        }

        return {r / l, g / l, b / l, a / l};
    }

    [[nodiscard]] constexpr float length_squared() const { return (r * r) + (g * g) + (b * b) + (a * a); }

    [[nodiscard]] constexpr float length() const { return std::sqrt(length_squared()); }

    [[nodiscard]] constexpr Color smooth_damp(Color const& target, Color const& rate) const
    {
        return Math::smooth_damp(*this, target, rate);
    }

    [[nodiscard]] constexpr Color get_unit() const
    {
        auto l = length();
        return {r / l, g / l, b / l, a / l};
    }

    [[nodiscard]] constexpr uint32_t as_int(Type type = Type::RGBA) const
    {
        switch (type) {
        case Type::RGBA:
            return (static_cast<uint8_t>(r * 255.0f) << 24) | (static_cast<uint8_t>(g * 255.0f) << 16) |
                   (static_cast<uint8_t>(b * 255.0f) << 8) | (static_cast<uint8_t>(a * 255.0f) & 0xFF);
        case Type::ARGB:
            return (static_cast<uint8_t>(a * 255.0f) << 24) | (static_cast<uint8_t>(r * 255.0f) << 16) |
                   (static_cast<uint8_t>(g * 255.0f) << 8) | (static_cast<uint8_t>(b * 255.0f) & 0xFF);
        case Type::RGB:
            return (static_cast<uint8_t>(r * 255.0f) << 16) | (static_cast<uint8_t>(g * 255.0f) << 8) |
                   (static_cast<uint8_t>(b * 255.0f) & 0xFF);
        default:
            throw std::runtime_error("Unknown Color type");
        }
    }

    [[nodiscard]] std::string as_hex() const
    {
        std::stringstream stream;
        stream << "#";

        auto hexValue = ((static_cast<uint32_t>(r * 255.0f) & 0xff) << 16) +
                        ((static_cast<uint32_t>(g * 255.0f) & 0xff) << 8) +
                        ((static_cast<uint32_t>(b * 255.0f) & 0xff) << 0);
        stream << std::hex << std::setfill('0') << std::setw(6) << hexValue;

        return stream.str();
    }

    [[nodiscard]] constexpr operator Vector3f() const { return {r, g, b}; }

    [[nodiscard]] constexpr operator Vector4f() const { return {r, g, b, a}; }

    [[nodiscard]] constexpr float operator[](uint32_t i) const
    {
        assert(i < 4 && "Color subscript out of range");
        return i == 0 ? r : i == 1 ? g : i == 2 ? b : a;
    }
    [[nodiscard]] constexpr float& operator[](uint32_t i)
    {
        assert(i < 4 && "Color subscript out of range");
        return i == 0 ? r : i == 1 ? g : i == 2 ? b : a;
    }

    constexpr bool operator==(Color const& rhs) const;
    constexpr bool operator!=(Color const& rhs) const;

    constexpr friend Color operator+(Color const& lhs, Color const& rhs);
    constexpr friend Color operator-(Color const& lhs, Color const& rhs);
    constexpr friend Color operator*(Color const& lhs, Color const& rhs);
    constexpr friend Color operator/(Color const& lhs, Color const& rhs);
    constexpr friend Color operator+(float lhs, Color const& rhs);
    constexpr friend Color operator-(float lhs, Color const& rhs);
    constexpr friend Color operator*(float lhs, Color const& rhs);
    constexpr friend Color operator/(float lhs, Color const& rhs);
    constexpr friend Color operator+(Color const& lhs, float rhs);
    constexpr friend Color operator-(Color const& lhs, float rhs);
    constexpr friend Color operator*(Color const& lhs, float rhs);
    constexpr friend Color operator/(Color const& lhs, float rhs);

    constexpr Color& operator+=(Color const& rhs);
    constexpr Color& operator-=(Color const& rhs);
    constexpr Color& operator*=(Color const& rhs);
    constexpr Color& operator/=(Color const& rhs);
    constexpr Color& operator+=(float rhs);
    constexpr Color& operator-=(float rhs);
    constexpr Color& operator*=(float rhs);
    constexpr Color& operator/=(float rhs);

    friend std::ostream& operator<<(std::ostream& stream, Color const& Color);

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(r, g, b, a);
    }
};
}

#include "color.inl"