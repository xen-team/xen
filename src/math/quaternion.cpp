#include "quaternion.hpp"

namespace xen {
Quaternion const Quaternion::zero(0.0f, 0.0f, 0.0f, 0.0f);
Quaternion const Quaternion::one(1.0f, 1.0f, 1.0f, 1.0f);
Quaternion const Quaternion::positive_infinity(
    +std::numeric_limits<value_type>::infinity(), +std::numeric_limits<value_type>::infinity(),
    +std::numeric_limits<value_type>::infinity(), +std::numeric_limits<value_type>::infinity()
);
Quaternion const Quaternion::negative_infinity(
    -std::numeric_limits<value_type>::infinity(), -std::numeric_limits<value_type>::infinity(),
    -std::numeric_limits<value_type>::infinity(), -std::numeric_limits<value_type>::infinity()
);
}