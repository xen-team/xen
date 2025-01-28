#pragma once

#include "core.hpp"
#include "time.hpp"

namespace xen {
class XEN_API ElapsedTime {
private:
    Time start_time;
    Time interval;

public:
    explicit ElapsedTime(Time const& interval = -1s);

    uint32_t get_elapsed();

    [[nodiscard]] Time const& get_start_time() const { return start_time; }
    void set_start_time(Time const& startTime) { this->start_time = startTime; }

    [[nodiscard]] Time const& get_interval() const { return interval; }
    void set_interval(Time const& interval) { this->interval = interval; }
};
}