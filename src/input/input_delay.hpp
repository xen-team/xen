#pragma once

#include "utils/elapsed_time.hpp"

namespace xen {
class XEN_API InputDelay {
private:
    ElapsedTime elapsed_delay;
    ElapsedTime elapsed_repeat;
    bool delay_over = false;

public:
    explicit InputDelay(Time const& delay = 0.06s, Time const& repeat = Time::seconds(0.06f));

    void update(bool key_is_down);

    bool can_input();

    [[nodiscard]] Time const& get_delay() const { return elapsed_delay.get_interval(); }
    void set_delay(Time const& delay) { elapsed_delay.set_interval(delay); }

    [[nodiscard]] Time const& get_repeat() const { return elapsed_repeat.get_interval(); }
    void set_repeat(Time const& repeat) { elapsed_repeat.set_interval(repeat); }
};
}