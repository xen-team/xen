#pragma once

#include <data/owner_value.hpp>

namespace xen {
/// Reverberation sound effect parameters. Some require EAX reverberation to be supported by the implementation.
struct ReverberationParams {
    float density = 1.f;   ///< Must be between [0; 1].
    float diffusion = 1.f; ///< Reverberation smoothness; lower is sharper, higher is smoother. Must be between [0; 1].
    float gain = 0.32f;    ///< Reverberation global volume. Must be between [0; 1].
    float gain_high_frequency = 0.89f; ///< High frequencies volume. Must be between [0; 1].
    float gain_low_frequency = 1.f;    ///< Low frequencies volume. Must be between [0; 1]. Requires EAX reverb.
    float decay_time = 1.49f;          ///< Amount of time for the sound to last, in seconds. Must be between [0.1; 20].
    float decay_high_frequency_ratio = 0.83f; ///< Must be between [0.1; 2].
    float decay_low_frequency_ratio = 1.f;    ///< Must be between [0.1; 2]. Requires EAX reverb.
    float reflections_gain = 0.05f;           ///< Initial reverberation volume. Must be between [0; 3.16].
    float reflections_delay =
        0.007f; ///< Pre delay; time to start the initial reverberation from, in seconds. Must be between [0; 0.3].
    std::array<float, 3> reflections_pan = {0.f, 0.f, 0.f}; ///< Initial reverberation position. Requires EAX reverb.
    float late_reverb_gain = 1.26f;                         ///< Late reverberation volume. Must be between [0; 10].
    float late_reverb_delay =
        0.011f; ///< Amount of time to start the late reverberation from, in seconds. Must be between [0; 0.1].
    std::array<float, 3> late_reverb_pan = {0.f, 0.f, 0.f}; ///< Late reverberation position. Requires EAX reverb.
    float echo_time = 0.25f;                                ///< Must be between [0.075; 0.25]. Requires EAX reverb.
    float echo_depth = 0.f;                                 ///< Must be between [0; 1]. Requires EAX reverb.
    float modulation_time = 0.25f;                          ///< Must be between [0.04; 4]. Requires EAX reverb.
    float modulation_depth = 0.f;                           ///< Must be between [0; 1]. Requires EAX reverb.
    float air_absorption_gain_high_frequency = 0.994f;      ///< Must be between [0.892; 1].
    float high_frequency_reference = 5000.f;                ///< Must be between [1000; 20000]. Requires EAX reverb.
    float low_frequency_reference = 250.f;                  ///< Must be between [20; 1000]. Requires EAX reverb.
    float room_rolloff_factor = 0.f;                        ///< Must be between [0; 10].
    bool decay_high_frequency_limit = true;                 ///<
};

enum class SoundWaveform {
    SINUSOID = 0, ///< Sinusoid wave, giving smooth modulations.
    TRIANGLE = 1  ///< Triangle wave, giving sharp modulations.
};

struct ChorusParams {
    SoundWaveform waveform = SoundWaveform::TRIANGLE; ///< Waveform of the effect.
    int phase = 90;                                   ///< Must be between [-180: 180].
    float rate = 1.1f;                                ///< Modulation speed, in Hertz. Must be between [0; 10].
    float depth = 0.1f;                               ///< Modulation frequency range. Must be between [0; 1].
    float feedback = 0.25f;                           ///< Must be between [-1; 1].
    float delay = 0.016f;                             ///< Must be between [0; 0.016].
};

struct DistortionParams {
    float edge = 0.2f;             ///< Must be between [0; 1].
    float gain = 0.05f;            ///< Must be between [0.01; 1].
    float lowpass_cutoff = 8000.f; ///< Must be between [80; 24000].
    float eq_center = 3600.f;      ///< Must be between [80; 24000].
    float eq_bandwidth = 3600.f;   ///< Must be between [80; 24000].
};

struct EchoParams {
    float delay = 0.1f; ///< Delay between each echo, in seconds. Must be between [0; 0.207].
    float left_right_delay =
        0.1f; ///< Delay between left & right echoes, in seconds; 0 disables stereo. Must be between [0; 0.404].
    float damping = 0.5f;  ///< Must be between [0; 0.99].
    float feedback = 0.5f; ///< Falloff ratio of each subsequent echo. Must be between [0; 1].
    float spread = -1.f;   ///< Must be between [-1; 1].
};

class SoundEffect {
public:
    SoundEffect() { init(); }
    SoundEffect(SoundEffect const&) = delete;
    SoundEffect(SoundEffect&&) noexcept = default;

    SoundEffect& operator=(SoundEffect const&) = delete;
    SoundEffect& operator=(SoundEffect&&) noexcept = default;

    ~SoundEffect() { destroy(); }

    [[nodiscard]] uint get_index() const { return index; }

    /// Initializes the sound effect.
    /// \note A SoundEffect must be initialized again after opening an audio device.
    /// \see AudioSystem::open()
    void init();

    /// Loads the given reverberation effect parameters.
    /// \param params Parameters to be loaded.
    void load(ReverberationParams const& params);

    /// Loads the given chorus effect parameters.
    /// \param params Parameters to be loaded.
    void load(ChorusParams const& params);

    /// Loads the given distortion effect parameters.
    /// \param params Parameters to be loaded.
    void load(DistortionParams const& params);

    /// Loads the given echo effect parameters.
    /// \param params Parameters to be loaded.
    void load(EchoParams const& params);

    /// Resets the effect, removing any currently assigned.
    void reset();

    /// Destroys the sound effect.
    void destroy();

private:
    OwnerValue<uint, std::numeric_limits<uint>::max()> index{};
};
}