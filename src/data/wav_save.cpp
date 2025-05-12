#include <data/wav_format.hpp>
#include <audio/sound.hpp>
#include <utils/filepath.hpp>

#include <tracy/Tracy.hpp>

namespace {
constexpr std::array<char, 4> to_little_endian32(uint32_t val)
{
    return {
        static_cast<char>(val & 0xFFu), static_cast<char>((val >> 8u) & 0xFFu), static_cast<char>((val >> 16u) & 0xFFu),
        static_cast<char>(val >> 24u)
    };
}

constexpr std::array<char, 2> to_little_endian16(uint16_t val)
{
    return {static_cast<char>(val & 0xFFu), static_cast<char>(val >> 8u)};
}

}

namespace xen::WavFormat {
void save(FilePath const& filepath, AudioData const& data)
{
    ZoneScopedN("WavFormat::save");
    ZoneTextF("Path: %s", filepath.to_utf8().c_str());

    Log::debug("[WavSave] Saving WAV file ('" + filepath + "')...");

    std::ofstream file(filepath, std::ios_base::binary);

    if (!file) {
        throw std::invalid_argument(
            "[WavSave] Unable to create a WAV file as '" + filepath + "'; path to file must exist"
        );
    }

    ////////////
    // Header //
    ////////////

    file << "RIFF";
    file.write(to_little_endian32(static_cast<uint32_t>(data.buffer.size()) + 36).data(), 4); // File size - 8
    file << "WAVE";

    //////////////////
    // Audio format //
    //////////////////

    file << "fmt ";
    file.write(to_little_endian32(16).data(), 4); // Format section size

    uint8_t bit_count{};

    switch (data.format) {
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
        throw std::invalid_argument("[WavSave] Unhandled audio format");
    }

    file.write(to_little_endian16((bit_count >= 32 ? 3 : 1)).data(), 2); // Writing 1 if integer, 3 if floating-point

    uint16_t channel_count{};

    switch (data.format) {
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
        throw std::invalid_argument("[WavSave] Unhandled audio format");
    }

    file.write(to_little_endian16(channel_count).data(), 2);
    file.write(to_little_endian32(static_cast<uint32_t>(data.frequency)).data(), 4);

    auto const frame_size = static_cast<uint16_t>(bit_count / 8 * channel_count);

    file.write(to_little_endian32(static_cast<uint32_t>(data.frequency) * frame_size).data(), 4); // Bytes per second
    file.write(to_little_endian16(frame_size).data(), 2); // Bytes per block (bits per sample / 8 * channel count)
    file.write(to_little_endian16(bit_count).data(), 2);  // Bits per sample (bit depth)

    ////////////////
    // Data block //
    ////////////////

    file << "data";
    file.write(to_little_endian32(static_cast<uint32_t>(data.buffer.size())).data(), 4);
    file.write(reinterpret_cast<char const*>(data.buffer.data()), static_cast<std::streamsize>(data.buffer.size()));

    Log::debug("[WavSave] Saved WAV file");
}
}