#include "audio_utils.hpp"

#include <audio/audio_data.hpp>

#include <tracy/Tracy.hpp>

namespace xen {
namespace {
constexpr int16_t from_little_endian(uint8_t byte1, uint8_t byte2)
{
    return static_cast<int16_t>((byte1 << 0u) | (byte2 << 8u));
}

constexpr std::array<uint8_t, 2> to_little_endian16(int16_t val)
{
    return {static_cast<uint8_t>(val & 0xFFu), static_cast<uint8_t>(val >> 8u)};
}

constexpr std::array<uint8_t, 2>
computeMonoI16(std::array<uint8_t, 2> leftSampleBytes, std::array<uint8_t, 2> rightSampleBytes)
{
    int16_t const leftSample = from_little_endian(leftSampleBytes[0], leftSampleBytes[1]);
    int16_t const rightSample = from_little_endian(rightSampleBytes[0], rightSampleBytes[1]);
    auto const mergedSample = static_cast<int32_t>(leftSample) + static_cast<int32_t>(rightSample);

    return to_little_endian16(static_cast<int16_t>(mergedSample / 2));
}
}
void AudioUtils::convert_to_mono(AudioData& audio_data)
{
    ZoneScopedN("AudioUtils::convert_to_mono");

    AudioFormat const orig_format = audio_data.format;

    if (orig_format == AudioFormat::MONO_U8 || orig_format == AudioFormat::MONO_I16 ||
        orig_format == AudioFormat::MONO_F32 || orig_format == AudioFormat::MONO_F64) {
        return;
    }

    if (orig_format == AudioFormat::STEREO_F32 || orig_format == AudioFormat::STEREO_F64) {
        throw std::invalid_argument("[AudioUtils] Mono conversion currently unsupported for floating-point formats");
    }

    uint8_t bytes_per_sample{};

    switch (orig_format) {
    case AudioFormat::STEREO_U8:
        bytes_per_sample = 1;
        audio_data.format = AudioFormat::MONO_U8;
        break;

    case AudioFormat::STEREO_I16:
        bytes_per_sample = 2;
        audio_data.format = AudioFormat::MONO_I16;
        break;

        // case AudioFormat::STEREO_F32:
        //   bytes_per_sample   = 4;
        //   audio_data.format = AudioFormat::MONO_F32;
        //   break;

        // case AudioFormat::STEREO_F64:
        //   bytes_per_sample   = 8;
        //   audio_data.format = AudioFormat::MONO_F64;
        //   break;

    default:
        throw std::invalid_argument("[AudioData] Unexpected format to convert to mono");
    }

    std::vector<uint8_t> mono_buffer(audio_data.buffer.size() / 2);

    for (size_t value_index = 0; value_index < audio_data.buffer.size(); value_index += bytes_per_sample * 2) {
        size_t const mono_index = value_index / 2;

        if (bytes_per_sample == 1) {
            mono_buffer[mono_index] =
                static_cast<uint8_t>((audio_data.buffer[value_index] + audio_data.buffer[value_index + 1]) / 2);
        }
        else if (bytes_per_sample == 2) {
            std::array<uint8_t, 2> const mono_sample = computeMonoI16(
                {audio_data.buffer[value_index], audio_data.buffer[value_index + 1]},
                {audio_data.buffer[value_index + 2], audio_data.buffer[value_index + 3]}
            );
            mono_buffer[mono_index] = mono_sample[0];
            mono_buffer[mono_index + 1] = mono_sample[1];
        }
    }

    audio_data.buffer = std::move(mono_buffer);
}
}