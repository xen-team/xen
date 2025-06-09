#pragma once

namespace xen {
class Bitset {
public:
    Bitset() = default;

    explicit Bitset(size_t bit_count, bool init_value = false) : bits(bit_count, init_value) {}

    Bitset(std::initializer_list<bool> values) : bits(values) {}

    size_t get_byte_size() const { return bits.size(); }

    bool empty() const { return (std::find(bits.cbegin(), bits.cend(), true) == bits.cend()); }

    size_t get_enabled_bit_count() const { return static_cast<size_t>(std::count(bits.cbegin(), bits.cend(), true)); }

    size_t get_disabled_bit_count() const { return (bits.size() - get_enabled_bit_count()); }

    void set_bit(size_t index, bool value = true);

    void resize(size_t new_size) { bits.resize(new_size); }

    void reset() { std::fill(bits.begin(), bits.end(), false); }

    void clear() { bits.clear(); }

    Bitset operator~() const;
    Bitset operator&(Bitset const& bitset) const;
    Bitset operator|(Bitset const& bitset) const;
    Bitset operator^(Bitset const& bitset) const;
    Bitset operator<<(size_t shift) const;
    Bitset operator>>(size_t shift) const;
    Bitset& operator&=(Bitset const& bitset);
    Bitset& operator|=(Bitset const& bitset);
    Bitset& operator^=(Bitset const& bitset);
    Bitset& operator<<=(size_t shift);
    Bitset& operator>>=(size_t shift);
    bool operator[](size_t index) const { return bits[index]; }
    bool operator==(Bitset const& bitset) const { return std::equal(bits.cbegin(), bits.cend(), bitset.bits.cbegin()); }
    bool operator!=(Bitset const& bitset) const { return !(*this == bitset); }
    friend std::ostream& operator<<(std::ostream& stream, Bitset const& bitset);

private:
    std::vector<bool> bits{};
};
}