#pragma once

#include <cstdint>

namespace xen {
class UUID {
private:
    uint64_t uuid;

public:
    UUID();
    UUID(uint64_t uuid);

    UUID(const UUID& other) = default;
    UUID& operator=(const UUID& other) = default;
    ~UUID() = default;

    operator uint64_t() const { return uuid; }

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(uuid);
    }
};
}

namespace std {
template <>
struct hash<xen::UUID> {
    std::size_t operator()(xen::UUID const& uuid) const { return hash<uint64_t>()((uint64_t)uuid); }
};
}