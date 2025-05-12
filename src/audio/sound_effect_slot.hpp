#pragma once

#include <data/owner_value.hpp>

namespace xen {
class SoundEffect;

class SoundEffectSlot {
public:
    SoundEffectSlot() { init(); }
    SoundEffectSlot(SoundEffectSlot const&) = delete;
    SoundEffectSlot(SoundEffectSlot&&) noexcept = default;

    SoundEffectSlot& operator=(SoundEffectSlot const&) = delete;
    SoundEffectSlot& operator=(SoundEffectSlot&&) noexcept = default;

    ~SoundEffectSlot() { destroy(); }

    [[nodiscard]] uint get_index() const { return index; }

    /// Initializes the sound effect slot.
    /// \note A SoundEffectSlot must be initialized again after opening an audio device.
    /// \see AudioSystem::open()
    void init();

    /// Loads a sound effect into the current slot.
    /// \param effect Sound effect to be loaded.
    void load_effect(SoundEffect const& effect) const;

    /// Destroys the sound effect slot.
    void destroy();

private:
    OwnerValue<uint, std::numeric_limits<uint>::max()> index{};
};
}