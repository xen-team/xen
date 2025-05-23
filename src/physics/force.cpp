#include "force.hpp"

namespace xen {
Force::Force(Vector3f const& force, Vector3f const& position) : force(force), position(position), never_expires(true) {}

Force::Force(Vector3f const& force, float const& time, Vector3f const& position) :
    force(force), position(position), time_left(time), never_expires(false)
{
}

void Force::update(float const& delta)
{
    time_left -= delta;
}
}