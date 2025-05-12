#include "audio_system.hpp"

#include <audio/listener.hpp>
#include <audio/sound.hpp>

#include <math/transform/transform.hpp>
#include <physics/rigid_body.hpp>

#include <tracy/Tracy.hpp>

#include <AL/al.h>
#include <AL/alc.h>

namespace xen {
namespace {
constexpr char const* recover_alc_error_str(int error_code)
{
    switch (error_code) {
    case ALC_INVALID_DEVICE:
        return "Invalid device";
    case ALC_INVALID_CONTEXT:
        return "Invalid context";
    case ALC_INVALID_ENUM:
        return "Invalid enum";
    case ALC_INVALID_VALUE:
        return "Invalid value";
    case ALC_OUT_OF_MEMORY:
        return "Out of memory";
    case ALC_NO_ERROR:
        return "No error";
    default:
        return "Unknown error";
    }
}

inline void check_error(void* device, std::string const& error_msg)
{
    int const error_code = alcGetError(static_cast<ALCdevice*>(device));

    if (error_code != ALC_NO_ERROR) {
        Log::error("[OpenAL] " + error_msg + " (" + recover_alc_error_str(error_code) + ").");
    }
}
}
AudioSystem::AudioSystem(std::string const& device_name)
{
    ZoneScopedN("AudioSystem::AudioSystem");

    register_components<Sound, Listener>();
    open_device(device_name);

    if (device == nullptr || context == nullptr) {
        return;
    }

    std::string_view const al_renderer = alGetString(AL_RENDERER);

    Log::debug("[AudioSystem] OpenAL renderer: ", al_renderer);

#if !defined(XEN_IS_PLATFORM_EMSCRIPTEN) // Emscripten has its own implementation with some OpenAL Soft extensions
    if (al_renderer != "OpenAL Soft") {
        Log::warning("[OpenAL] Standard OpenAL detected; make sure to use OpenAL Soft to get all possible features");
    }
#endif
}

std::vector<std::string> AudioSystem::recover_devices()
{
    if (!alcIsExtensionPresent(
            nullptr, "ALC_ENUMERATE_ALL_EXT"
        )) // If the needed extension is unsupported, return an empty vector
        return {};

    std::vector<std::string> devices;

    // This recovers all devices' names in a single string, each name separated by a null character ('\0'), and ending
    // with two of those For example: "First device\0Second device\0Third device\0\0"
    char const* devices_names = alcGetString(nullptr, ALC_ALL_DEVICES_SPECIFIER);

    while (devices_names[0] != '\0') {
        devices.emplace_back(devices_names); // This automatically fills the string until the first \0
        devices_names += devices.back().size() + 1;
    }

    return devices;
}

void AudioSystem::open_device(std::string const& device_name)
{
    ZoneScopedN("AudioSystem::open_device");

    Log::debug(
        "[AudioSystem] Opening " + (!device_name.empty() ? +"device '" + device_name + '\'' : "default device") + "..."
    );

    destroy();

    device = alcOpenDevice((!device_name.empty() ? device_name.c_str() : nullptr));
    if (!device) {
        Log::error("[OpenAL] Failed to open an audio device.");
        return;
    }

    context = alcCreateContext(static_cast<ALCdevice*>(device), nullptr);
    check_error(device, "Failed to create context");

    if (!alcMakeContextCurrent(static_cast<ALCcontext*>(context))) {
        Log::error("[OpenAL] Failed to make the audio context current.");
        alcGetError(static_cast<ALCdevice*>(device)
        ); // Flushing errors, since alcMakeContextCurrent() produces one on failure, which we already handled
    }

    Log::debug("[AudioSystem] Opened device '" + recover_current_device() + '\'');
}

std::string AudioSystem::recover_current_device() const
{
    if (device == nullptr) { // The system has failed to initialize; returning an empty device name
        return {};
    }

    if (!alcIsExtensionPresent(
            nullptr, "ALC_ENUMERATE_ALL_EXT"
        )) // If the needed extension is unsupported, return an empty string
        return {};

    return alcGetString(static_cast<ALCdevice*>(device), ALC_ALL_DEVICES_SPECIFIER);
}

bool AudioSystem::update(FrameTimeInfo const&)
{
    ZoneScopedN("AudioSystem::update");

#if defined(XEN_CONFIG_DEBUG)
    // Checking that only one Listener exists
    bool has_one_listener = false;
#endif

    for (Entity* entity : entities) {
        if (entity->has_component<Sound>()) {
            Sound const& sound = entity->get_component<Sound>();

            if (entity->has_component<Transform>()) {
                auto& sound_transform = entity->get_component<Transform>();

                // TODO: Transform's update status may be reinitialized in the RenderSystem (and should theoretically be
                // reset in every system, including here)
                //  A viable solution must be implemented to check for and reset this status in all systems
                // if (sound_transform.has_updated()) {
                sound.set_position(sound_transform.get_position());
                // sound_transform.set_updated(false);
                //}
            }

            // TODO: velocity should be set only if it has been updated since last time
            if (entity->has_component<RigidBody>()) {
                sound.set_velocity(entity->get_component<RigidBody>().get_velocity());
            }
        }

        if (entity->has_component<Listener>()) {

#if defined(XEN_CONFIG_DEBUG)
            Log::rt_assert(!has_one_listener, "Error: Only one Listener component must exist in an AudioSystem.");
            has_one_listener = true;
#endif

            Log::rt_assert(
                entity->has_component<Transform>(), "Error: A Listener entity must have a Transform component."
            );

            auto& listener = entity->get_component<Listener>();
            auto& listener_transform = entity->get_component<Transform>();

            // if (listener_transform.has_updated()) {
            listener.set_position(listener_transform.get_position());
            listener.set_orientation(Matrix3(listener_transform.get_rotation().to_rotation_matrix()));

            // listener_transform.set_updated(false);
            //}

            if (entity->has_component<RigidBody>()) {
                listener.set_velocity(entity->get_component<RigidBody>().get_velocity());
            }
        }
    }

    return true;
}

void AudioSystem::destroy()
{
    ZoneScopedN("AudioSystem::destroy");

    if (context == nullptr && device == nullptr) {
        return;
    }

    Log::debug("[AudioSystem] Destroying...");

    alcMakeContextCurrent(nullptr);

    if (context != nullptr) {
        alcDestroyContext(static_cast<ALCcontext*>(context));
        check_error(device, "Failed to destroy context");
        context = nullptr;
    }

    if (device != nullptr) {
        if (!alcCloseDevice(static_cast<ALCdevice*>(device))) {
            Log::error("[OpenAL] Failed to close the audio device.");
        }

        device = nullptr;
    }

    Log::debug("[AudioSystem] Destroyed");
}

}
