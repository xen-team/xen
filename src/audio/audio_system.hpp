#pragma once

#include <system.hpp>

namespace xen {
class AudioSystem final : public System {
public:
    /// Creates a system handling audio.
    /// \param device_name Name of the audio device to open; if empty, will use the default one.
    /// \see AudioSystem::recover_devices()
    explicit AudioSystem(std::string const& device_name = {});

    ~AudioSystem() override { destroy(); }

    /// Recovers the names of all available audio devices.
    /// \return List of the available devices' names, or an empty list if the required extension is unsupported.
    static std::vector<std::string> recover_devices();

    /// Opens the audio device corresponding to the given name.
    /// \param device_name Name of the audio device to open; if empty, will use the default one.
    /// \see AudioSystem::recover_devices()
    void open_device(std::string const& device_name = {});

    /// Recovers the name of the current audio device.
    /// \return The current device's name, or an empty string if the required extension is unsupported.
    std::string recover_current_device() const;

    bool update(FrameTimeInfo const& time_info) override;

    void destroy() override;

private:
    void* device{};
    void* context{};
};
}