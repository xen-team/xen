#pragma once

namespace xen {
template <typename T, T InvalidValue = T()>
class OwnerValue {
public:
    constexpr OwnerValue() = default;

    constexpr explicit OwnerValue(T const& value) : value{value} {}
    constexpr explicit OwnerValue(T&& value) noexcept(std::is_nothrow_move_constructible_v<T>) : value{std::move(value)}
    {
    }

    constexpr OwnerValue(OwnerValue const&) = delete;
    constexpr OwnerValue(OwnerValue&& owner) noexcept : value{std::exchange(owner.value, InvalidValue)} {}

    constexpr OwnerValue& operator=(OwnerValue const&) = delete;
    constexpr OwnerValue& operator=(OwnerValue&& owner) noexcept
    {
        std::swap(value, owner.value);
        return *this;
    }
    constexpr OwnerValue& operator=(T const& value)
    {
        this->value = value;
        return *this;
    }
    constexpr OwnerValue& operator=(T&& value) noexcept(std::is_nothrow_move_assignable_v<T>)
    {
        this->value = std::move(value);
        return *this;
    }

    constexpr T const& get() const { return value; }

    constexpr T& get() { return value; }

    constexpr bool is_valid() const { return (value != InvalidValue); }

    constexpr void reset() { value = InvalidValue; }

    constexpr operator T const&() const { return value; }
    constexpr operator T&() { return value; }

private:
    T value = InvalidValue;
};
}