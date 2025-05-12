#include "sound.hpp"

#include <audio/sound_effect_slot.hpp>

#include <tracy/Tracy.hpp>

#include <AL/al.h>
#if !defined(XEN_IS_PLATFORM_EMSCRIPTEN)
#include <AL/efx.h>
#endif

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
        Log::verror("[OpenAL] {} ({}).", error_msg, recover_al_error_str(error_code));
}
}
void Sound::init()
{
    ZoneScopedN("Sound::init");

    Log::debug("[Sound] Initializing...");

    alGetError(); // Flushing errors

    destroy();

    Log::debug("[Sound] Creating buffer...");
    alGenBuffers(1, &buffer_index.get());
    check_error("Failed to create a sound buffer");
    Log::debug("[Sound] Created buffer (ID: " + std::to_string(buffer_index) + ")");

    Log::debug("[Sound] Creating source...");
    alGenSources(1, &source_index.get());
    check_error("Failed to create a sound source");
    Log::debug("[Sound] Created source (ID: " + std::to_string(source_index) + ")");

    if (!data.buffer.empty()) {
        load();
    }

    Log::debug("[Sound] Initialized");
}

void Sound::set_pitch(float pitch) const
{
    Log::rt_assert(pitch >= 0.f, "Error: The source's pitch must be positive.");

    alSourcef(source_index, AL_PITCH, pitch);
    check_error("Failed to set the source's pitch");
}

float Sound::recover_pitch() const
{
    float pitch{};

    alGetSourcef(source_index, AL_PITCH, &pitch);
    check_error("Failed to recover the source's pitch");

    return pitch;
}

void Sound::set_gain(float gain) const
{
    Log::rt_assert(gain >= 0.f, "Error: The source's gain must be positive.");

    alSourcef(source_index, AL_GAIN, gain);
    check_error("Failed to set the source's gain");
}

float Sound::recover_gain() const
{
    float gain{};

    alGetSourcef(source_index, AL_GAIN, &gain);
    check_error("Failed to recover the source's gain");

    return gain;
}

void Sound::set_position(Vector3f const& position) const
{
    alSource3f(source_index, AL_POSITION, position.x, position.y, position.z);
    check_error("Failed to set the source's position");
}

Vector3f Sound::recover_position() const
{
    Vector3f position;

    alGetSource3f(source_index, AL_POSITION, &position.x, &position.y, &position.z);
    check_error("Failed to recover the source's position");

    return position;
}

void Sound::set_velocity(Vector3f const& velocity) const
{
    alSource3f(source_index, AL_VELOCITY, velocity.x, velocity.y, velocity.z);
    check_error("Failed to set the source's velocity");
}

Vector3f Sound::recover_velocity() const
{
    Vector3f velocity;

    alGetSource3f(source_index, AL_VELOCITY, &velocity.x, &velocity.y, &velocity.z);
    check_error("Failed to recover the source's velocity");

    return velocity;
}

#if !defined(XEN_IS_PLATFORM_EMSCRIPTEN)
void Sound::link_slot(const SoundEffectSlot& slot) const
{
    alSource3i(source_index, AL_AUXILIARY_SEND_FILTER, static_cast<int>(slot.get_index()), 0, AL_FILTER_NULL);
    check_error("Failed to link the sound effect slot to the sound");
}

void Sound::unlink_slot() const
{
    alSource3i(source_index, AL_AUXILIARY_SEND_FILTER, 0, 0, AL_FILTER_NULL);
    check_error("Failed to unlink the sound effect slot from the sound");
}
#endif

void Sound::set_repeat(bool repeat) const
{
    alSourcei(source_index, AL_LOOPING, repeat);
    check_error("Failed to change the sound's repeat state");
}

void Sound::play() const
{
    if (is_playing()) {
        return;
    }

    alSourcePlay(source_index);
    check_error("Failed to play/resume the sound");
}

void Sound::pause() const
{
    alSourcePause(source_index);
    check_error("Failed to pause the sound");
}

void Sound::stop() const
{
    alSourceStop(source_index);
    check_error("Failed to stop the sound");
}

void Sound::rewind() const
{
    alSourceRewind(source_index);
    check_error("Failed to rewind the sound");
}

SoundState Sound::recover_state() const
{
    int state{};
    alGetSourcei(source_index, AL_SOURCE_STATE, &state);

    return static_cast<SoundState>(state);
}

float Sound::recover_elapsed_time() const
{
    float seconds{};
    alGetSourcef(source_index, AL_SEC_OFFSET, &seconds);

    return (seconds / 60.f);
}

void Sound::destroy()
{
    ZoneScopedN("Sound::destroy");

    if (!source_index.is_valid() && !buffer_index.is_valid()) {
        return;
    }

    Log::debug("[Sound] Destroying...");

    if (source_index.is_valid() && alIsSource(source_index)) {
        Log::debug("[Sound] Destroying source (ID: " + std::to_string(source_index) + ")...");

        alDeleteSources(1, &source_index.get());
        check_error("Failed to delete source");

        Log::debug("[Sound] Destroyed source");
    }

    source_index.reset();

    if (buffer_index.is_valid() && alIsBuffer(buffer_index)) {
        Log::debug("[Sound] Destroying buffer (ID: " + std::to_string(buffer_index) + ")...");

        alDeleteBuffers(1, &buffer_index.get());
        check_error("Failed to delete buffer");

        Log::debug("[Sound] Destroyed buffer");
    }

    buffer_index.reset();

    Log::debug("[Sound] Destroyed");
}

void Sound::load()
{
    ZoneScopedN("Sound::load");

    Log::vdebug("[Sound] data... {} | {} | {}", data.buffer.size(), static_cast<int>(data.format), data.frequency);

    stop();                                // Making sure the sound isn't paused or currently playing
    alSourcei(source_index, AL_BUFFER, 0); // Detaching the previous buffer (if any) from the source

    if ((data.format == AudioFormat::MONO_F32 || data.format == AudioFormat::STEREO_F32) &&
        !alIsExtensionPresent("AL_EXT_float32")) {
        Log::error("[Sound] Float audio format is not supported by the audio driver.");
        return;
    }

    if ((data.format == AudioFormat::MONO_F64 || data.format == AudioFormat::STEREO_F64) &&
        !alIsExtensionPresent("AL_EXT_double")) {
        Log::error("[Sound] Double audio format is not supported by the audio driver.");
        return;
    }

    alBufferData(
        buffer_index, static_cast<int>(data.format), data.buffer.data(), static_cast<int>(data.buffer.size()),
        static_cast<int>(data.frequency)
    );
    check_error("Failed to send audio data to the buffer");

    alSourcei(source_index, AL_BUFFER, static_cast<int>(buffer_index));
    check_error("Failed to map the sound buffer to the source");
}

}
