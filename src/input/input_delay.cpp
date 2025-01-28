#include "input_delay.hpp"

namespace xen {
InputDelay::InputDelay(Time const& delay, Time const& repeat) : elapsed_delay(delay), elapsed_repeat(repeat) {}

void InputDelay::update(bool key_is_down)
{
    if (key_is_down) {
        delay_over = elapsed_delay.get_elapsed() != 0;
    }
    else {
        delay_over = false;
        elapsed_delay.set_start_time(0s);
        elapsed_repeat.set_start_time(0s);
    }
}

bool InputDelay::can_input()
{
    return delay_over && elapsed_repeat.get_elapsed() != 0;
}
}