#include "elapsed_time.hpp"

#include <cmath>

namespace xen {
ElapsedTime::ElapsedTime(Time const& interval) : start_time(Time::now()), interval(interval) {}

uint32_t ElapsedTime::get_elapsed()
{
    auto const now = Time::now();
    auto const elapsed = static_cast<uint32_t>(std::floor((now - start_time) / interval));

    if (elapsed != 0u) {
        start_time = now;
    }

    return elapsed;
}
}