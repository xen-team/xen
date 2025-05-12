#include "sound_effect.hpp"

#include <tracy/Tracy.hpp>

#include <AL/efx.h>
#include <AL/efx-presets.h>

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

// Effect functions
LPALGENEFFECTS alGenEffects;
LPALDELETEEFFECTS alDeleteEffects;
LPALISEFFECT alIsEffect;
LPALEFFECTI alEffecti;
// LPALEFFECTIV alEffectiv;
LPALEFFECTF alEffectf;
LPALEFFECTFV alEffectfv;
// LPALGETEFFECTI alGetEffecti;
// LPALGETEFFECTIV alGetEffectiv;
// LPALGETEFFECTF alGetEffectf;
// LPALGETEFFECTFV alGetEffectfv;

inline bool load_functions()
{
    PUSH_WARNINGS_STATE
    DISABLE_WARNING_GCC(-Wconditionally - supported)

    alGenEffects = reinterpret_cast<LPALGENEFFECTS>(alGetProcAddress("alGenEffects"));
    alDeleteEffects = reinterpret_cast<LPALDELETEEFFECTS>(alGetProcAddress("alDeleteEffects"));
    alIsEffect = reinterpret_cast<LPALISEFFECT>(alGetProcAddress("alIsEffect"));
    alEffecti = reinterpret_cast<LPALEFFECTI>(alGetProcAddress("alEffecti"));
    // alEffectiv      = reinterpret_cast<LPALEFFECTIV>(alGetProcAddress("alEffectiv"));
    alEffectf = reinterpret_cast<LPALEFFECTF>(alGetProcAddress("alEffectf"));
    alEffectfv = reinterpret_cast<LPALEFFECTFV>(alGetProcAddress("alEffectfv"));
    // alGetEffecti    = reinterpret_cast<LPALGETEFFECTI>(alGetProcAddress("alGetEffecti"));
    // alGetEffectiv   = reinterpret_cast<LPALGETEFFECTIV>(alGetProcAddress("alGetEffectiv"));
    // alGetEffectf    = reinterpret_cast<LPALGETEFFECTF>(alGetProcAddress("alGetEffectf"));
    // alGetEffectfv   = reinterpret_cast<LPALGETEFFECTFV>(alGetProcAddress("alGetEffectfv"));

    POP_WARNINGS_STATE

    return true;
}
}
void SoundEffect::init()
{
    ZoneScopedN("SoundEffect::init");

    if (!alcIsExtensionPresent(alcGetContextsDevice(alcGetCurrentContext()), "ALC_EXT_EFX")) {
        Log::error("[SoundEffect] Sound effects are unavailable.");
        return;
    }

    [[maybe_unused]] static bool const funcs_loaded = load_functions();

    Log::debug("[SoundEffect] Initializing...");

    alGetError(); // Flushing errors

    destroy();

    alGenEffects(1, &index.get());
    check_error("Failed to create a sound effect");

    Log::debug("[SoundEffect] Initialized (ID: " + std::to_string(index) + ")");
}

void SoundEffect::load(ReverberationParams const& params)
{
    alGetError(); // Flushing errors

    if (alGetEnumValue("AL_EFFECT_EAXREVERB") != 0) {
        alEffecti(index, AL_EFFECT_TYPE, AL_EFFECT_EAXREVERB);

        alEffectf(index, AL_EAXREVERB_DENSITY, params.density);
        alEffectf(index, AL_EAXREVERB_DIFFUSION, params.diffusion);
        alEffectf(index, AL_EAXREVERB_GAIN, params.gain);
        alEffectf(index, AL_EAXREVERB_GAINHF, params.gain_high_frequency);
        alEffectf(index, AL_EAXREVERB_GAINLF, params.gain_low_frequency);
        alEffectf(index, AL_EAXREVERB_DECAY_TIME, params.decay_time);
        alEffectf(index, AL_EAXREVERB_DECAY_HFRATIO, params.decay_high_frequency_ratio);
        alEffectf(index, AL_EAXREVERB_DECAY_LFRATIO, params.decay_low_frequency_ratio);
        alEffectf(index, AL_EAXREVERB_REFLECTIONS_GAIN, params.reflections_gain);
        alEffectf(index, AL_EAXREVERB_REFLECTIONS_DELAY, params.reflections_delay);
        alEffectfv(index, AL_EAXREVERB_REFLECTIONS_PAN, params.reflections_pan.data());
        alEffectf(index, AL_EAXREVERB_LATE_REVERB_GAIN, params.late_reverb_gain);
        alEffectf(index, AL_EAXREVERB_LATE_REVERB_DELAY, params.late_reverb_delay);
        alEffectfv(index, AL_EAXREVERB_LATE_REVERB_PAN, params.late_reverb_pan.data());
        alEffectf(index, AL_EAXREVERB_ECHO_TIME, params.echo_time);
        alEffectf(index, AL_EAXREVERB_ECHO_DEPTH, params.echo_depth);
        alEffectf(index, AL_EAXREVERB_MODULATION_TIME, params.modulation_time);
        alEffectf(index, AL_EAXREVERB_MODULATION_DEPTH, params.modulation_depth);
        alEffectf(index, AL_EAXREVERB_AIR_ABSORPTION_GAINHF, params.air_absorption_gain_high_frequency);
        alEffectf(index, AL_EAXREVERB_HFREFERENCE, params.high_frequency_reference);
        alEffectf(index, AL_EAXREVERB_LFREFERENCE, params.low_frequency_reference);
        alEffectf(index, AL_EAXREVERB_ROOM_ROLLOFF_FACTOR, params.room_rolloff_factor);
        alEffecti(index, AL_EAXREVERB_DECAY_HFLIMIT, params.decay_high_frequency_limit);
    }
    else {
        alEffecti(index, AL_EFFECT_TYPE, AL_EFFECT_REVERB);

        alEffectf(index, AL_REVERB_DENSITY, params.density);
        alEffectf(index, AL_REVERB_DIFFUSION, params.diffusion);
        alEffectf(index, AL_REVERB_GAIN, params.gain);
        alEffectf(index, AL_REVERB_GAINHF, params.gain_high_frequency);
        alEffectf(index, AL_REVERB_DECAY_TIME, params.decay_time);
        alEffectf(index, AL_REVERB_DECAY_HFRATIO, params.decay_high_frequency_ratio);
        alEffectf(index, AL_REVERB_REFLECTIONS_GAIN, params.reflections_gain);
        alEffectf(index, AL_REVERB_REFLECTIONS_DELAY, params.reflections_delay);
        alEffectf(index, AL_REVERB_LATE_REVERB_GAIN, params.late_reverb_gain);
        alEffectf(index, AL_REVERB_LATE_REVERB_DELAY, params.late_reverb_delay);
        alEffectf(index, AL_REVERB_AIR_ABSORPTION_GAINHF, params.air_absorption_gain_high_frequency);
        alEffectf(index, AL_REVERB_ROOM_ROLLOFF_FACTOR, params.room_rolloff_factor);
        alEffecti(index, AL_REVERB_DECAY_HFLIMIT, params.decay_high_frequency_limit);
    }

    check_error("Failed to set the reverberation effect");
}

