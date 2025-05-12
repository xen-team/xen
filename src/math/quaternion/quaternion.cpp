#include "quaternion.hpp"

namespace xen {
Quaternion const Quaternion::Identity(0.0f, 0.0f, 0.0f, 1.0f);
Quaternion const Quaternion::Zero(0.0f, 0.0f, 0.0f, 0.0f);
Quaternion const Quaternion::One(1.0f, 1.0f, 1.0f, 1.0f);
Quaternion const Quaternion::PositiveInfinity(
    +std::numeric_limits<value_type>::infinity(), +std::numeric_limits<value_type>::infinity(),
    +std::numeric_limits<value_type>::infinity(), +std::numeric_limits<value_type>::infinity()
);
Quaternion const Quaternion::NegativeInfinity(
    -std::numeric_limits<value_type>::infinity(), -std::numeric_limits<value_type>::infinity(),
    -std::numeric_limits<value_type>::infinity(), -std::numeric_limits<value_type>::infinity()
);
}