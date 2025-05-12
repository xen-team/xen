#include <audio/sound_effect_slot.hpp>

#include <audio/sound_effect.hpp>
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

    if (error_code != AL_NO_ERROR) {
        Log::verror("[OpenAL] {} ({}).", error_msg, recover_al_error_str(error_code));
    }
}

// Auxiliary effect slot functions
LPALGENAUXILIARYEFFECTSLOTS alGenAuxiliaryEffectSlots;
LPALDELETEAUXILIARYEFFECTSLOTS alDeleteAuxiliaryEffectSlots;
LPALISAUXILIARYEFFECTSLOT alIsAuxiliaryEffectSlot;
LPALAUXILIARYEFFECTSLOTI alAuxiliaryEffectSloti;
// LPALAUXILIARYEFFECTSLOTIV alAuxiliaryEffectSlotiv;
// LPALAUXILIARYEFFECTSLOTF alAuxiliaryEffectSlotf;
// LPALAUXILIARYEFFECTSLOTFV alAuxiliaryEffectSlotfv;
// LPALGETAUXILIARYEFFECTSLOTI alGetAuxiliaryEffectSloti;
// LPALGETAUXILIARYEFFECTSLOTIV alGetAuxiliaryEffectSlotiv;
// LPALGETAUXILIARYEFFECTSLOTF alGetAuxiliaryEffectSlotf;
// LPALGETAUXILIARYEFFECTSLOTFV alGetAuxiliaryEffectSlotfv;

inline bool load_functions()
{
    PUSH_WARNINGS_STATE
    DISABLE_WARNING_GCC(-Wconditionally - supported)

    alGenAuxiliaryEffectSlots =
        reinterpret_cast<LPALGENAUXILIARYEFFECTSLOTS>(alGetProcAddress("alGenAuxiliaryEffectSlots"));
    alDeleteAuxiliaryEffectSlots =
        reinterpret_cast<LPALDELETEAUXILIARYEFFECTSLOTS>(alGetProcAddress("alDeleteAuxiliaryEffectSlots"));
    alIsAuxiliaryEffectSlot = reinterpret_cast<LPALISAUXILIARYEFFECTSLOT>(alGetProcAddress("alIsAuxiliaryEffectSlot"));
    alAuxiliaryEffectSloti = reinterpret_cast<LPALAUXILIARYEFFECTSLOTI>(alGetProcAddress("alAuxiliaryEffectSloti"));
    // alAuxiliaryEffectSlotiv      =
    // reinterpret_cast<LPALAUXILIARYEFFECTSLOTIV>(alGetProcAddress("alAuxiliaryEffectSlotiv")); alAuxiliaryEffectSlotf
    // = reinterpret_cast<LPALAUXILIARYEFFECTSLOTF>(alGetProcAddress("alAuxiliaryEffectSlotf")); alAuxiliaryEffectSlotfv
    // = reinterpret_cast<LPALAUXILIARYEFFECTSLOTFV>(alGetProcAddress("alAuxiliaryEffectSlotfv"));
    // alGetAuxiliaryEffectSloti    =
    // reinterpret_cast<LPALGETAUXILIARYEFFECTSLOTI>(alGetProcAddress("alGetAuxiliaryEffectSloti"));
    // alGetAuxiliaryEffectSlotiv   =
    // reinterpret_cast<LPALGETAUXILIARYEFFECTSLOTIV>(alGetProcAddress("alGetAuxiliaryEffectSlotiv"));
    // alGetAuxiliaryEffectSlotf    =
    // reinterpret_cast<LPALGETAUXILIARYEFFECTSLOTF>(alGetProcAddress("alGetAuxiliaryEffectSlotf"));
    // alGetAuxiliaryEffectSlotfv   =
    // reinterpret_cast<LPALGETAUXILIARYEFFECTSLOTFV>(alGetProcAddress("alGetAuxiliaryEffectSlotfv"));

    POP_WARNINGS_STATE

    return true;
}
}
void SoundEffectSlot::init()
{
    ZoneScopedN("SoundEffectSlot::init");

    if (!alcIsExtensionPresent(alcGetContextsDevice(alcGetCurrentContext()), "ALC_EXT_EFX")) {
        Log::error("[SoundEffectSlot] Sound effects are unavailable.");
        return;
    }

    [[maybe_unused]] static bool const funcsLoaded = load_functions();

    Log::debug("[SoundEffectSlot] Initializing...");

    alGetError(); // Flushing errors

    destroy();

    alGenAuxiliaryEffectSlots(1, &index.get());
    check_error("Failed to create a sound effect slot");

    Log::debug("[SoundEffectSlot] Initialized (ID: " + std::to_string(index) + ")...");
}

void SoundEffectSlot::load_effect(SoundEffect const& effect) const
{
    ZoneScopedN("SoundEffectSlot::load_effect");

    alAuxiliaryEffectSloti(index, AL_EFFECTSLOT_EFFECT, static_cast<int>(effect.get_index()));
    check_error("Failed to load the sound effect");
}

void SoundEffectSlot::destroy()
{
    ZoneScopedN("SoundEffectSlot::destroy");

    if (!index.is_valid()) {
        return;
    }

    Log::debug("[SoundEffectSlot] Destroying (ID: " + std::to_string(index) + ")...");

    if (alIsAuxiliaryEffectSlot(index)) {
        alDeleteAuxiliaryEffectSlots(1, &index.get());
        check_error("Failed to delete sound effect slot");
    }

    index.reset();

    Log::debug("[SoundEffectSlot] Destroyed");
}

}