void SoundEffect::load(ChorusParams const& params)
{
    alGetError(); // Flushing errors

    alEffecti(index, AL_EFFECT_TYPE, AL_EFFECT_CHORUS);

    if (alGetError() != AL_NO_ERROR) {
        Log::error("[OpenAL] Failed to set the chorus effect.");
        return;
    }

    alEffecti(index, AL_CHORUS_WAVEFORM, static_cast<int>(params.waveform));
    alEffecti(index, AL_CHORUS_PHASE, params.phase);
    alEffectf(index, AL_CHORUS_RATE, params.rate);
    alEffectf(index, AL_CHORUS_DEPTH, params.depth);
    alEffectf(index, AL_CHORUS_FEEDBACK, params.feedback);
    alEffectf(index, AL_CHORUS_DELAY, params.delay);

    check_error("Failed to set the chorus effect's parameters");
}

void SoundEffect::load(DistortionParams const& params)
{
    alGetError(); // Flushing errors

    alEffecti(index, AL_EFFECT_TYPE, AL_EFFECT_DISTORTION);

    if (alGetError() != AL_NO_ERROR) {
        Log::error("[OpenAL] Failed to set the distortion effect.");
        return;
    }

    alEffectf(index, AL_DISTORTION_EDGE, params.edge);
    alEffectf(index, AL_DISTORTION_GAIN, params.gain);
    alEffectf(index, AL_DISTORTION_LOWPASS_CUTOFF, params.lowpass_cutoff);
    alEffectf(index, AL_DISTORTION_EQCENTER, params.eq_center);
    alEffectf(index, AL_DISTORTION_EQBANDWIDTH, params.eq_bandwidth);

    check_error("Failed to set the distortion effect's parameters");
}

void SoundEffect::load(EchoParams const& params)
{
    alGetError(); // Flushing errors

    alEffecti(index, AL_EFFECT_TYPE, AL_EFFECT_ECHO);

    if (alGetError() != AL_NO_ERROR) {
        Log::error("[OpenAL] Failed to set the echo effect.");
        return;
    }

    alEffectf(index, AL_ECHO_DELAY, params.delay);
    alEffectf(index, AL_ECHO_LRDELAY, params.left_right_delay);
    alEffectf(index, AL_ECHO_DAMPING, params.damping);
    alEffectf(index, AL_ECHO_FEEDBACK, params.feedback);
    alEffectf(index, AL_ECHO_SPREAD, params.spread);

    check_error("Failed to set the echo effect's parameters");
}

void SoundEffect::reset()
{
    alGetError(); // Flushing errors

    alEffecti(index, AL_EFFECT_TYPE, AL_EFFECT_NULL);
    check_error("Failed to reset the effect");
}

void SoundEffect::destroy()
{
    ZoneScopedN("SoundEffect::destroy");

    if (!index.is_valid()) {
        return;
    }

    Log::debug("[SoundEffect] Destroying (ID: " + std::to_string(index) + ")...");

    if (alIsEffect(index)) {
        alDeleteEffects(1, &index.get());
        check_error("Failed to delete sound effect");
    }

    index.reset();

    Log::debug("[SoundEffect] Destroyed");
}

}
