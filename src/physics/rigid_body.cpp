#include "rigid_body.hpp"

namespace xen {
RigidBody::RigidBody(float mass, float bounciness)
{
    set_mass(mass);
    set_bounciness(bounciness);
}

void RigidBody::set_mass(float mass)
{
    this->mass = mass;
    inv_mass = (this->mass > 0.f ? 1.f / this->mass : 0.f);
}

void RigidBody::set_bounciness(float bounciness)
{
    Log::rt_assert(bounciness >= 0.f && bounciness <= 1.f, "Error: The bounciness value must be between 0 & 1.");
    this->bounciness = bounciness;
}
}