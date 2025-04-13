#include "force.hpp"

namespace xen {
Force::Force(Vector3f const& force, Vector3f const& position) : force(force), position(position), never_expires(true) {}

Force::Force(Vector3f const& force, Time const& time, Vector3f const& position) :
    force(force), position(position), time_left(time), never_expires(false)
{
}

void Force::update(Time const& delta)
{
    time_left -= delta;
}
}