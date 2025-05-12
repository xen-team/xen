#pragma once

/// Creates bitwise operators (NOT ~, AND &, OR |, XOR ^, and their assignment counterparts) for the given Enum type.
/// This allows an enum to be used as a flag.
/// \param Enum Enum type to create the operators for.
#define MAKE_ENUM_FLAG(Enum)                                                                                           \
    constexpr Enum operator~(Enum value)                                                                               \
    {                                                                                                                  \
        return static_cast<Enum>(~static_cast<std::underlying_type_t<Enum>>(value));                                   \
    }                                                                                                                  \
    constexpr Enum operator&(Enum lhs, Enum rhs)                                                                       \
    {                                                                                                                  \
        using EnumT = std::underlying_type_t<Enum>;                                                                    \
        return static_cast<Enum>(static_cast<EnumT>(lhs) & static_cast<EnumT>(rhs));                                   \
    }                                                                                                                  \
    constexpr Enum& operator&=(Enum& lhs, Enum rhs)                                                                    \
    {                                                                                                                  \
        lhs = lhs & rhs;                                                                                               \
        return lhs;                                                                                                    \
    }                                                                                                                  \
    constexpr Enum operator|(Enum lhs, Enum rhs)                                                                       \
    {                                                                                                                  \
        using EnumT = std::underlying_type_t<Enum>;                                                                    \
        return static_cast<Enum>(static_cast<EnumT>(lhs) | static_cast<EnumT>(rhs));                                   \
    }                                                                                                                  \
    constexpr Enum& operator|=(Enum& lhs, Enum rhs)                                                                    \
    {                                                                                                                  \
        lhs = lhs | rhs;                                                                                               \
        return lhs;                                                                                                    \
    }                                                                                                                  \
    constexpr Enum operator^(Enum lhs, Enum rhs)                                                                       \
    {                                                                                                                  \
        using EnumT = std::underlying_type_t<Enum>;                                                                    \
        return static_cast<Enum>(static_cast<EnumT>(lhs) ^ static_cast<EnumT>(rhs));                                   \
    }                                                                                                                  \
    constexpr Enum& operator^=(Enum& lhs, Enum rhs)                                                                    \
    {                                                                                                                  \
        lhs = lhs ^ rhs;                                                                                               \
        return lhs;                                                                                                    \
    }
