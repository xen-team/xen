#include <data/wav_format.hpp>
#include <audio/audio_data.hpp>
#include <utils/filepath.hpp>

#include <tracy/Tracy.hpp>

namespace xen::WavFormat {
namespace {
constexpr uint32_t from_little_endian(std::array<uint8_t, 4> const& bytes)
{
    return static_cast<uint32_t>((bytes[0] << 0u) | (bytes[1] << 8u) | (bytes[2] << 16u) | (bytes[3] << 24u));
}

constexpr uint16_t from_little_endian(uint8_t byte1, uint8_t byte2)
{
    return static_cast<uint16_t>((byte1 << 0u) | (byte2 << 8u));
}

struct WavInfo {
    uint32_t file_size{};
    uint32_t format_block_size{};
    uint16_t audio_format{};
    uint16_t channel_count{};
    uint32_t frequency{};
    uint32_t bytes_per_second{};
    uint16_t bytes_per_block{};
    uint16_t bits_per_sample{};
    uint32_t data_size{};
};

void load_fmt(std::ifstream& file, WavInfo& info)
{
    std::array<uint8_t, 4> bytes{};

    file.read(reinterpret_cast<char*>(bytes.data()), 4); // Format block size - 16
    info.format_block_size = from_little_endian(bytes);

    file.read(reinterpret_cast<char*>(bytes.data()), 2); // Audio format
    info.audio_format = from_little_endian(bytes[0], bytes[1]);
    // 0: Unknown
    // 1: PCM (uncompressed)
    // 2: Microsoft ADPCM
    // 3: IEEE float
    // 6: 8-bit ITU-T G.711 A-law
    // 7: 8-bit ITU-T G.711 µ-law
    // 17: IMA ADPCM
    // 20: ITU G.723 ADPCM (Yamaha)
    // 49: GSM 6.10
    // 64: ITU G.721 ADPCM
    // 80: MPEG

    if (info.audio_format != 1) {
        Log::warning("[WavLoad] Only WAV files with a PCM format are supported.");
    }

    file.read(reinterpret_cast<char*>(bytes.data()), 2); // Channel count
    info.channel_count = from_little_endian(bytes[0], bytes[1]);
    // 1 channel:  mono
    // 2 channels: stereo
    // 3 channels: left, center & right
    // 4 channels: front left, front right, back left & back right
    // 5 channels: left, center, right & surround
    // 6 channels: left, center left, center, center right, right & surround

    file.read(reinterpret_cast<char*>(bytes.data()), 4); // Sampling frequency
    info.frequency = from_little_endian(bytes);

    file.read(reinterpret_cast<char*>(bytes.data()), 4); // Bytes per second (frequency * bytes per block)
    info.bytes_per_second = from_little_endian(bytes);

    file.read(reinterpret_cast<char*>(bytes.data()), 2); // Bytes per block (bits per sample / 8 * channel count)
    info.bytes_per_block = from_little_endian(bytes[0], bytes[1]);

    file.read(reinterpret_cast<char*>(bytes.data()), 2); // Bits per sample (bit depth)
    info.bits_per_sample = from_little_endian(bytes[0], bytes[1]);
}

std::optional<WavInfo> validate_wav(std::ifstream& file)
{
    WavInfo info{};

    std::array<uint8_t, 4> bytes{};

    file.read(reinterpret_cast<char*>(bytes.data()), 4); // 'RIFF'
    if (bytes[0] != 'R' && bytes[1] != 'I' && bytes[2] != 'F' && bytes[3] != 'F') {
        return std::nullopt;
    }

    file.read(reinterpret_cast<char*>(bytes.data()), 4); // File size - 8
    info.file_size = from_little_endian(bytes);          // Values are in little-endian; they must be converted

    file.read(reinterpret_cast<char*>(bytes.data()), 4); // 'WAVE'
    if (bytes[0] != 'W' && bytes[1] != 'A' && bytes[2] != 'V' && bytes[3] != 'E') {
        return std::nullopt;
    }

    // Additional chunks can be present (such as 'JUNK', 'cue ', 'LIST', 'bext' and others), which aren't supported
    // there. They must be skipped See:
    // - https://en.wikipedia.org/wiki/WAV#File_specifications
    // - https://en.wikipedia.org/wiki/Broadcast_Wave_Format#Details
    // - https://stackoverflow.com/a/76137824/3292304
    while (file) {
        file.read(reinterpret_cast<char*>(bytes.data()), 4);

        if (bytes[0] == 'f' && bytes[1] == 'm' && bytes[2] == 't' && bytes[3] == ' ') {
            load_fmt(file, info);
            continue;
        }

        if (bytes[0] == 'd' && bytes[1] == 'a' && bytes[2] == 't' && bytes[3] == 'a') {
            file.read(
                reinterpret_cast<char*>(bytes.data()), 4
            ); // Data size (file size - header size (theoretically 44 bytes))
            info.data_size = from_little_endian(bytes);

            return info;
        }

        // Unsupported chunk, skip it
        file.read(reinterpret_cast<char*>(bytes.data()), 4); // Chunk size
        file.ignore(from_little_endian(bytes));
    }

    throw std::invalid_argument("[WavLoad] No data block found");
}
}
AudioData load(FilePath const& filepath)
{
    ZoneScopedN("WavFormat::load");
    ZoneTextF("Path: %s", filepath.to_utf8().c_str());

    Log::debug("[WavLoad] Loading WAV file ('" + filepath + "')...");

    std::ifstream file(filepath, std::ios_base::binary);

    if (!file) {
        throw std::invalid_argument("[WavLoad] Could not open the WAV file '" + filepath + "'");
    }

    std::optional<WavInfo> const info = validate_wav(file);

    if (info == std::nullopt) {
        throw std::runtime_error("[WavLoad] '" + filepath + "' is not a valid WAV audio file");
    }

    AudioData audio_data{};

    // Determining the right audio format
    switch (info->bits_per_sample) {
    case 8:
        if (info->channel_count == 1) {
            audio_data.format = AudioFormat::MONO_U8;
        }
        else if (info->channel_count == 2) {
            audio_data.format = AudioFormat::STEREO_U8;
        }
        break;

    case 16:
        if (info->channel_count == 1) {
            audio_data.format = AudioFormat::MONO_I16;
        }
        else if (info->channel_count == 2) {
            audio_data.format = AudioFormat::STEREO_I16;
        }
        break;

    case 32:
        if (info->channel_count == 1) {
            audio_data.format = AudioFormat::MONO_F32;
        }
        else if (info->channel_count == 2) {
            audio_data.format = AudioFormat::STEREO_F32;
        }
        break;

    case 64:
        if (info->channel_count == 1) {
            audio_data.format = AudioFormat::MONO_F64;
        }
        else if (info->channel_count == 2) {
            audio_data.format = AudioFormat::STEREO_F64;
        }
        break;

    default:
        throw std::runtime_error(
            "[WavLoad] " + std::to_string(info->bits_per_sample) + " bits WAV files are unsupported"
        );
    }

    // If the format is still unassigned, it is invalid
    if (static_cast<int>(audio_data.format) == 0) {
        throw std::runtime_error("[WavLoad] Unsupported WAV channel count");
    }

    audio_data.frequency = static_cast<int>(info->frequency);

    // Reading the actual audio data from the file
    audio_data.buffer.resize(info->data_size);
    file.read(
        reinterpret_cast<char*>(audio_data.buffer.data()), static_cast<std::streamsize>(audio_data.buffer.size())
    );

    Log::debug("[WavLoad] Loaded WAV file");

    return audio_data;
}
}