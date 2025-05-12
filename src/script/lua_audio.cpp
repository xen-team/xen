#include <audio/audio_system.hpp>
#include <audio/listener.hpp>
#include <audio/microphone.hpp>
#include <audio/sound.hpp>
#include <audio/sound_effect.hpp>
#include <audio/sound_effect_slot.hpp>

#include <script/lua_wrapper.hpp>
#include <utils/type_utils.hpp>

#define SOL_ALL_SAFETIES_ON 1
#include "sol/sol.hpp"

namespace xen {

using namespace TypeUtils;

void LuaWrapper::register_audio_types()
{
    sol::state& state = get_state();

    {
        sol::usertype<AudioData> audio_data =
            state.new_usertype<AudioData>("AudioData", sol::constructors<AudioData()>());
        audio_data["format"] = &AudioData::format;
        audio_data["frequency"] = &AudioData::frequency;
        audio_data["buffer"] = &AudioData::buffer;

        state.new_enum<AudioFormat>(
            "AudioFormat", {{"MONO_U8", AudioFormat::MONO_U8},
                            {"STEREO_U8", AudioFormat::STEREO_U8},
                            {"MONO_I16", AudioFormat::MONO_I16},
                            {"STEREO_I16", AudioFormat::STEREO_I16},
                            {"MONO_F32", AudioFormat::MONO_F32},
                            {"STEREO_F32", AudioFormat::STEREO_F32},
                            {"MONO_F64", AudioFormat::MONO_F64},
                            {"STEREO_F64", AudioFormat::STEREO_F64}}
        );
    }

    {
        sol::usertype<AudioSystem> audioSystem = state.new_usertype<AudioSystem>(
            "AudioSystem", sol::constructors<AudioSystem(), AudioSystem(std::string const&)>(), sol::base_classes,
            sol::bases<System>()
        );
        audioSystem["recover_devices"] = &AudioSystem::recover_devices;
        audioSystem["open_device"] = sol::overload(
            [](AudioSystem& s) { s.open_device(); }, PickOverload<std::string const&>(&AudioSystem::open_device)
        );
        audioSystem["recover_current_device"] = &AudioSystem::recover_current_device;
    }

    {
        sol::usertype<Listener> listener = state.new_usertype<Listener>(
            "Listener",
            sol::constructors<
                Listener(), Listener(Vector3f const&), Listener(Vector3f const&, Vector3f const&, Vector3f const&),
                Listener(Vector3f const&, Matrix3 const&)>(),
            sol::base_classes, sol::bases<Component>()
        );
        listener["gain"] = sol::property(&Listener::recover_gain, &Listener::set_gain);
        listener["position"] = sol::property(&Listener::recover_position, &Listener::set_position);
        listener["velocity"] = sol::property(&Listener::recover_velocity, &Listener::set_velocity);
        listener["set_orientation"] = sol::overload(
            PickOverload<Vector3f const&>(&Listener::set_orientation),
            PickOverload<Vector3f const&, Vector3f const&>(&Listener::set_orientation),
            PickOverload<Matrix3 const&>(&Listener::set_orientation)
        );
        listener["recover_orientation"] = &Listener::recover_orientation;
        listener["recover_forward_orientation"] = &Listener::recover_forward_orientation;
        listener["recover_up_orientation"] = &Listener::recover_up_orientation;
    }

    {
        sol::usertype<Microphone> microphone = state.new_usertype<Microphone>(
            "Microphone",
            sol::constructors<
                Microphone(AudioFormat, uint, float), Microphone(AudioFormat, uint, float, std::string const&)>()
        );
        microphone["recover_devices"] = &Microphone::recover_devices;
        microphone["open_device"] = sol::overload(
            [](Microphone& m, AudioFormat fmt, uint freq, float d) { m.open_device(fmt, freq, d); },
            PickOverload<AudioFormat, uint, float, std::string const&>(&Microphone::open_device)
        );
        microphone["recover_current_device"] = &Microphone::recover_current_device;
        microphone["start"] = &Microphone::start;
        microphone["stop"] = &Microphone::stop;
        microphone["recover_available_sample_count"] = &Microphone::recover_available_sample_count;
        microphone["recover_available_duration"] = &Microphone::recover_available_duration;
        microphone["recover_data"] = sol::overload(
            [](Microphone const& m) { return m.recover_data(); }, PickOverload<float>(&Microphone::recover_data)
        );
    }

    {
        sol::usertype<Sound> sound = state.new_usertype<Sound>(
            "Sound", sol::constructors<Sound(), Sound(AudioData)>(), sol::base_classes, sol::bases<Component>()
        );
        sound["get_buffer_index"] = &Sound::get_buffer_index;
        sound["get_data"] = &Sound::get_data;
        sound["init"] = &Sound::init;
        sound["load"] = PickOverload<AudioData>(&Sound::load);
        sound["pitch"] = sol::property(&Sound::recover_pitch, &Sound::set_pitch);
        sound["gain"] = sol::property(&Sound::recover_gain, &Sound::set_gain);
        sound["position"] =
            sol::property(&Sound::recover_position, PickOverload<Vector3f const&>(&Sound::set_position));
        sound["velocity"] =
            sol::property(&Sound::recover_velocity, PickOverload<Vector3f const&>(&Sound::set_velocity));
#if !defined(XEN_IS_PLATFORM_EMSCRIPTEN)
        sound["link_slot"] = &Sound::link_slot;
        sound["unlink_slot"] = &Sound::unlink_slot;
#endif
        sound["set_repeat"] = &Sound::set_repeat;
        sound["play"] = &Sound::play;
        sound["pause"] = &Sound::pause;
        sound["stop"] = &Sound::stop;
        sound["rewind"] = &Sound::rewind;
        sound["is_playing"] = &Sound::is_playing;
        sound["is_paused"] = &Sound::is_paused;
        sound["is_stopped"] = &Sound::is_stopped;
        sound["recover_elapsed_time"] = &Sound::recover_elapsed_time;
    }

#if !defined(XEN_IS_PLATFORM_EMSCRIPTEN)
    {
        sol::usertype<SoundEffect> soundEffect =
            state.new_usertype<SoundEffect>("SoundEffect", sol::constructors<SoundEffect()>());
        soundEffect["get_index"] = &SoundEffect::get_index;
        soundEffect["init"] = &SoundEffect::init;
        soundEffect["load"] = sol::overload(
            PickOverload<ReverberationParams const&>(&SoundEffect::load),
            PickOverload<ChorusParams const&>(&SoundEffect::load),
            PickOverload<DistortionParams const&>(&SoundEffect::load),
            PickOverload<EchoParams const&>(&SoundEffect::load)
        );
        soundEffect["reset"] = &SoundEffect::reset;
        soundEffect["destroy"] = &SoundEffect::destroy;

        // Effects
        {
            sol::usertype<ReverberationParams> reverbParams = state.new_usertype<ReverberationParams>(
                "ReverberationParams", sol::constructors<ReverberationParams()>()
            );
            reverbParams["density"] = &ReverberationParams::density;
            reverbParams["diffusion"] = &ReverberationParams::diffusion;
            reverbParams["gain"] = &ReverberationParams::gain;
            reverbParams["gain_high_frequency"] = &ReverberationParams::gain_high_frequency;
            reverbParams["gain_low_frequency"] = &ReverberationParams::gain_low_frequency;
            reverbParams["decay_time"] = &ReverberationParams::decay_time;
            reverbParams["decay_high_frequency_ratio"] = &ReverberationParams::decay_high_frequency_ratio;
            reverbParams["decay_low_frequency_ratio"] = &ReverberationParams::decay_low_frequency_ratio;
            reverbParams["reflections_gain"] = &ReverberationParams::reflections_gain;
            reverbParams["reflections_delay"] = &ReverberationParams::reflections_delay;
            reverbParams["reflections_pan"] = &ReverberationParams::reflections_pan;
            reverbParams["late_reverb_gain"] = &ReverberationParams::late_reverb_gain;
            reverbParams["late_reverb_delay"] = &ReverberationParams::late_reverb_delay;
            reverbParams["late_reverb_pan"] = &ReverberationParams::late_reverb_pan;
            reverbParams["echo_time"] = &ReverberationParams::echo_time;
            reverbParams["echo_depth"] = &ReverberationParams::echo_depth;
            reverbParams["modulation_time"] = &ReverberationParams::modulation_time;
            reverbParams["modulation_depth"] = &ReverberationParams::modulation_depth;
            reverbParams["air_absorption_gain_high_frequency"] =
                &ReverberationParams::air_absorption_gain_high_frequency;
            reverbParams["high_frequency_reference"] = &ReverberationParams::high_frequency_reference;
            reverbParams["low_frequency_reference"] = &ReverberationParams::low_frequency_reference;
            reverbParams["room_rolloff_factor"] = &ReverberationParams::room_rolloff_factor;
            reverbParams["decay_high_frequency_limit"] = &ReverberationParams::decay_high_frequency_limit;

            state.new_enum<SoundWaveform>(
                "SoundWaveform", {{"SINUSOID", SoundWaveform::SINUSOID}, {"TRIANGLE", SoundWaveform::TRIANGLE}}
            );

            sol::usertype<ChorusParams> chorusParams =
                state.new_usertype<ChorusParams>("ChorusParams", sol::constructors<ChorusParams()>());
            chorusParams["waveform"] = &ChorusParams::waveform;
            chorusParams["phase"] = &ChorusParams::phase;
            chorusParams["rate"] = &ChorusParams::rate;
            chorusParams["depth"] = &ChorusParams::depth;
            chorusParams["feedback"] = &ChorusParams::feedback;
            chorusParams["delay"] = &ChorusParams::delay;

            sol::usertype<DistortionParams> distortionParams =
                state.new_usertype<DistortionParams>("DistortionParams", sol::constructors<DistortionParams()>());
            distortionParams["edge"] = &DistortionParams::edge;
            distortionParams["gain"] = &DistortionParams::gain;
            distortionParams["lowpass_cutoff"] = &DistortionParams::lowpass_cutoff;
            distortionParams["eq_center"] = &DistortionParams::eq_center;
            distortionParams["eq_bandwidth"] = &DistortionParams::eq_bandwidth;

            sol::usertype<EchoParams> echoParams =
                state.new_usertype<EchoParams>("EchoParams", sol::constructors<EchoParams()>());
            echoParams["delay"] = &EchoParams::delay;
            echoParams["left_right_delay"] = &EchoParams::left_right_delay;
            echoParams["damping"] = &EchoParams::damping;
            echoParams["feedback"] = &EchoParams::feedback;
            echoParams["spread"] = &EchoParams::spread;
        }
    }

    {
        sol::usertype<SoundEffectSlot> soundEffectSlot =
            state.new_usertype<SoundEffectSlot>("SoundEffectSlot", sol::constructors<SoundEffectSlot()>());
        soundEffectSlot["get_index"] = &SoundEffectSlot::get_index;
        soundEffectSlot["init"] = &SoundEffectSlot::init;
        soundEffectSlot["load_effect"] = &SoundEffectSlot::load_effect;
        soundEffectSlot["destroy"] = &SoundEffectSlot::destroy;
    }
#endif
}

}
