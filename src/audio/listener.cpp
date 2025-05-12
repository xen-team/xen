#include <audio/listener.hpp>

#include <AL/al.h>

namespace xen {
namespace {
constexpr char const* recover_al_error_str(int error_code)
{
    switch (error_code) {
    case AL_INVALID_NAME:
        return "Invalid name";
    case AL_INVALID_ENUM:
        return "Invalid enum";
    case AL_INVALID_VALUE:
        return "Invalid value";
    case AL_INVALID_OPERATION:
        return "Invalid operation";
    case AL_OUT_OF_MEMORY:
        return "Out of memory";
    case AL_NO_ERROR:
        return "No error";
    default:
        return "Unknown error";
    }
}

inline void check_error(std::string const& error_msg)
{
    int const error_code = alGetError();

    if (error_code != AL_NO_ERROR)
        Log::error("[OpenAL] " + error_msg + " (" + recover_al_error_str(error_code) + ").");
}
}
Listener::Listener(Vector3f const& position, Vector3f const& forwardDir, Vector3f const& upDir)
{
    set_position(position);
    set_orientation(forwardDir, upDir);
}

Listener::Listener(Vector3f const& position, Matrix3 const& rotation)
{
    set_position(position);
    set_orientation(rotation);
}

void Listener::set_gain(float gain) const
{
    Log::rt_assert(gain >= 0.f, "Error: The listener's gain must be positive.");

    alListenerf(AL_GAIN, gain);
    check_error("Failed to set the listener's gain");
}

float Listener::recover_gain() const
{
    float gain{};

    alGetListenerf(AL_GAIN, &gain);
    check_error("Failed to recover the listener's gain");

    return gain;
}

void Listener::set_position(Vector3f const& position) const
{
    alListener3f(AL_POSITION, position.x, position.y, position.z);
    check_error("Failed to set the listener's position");
}

Vector3f Listener::recover_position() const
{
    Vector3f position;

    alGetListener3f(AL_POSITION, &position.x, &position.y, &position.z);
    check_error("Failed to recover the listener's position");

    return position;
}

void Listener::set_velocity(Vector3f const& velocity) const
{
    alListener3f(AL_VELOCITY, velocity.x, velocity.y, velocity.z);
    check_error("Failed to set the listener's velocity");
}

Vector3f Listener::recover_velocity() const
{
    Vector3f velocity;

    alGetListener3f(AL_VELOCITY, &velocity.x, &velocity.y, &velocity.z);
    check_error("Failed to recover the listener's velocity");

    return velocity;
}

void Listener::set_orientation(Vector3f const& forward_direction) const
{
    set_orientation(forward_direction, Vector3f::Up);
}

void Listener::set_orientation(Vector3f const& forward_direction, Vector3f const& up_direction) const
{
    Log::rt_assert(
        Math::almost_equal(forward_direction.length(), 1.f),
        "Error: The Listener's forward direction must be normalized."
    );
    Log::rt_assert(
        Math::almost_equal(up_direction.length(), 1.f), "Error: The Listener's up direction must be normalized."
    );

    std::array<float, 6> const orientation = {forward_direction.x, forward_direction.y, forward_direction.z,
                                              up_direction.x,      up_direction.y,      up_direction.z};

    alListenerfv(AL_ORIENTATION, &orientation[0]);
    check_error("Failed to set the listener's orientation");
}

void Listener::set_orientation(Matrix3 const& rotation) const
{
    set_orientation(-rotation[2].normalize(), rotation[1].normalize());
}

std::pair<Vector3f, Vector3f> Listener::recover_orientation() const
{
    std::array<float, 6> orientation;

    alGetListenerfv(AL_ORIENTATION, &orientation[0]);
    check_error("Failed to recover the listener's orientation");

    return {
        Vector3f(orientation[0], orientation[1], orientation[2]),
        Vector3f(orientation[3], orientation[4], orientation[5])
    };
}

Vector3f Listener::recover_forward_orientation() const
{
    return recover_orientation().first;
}

Vector3f Listener::recover_up_orientation() const
{
    return recover_orientation().second;
}

}
