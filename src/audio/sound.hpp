#pragma once

#include "rocket.hpp"
#include <component.hpp>
#include <audio/audio_data.hpp>
#include <data/owner_value.hpp>

namespace xen {
class SoundEffectSlot;

enum class SoundState : int {
    INITIAL = 4113 /* AL_INITIAL */, ///< Initial state, nothing is happening.
    PLAYING = 4114 /* AL_PLAYING */, ///< Being played.
    PAUSED = 4115 /* AL_PAUSED  */,  ///< Paused.
    STOPPED = 4116 /* AL_STOPPED */  ///< Stopped.
};

class Sound final : public Component {
public:
    Sound() { init(); }
    explicit Sound(AudioData data) : Sound() { load(std::move(data)); }
    Sound(Sound const&) = delete;
    Sound(Sound&&) noexcept = default;

    Sound& operator=(Sound const&) = delete;
    Sound& operator=(Sound&&) noexcept = default;

    ~Sound() override { destroy(); }

    constexpr uint32_t get_buffer_index() const { return buffer_index; }

    constexpr AudioData const& get_data() const { return data; }

    /// Initializes the sound. If there is audio data, also loads it into memory.
    /// \note A Sound must be initialized again after opening an audio device.
    /// \see AudioSystem::open()
    void init();

    /// Loads the given audio data into memory.
    /// @param data Data to be loaded.
    void load(AudioData data)
    {
        this->data = std::move(data);
        load();
    }

    /// Sets the sound's pitch multiplier.
    /// \param pitch Sound's pitch multiplier; must be positive. 1 is the default.
    void set_pitch(float pitch) const;

    /// Recovers the sound's pitch multiplier.
    /// \return Sound's pitch multiplier.
    float recover_pitch() const;

    /// Sets the sound's gain (volume).
    /// \param gain Sound's gain; must be positive. 1 is the default.
    void set_gain(float gain) const;

    /// Recovers the source's gain (volume).
    /// \return Source's gain.
    float recover_gain() const;

    /// Sets the audio source's position.
    /// \note Note that positional audio will only be effective with sounds having a mono format.
    /// \param position New source's position.
    void set_position(Vector3f const& position) const;

    /// Recovers the position of the sound emitter.
    /// \return Sound's position.
    Vector3f recover_position() const;

    /// Sets the audio source's velocity.
    /// \param velocity New source's velocity.
    void set_velocity(Vector3f const& velocity) const;

    /// Recovers the velocity of the sound emitter.
    /// \return Sound's velocity.
    Vector3f recover_velocity() const;

#if !defined(__EMSCRIPTEN__)
    /// Links a sound effect slot to the current sound.
    /// \param slot Slot to be linked.
    void link_slot(SoundEffectSlot const& slot) const;

    /// Unlinks any sound effect slot from the current sound.
    void unlink_slot() const;
#endif

    /// Sets the sound's repeat state.
    /// \param repeat Repeat state; true if the sound should be repeated, false otherwise.
    void set_repeat(bool repeat) const;

    /// Plays the sound.
    void play() const;

    /// Pauses the sound.
    void pause() const;

    /// Stops the sound.
    void stop() const;

    /// Rewinds the sound.
    void rewind() const;

    /// Recovers the current state of the sound.
    /// \return Sound's state.
    SoundState recover_state() const;

    /// Checks if the sound is currently being played.
    /// \return True if the sound is being played, false otherwise.
    /// \see recover_state()
    bool is_playing() const { return (recover_state() == SoundState::PLAYING); }

    /// Checks if the sound is currently paused.
    /// \return True if the sound is paused, false otherwise.
    /// \see recover_state()
    bool is_paused() const { return (recover_state() == SoundState::PAUSED); }

    /// Checks if the sound is currently stopped.
    /// \return True if the sound is stopped, false otherwise.
    /// \see recover_state()
    bool is_stopped() const { return (recover_state() == SoundState::STOPPED); }

    /// Recovers the amount of minutes the sound has been played so far.
    /// \return Sound's elapsed time, in minutes.
    float recover_elapsed_time() const;

    /// Destroys the sound.
    void destroy();

private:
    OwnerValue<uint32_t, std::numeric_limits<uint32_t>::max()> buffer_index{};
    OwnerValue<uint32_t, std::numeric_limits<uint32_t>::max()> source_index{};

    AudioData data{};

private:
    /// Loads the audio data into memory.
    void load();
};
}