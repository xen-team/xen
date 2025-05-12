#include "bitset.hpp"

namespace xen {
void Bitset::set_bit(size_t index, bool value)
{
    if (index >= bits.size()) {
        bits.resize(index + 1);
    }

    bits[index] = value;
}

Bitset Bitset::operator~() const
{
    Bitset res = *this;

    for (auto bit : res.bits) {
        bit = !bit;
    }

    return res;
}

Bitset Bitset::operator&(Bitset const& bitset) const
{
    Bitset res(std::min(bits.size(), bitset.get_byte_size()));
    std::copy(bits.cbegin(), bits.cbegin() + static_cast<std::ptrdiff_t>(res.get_byte_size()), res.bits.begin());

    res &= bitset;
    return res;
}

Bitset Bitset::operator|(Bitset const& bitset) const
{
    Bitset res(std::min(bits.size(), bitset.get_byte_size()));
    std::copy(bits.cbegin(), bits.cbegin() + static_cast<std::ptrdiff_t>(res.get_byte_size()), res.bits.begin());

    res |= bitset;
    return res;
}

Bitset Bitset::operator^(Bitset const& bitset) const
{
    Bitset res(std::min(bits.size(), bitset.get_byte_size()));
    std::copy(bits.cbegin(), bits.cbegin() + static_cast<std::ptrdiff_t>(res.get_byte_size()), res.bits.begin());

    res ^= bitset;
    return res;
}

Bitset Bitset::operator<<(size_t shift) const
{
    Bitset res = *this;
    res <<= shift;
    return res;
}

Bitset Bitset::operator>>(size_t shift) const
{
    Bitset res = *this;
    res >>= shift;
    return res;
}

Bitset& Bitset::operator&=(Bitset const& bitset)
{
    for (size_t bit_index = 0; bit_index < std::min(bits.size(), bitset.get_byte_size()); ++bit_index) {
        bits[bit_index] = bits[bit_index] && bitset[bit_index];
    }

    return *this;
}

Bitset& Bitset::operator|=(Bitset const& bitset)
{
    for (size_t bit_index = 0; bit_index < std::min(bits.size(), bitset.get_byte_size()); ++bit_index) {
        bits[bit_index] = bits[bit_index] || bitset[bit_index];
    }

    return *this;
}

Bitset& Bitset::operator^=(Bitset const& bitset)
{
    for (size_t bit_index = 0; bit_index < std::min(bits.size(), bitset.get_byte_size()); ++bit_index) {
        bits[bit_index] = bits[bit_index] ^ bitset[bit_index];
    }

    return *this;
}

Bitset& Bitset::operator<<=(size_t shift)
{
    bits.resize(bits.size() + shift);
    return *this;
}

Bitset& Bitset::operator>>=(size_t shift)
{
    bits.resize(bits.size() - shift);
    return *this;
}

std::ostream& operator<<(std::ostream& stream, Bitset const& bitset)
{
    stream << "[ " << bitset[0];

    for (size_t i = 1; i < bitset.get_byte_size(); ++i) {
        stream << ", " << bitset[i];
    }

    stream << " ]";

    return stream;
}
}