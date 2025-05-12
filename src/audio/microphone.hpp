#pragma once

#include <audio/audio_data.hpp>

namespace xen {
class Microphone {
public:
    /// Creates a microphone object handling audio capture.
    /// \param format Format in which to record.
    /// \param frequency Sampling frequency in which to record, in Hertz.
    /// \param duration Amount of time to record, in seconds. This is a minimum; the actual available duration may
    /// exceed this value.
    /// \param device_name Name of the audio capture device to open; if empty, will use the default one.
    /// \see Microphone::recover_devices()
    Microphone(AudioFormat format, uint frequency, float duration, std::string const& device_name = {});
    Microphone(Microphone const&) = delete;
    Microphone(Microphone&&) = delete;

    Microphone& operator=(Microphone const&) = delete;
    Microphone& operator=(Microphone&&) = delete;

    ~Microphone() { destroy(); }

    /// Recovers the names of all available audio capture devices.
    /// \return List of the available devices' names, or an empty list if the required extension is unsupported.
    static std::vector<std::string> recover_devices();

    /// Opens the audio capture device corresponding to the given name.
    /// \param format Format in which to record.
    /// \param frequency Sampling frequency in which to record, in Hertz.
    /// \param duration Amount of time to record, in seconds. This is a minimum; the actual available duration may
    /// exceed this value.
    /// \param device_name Name of the audio capture device to open; if empty, will use the default one.
    /// \see Microphone::recover_devices()
    void open_device(AudioFormat format, uint frequency, float duration, std::string const& device_name = {});

    /// Recovers the name of the current audio capture device.
    /// \return The current device's name, or an empty string if the required extension is unsupported.
    std::string recover_current_device() const;

    /// Starts capturing audio.
    void start() const;

    /// Stops capturing audio.
    void stop() const;

    /// Recovers the amount of currently captured samples.
    /// \return Available captured sample count.
    int recover_available_sample_count() const;

    /// Recovers the amount of currently captured time.
    /// \return Available captured duration, in seconds.
    float recover_available_duration() const;

    /// Recovers captured audio data.
    /// \note This flushes the captured data; if recovering something, the available sample count right after this call
    /// will be less than it was before.
    /// \param max_duration Maximum amount of time to recover, in seconds. Giving a negative value will result in
    /// recovering all available samples.
    /// \return Captured audio data. The format & frequency are those of the current audio input device.
    AudioData recover_data(float max_duration = -1.f) const;

    /// Recovers captured audio data. This overload can be used to avoid reallocating the whole memory range on each
    /// call.
    /// \note This flushes the captured data; if recovering something, the available sample count right after this call
    /// will be less than it was before.
    /// \param data Data to be filled with the captured audio.
    /// \param max_duration Maximum amount of time to recover, in seconds. Giving a negative value will result in
    /// recovering all available samples.
    void recover_data(AudioData& data, float max_duration = -1.f) const;

private:
    void* device{};

    AudioFormat format{};
    uint frequency{};

private:
    /// Destroys the microphone.
    void destroy();
};
}