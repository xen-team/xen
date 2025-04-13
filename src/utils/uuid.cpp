#include "uuid.hpp"

#include "math/math.hpp"

namespace xen {
UUID::UUID() : uuid(Math::random(1ul, std::numeric_limits<uint64_t>::max())) {}

UUID::UUID(uint64_t uuid) : uuid(uuid) {}
}