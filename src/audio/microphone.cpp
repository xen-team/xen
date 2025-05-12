#include "microphone.hpp"

#include <audio/sound.hpp>

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

constexpr int recover_frame_size(AudioFormat format)
{
    uint8_t channel_count{};
    uint8_t bit_count{};

    switch (format) {
    case AudioFormat::MONO_U8:
    case AudioFormat::MONO_I16:
    case AudioFormat::MONO_F32:
    case AudioFormat::MONO_F64:
        channel_count = 1;
        break;

    case AudioFormat::STEREO_U8:
    case AudioFormat::STEREO_I16:
    case AudioFormat::STEREO_F32:
    case AudioFormat::STEREO_F64:
        channel_count = 2;
        break;

    default:
        throw std::invalid_argument("[Microphone] Unhandled audio format");
    }

    switch (format) {
    case AudioFormat::MONO_U8:
    case AudioFormat::STEREO_U8:
        bit_count = 8;
        break;

    case AudioFormat::MONO_I16:
    case AudioFormat::STEREO_I16:
        bit_count = 16;
        break;

    case AudioFormat::MONO_F32:
    case AudioFormat::STEREO_F32:
        bit_count = 32;
        break;

    case AudioFormat::MONO_F64:
    case AudioFormat::STEREO_F64:
        bit_count = 64;
        break;

    default:
        throw std::invalid_argument("[Microphone] Unhandled audio format");
    }

    return channel_count * bit_count / 8;
}
}
Microphone::Microphone(AudioFormat format, uint frequency, float duration, std::string const& device_name)
{
    ZoneScopedN("Microphone::Microphone");
    open_device(format, frequency, duration, device_name);
}

std::vector<std::string> Microphone::recover_devices()
{
    if (!alcIsExtensionPresent(
            nullptr, "ALC_ENUMERATE_ALL_EXT"
        )) // If the needed extension is unsupported, return an empty vector
        return {};

    std::vector<std::string> devices;

    // This recovers all devices' names in a single string, each name separated by a null character ('\0'), and ending
    // with two of those For example: "First device\0Second device\0Third device\0\0"
    char const* devices_names = alcGetString(nullptr, ALC_CAPTURE_DEVICE_SPECIFIER);

    while (devices_names[0] != '\0') {
        devices.emplace_back(devices_names); // This automatically fills the string until the first \0
        devices_names += devices.back().size() + 1;
    }

    return devices;
}

void Microphone::open_device(AudioFormat format, uint frequency, float duration, std::string const& device_name)
{
    ZoneScopedN("Microphone::open_device");

    Log::debug(
        "[Microphone] Opening capture " + (!device_name.empty() ? +"device '" + device_name + '\'' : "default device") +
        "..."
    );

    destroy();

    if ((format == AudioFormat::MONO_F32 || format == AudioFormat::STEREO_F32) &&
        !alIsExtensionPresent("AL_EXT_float32")) {
        Log::error("[Microphone] Float audio capture format is not supported by the audio driver.");
        return;
    }

    if ((format == AudioFormat::MONO_F64 || format == AudioFormat::STEREO_F64) &&
        !alIsExtensionPresent("AL_EXT_double")) {
        Log::error("[Microphone] Double audio capture format is not supported by the audio driver.");
        return;
    }

    device = alcCaptureOpenDevice(
        (!device_name.empty() ? device_name.c_str() : nullptr), frequency, static_cast<int>(format),
        static_cast<int>(duration * static_cast<float>(frequency))
    );
    if (!device) {
        Log::error("[OpenAL] Failed to open an audio capture device.");
        return;
    }

    this->format = format;
    this->frequency = frequency;

    Log::debug("[Microphone] Opened capture device '" + recover_current_device() + '\'');
}

std::string Microphone::recover_current_device() const
{
    if (device == nullptr) { // The system has failed to initialize; returning an empty device name
        return {};
    }

    if (!alcIsExtensionPresent(
            nullptr, "ALC_ENUMERATE_ALL_EXT"
        )) // If the needed extension is unsupported, return an empty string
        return {};

    return alcGetString(static_cast<ALCdevice*>(device), ALC_CAPTURE_DEVICE_SPECIFIER);
}

void Microphone::start() const
{
    alcCaptureStart(static_cast<ALCdevice*>(device));
}

void Microphone::stop() const
{
    alcCaptureStop(static_cast<ALCdevice*>(device));
}

int Microphone::recover_available_sample_count() const
{
    if (device == nullptr) {
        return 0;
    }

    int count{};
    alcGetIntegerv(static_cast<ALCdevice*>(device), ALC_CAPTURE_SAMPLES, 1, &count);

    return count;
}

float Microphone::recover_available_duration() const
{
    return (static_cast<float>(recover_available_sample_count()) / static_cast<float>(frequency));
}

AudioData Microphone::recover_data(float max_duration) const
{
    AudioData data{};
    recover_data(data, max_duration);

    return data;
}

void Microphone::recover_data(AudioData& data, float max_duration) const
{
    ZoneScopedN("Microphone::recover_data");

    data.format = format;
    data.frequency = frequency;
    data.buffer.clear();

    if (max_duration == 0.f) {
        return;
    }

    int sample_count = recover_available_sample_count();

    if (sample_count <= 0) {
        return;
    }

    if (max_duration > 0.f) {
        sample_count = std::min(sample_count, static_cast<int>(max_duration * static_cast<float>(frequency)));
    }

    data.buffer.resize(recover_frame_size(format) * sample_count);

    alcCaptureSamples(static_cast<ALCdevice*>(device), data.buffer.data(), sample_count);
    check_error(device, "Failed to recover captured data");
}

void Microphone::destroy()
{
    ZoneScopedN("Microphone::destroy");

    if (device == nullptr) {
        return;
    }

    Log::debug("[Microphone] Destroying...");

    if (!alcCaptureCloseDevice(static_cast<ALCdevice*>(device))) {
        Log::error("[OpenAL] Failed to close the audio capture device.");
    }

    device = nullptr;

    Log::debug("[Microphone] Destroyed");
}

}
