#include "math.hpp"

namespace xen {
std::mt19937 Math::random_generator(std::random_device{}());
}