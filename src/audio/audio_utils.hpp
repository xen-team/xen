#pragma once

namespace xen {
struct AudioData;

namespace AudioUtils {

/// Transforms audio data to be mono by averaging channels. Does nothing if the input is already mono.
/// \param audio_data Data to be converted.
void convert_to_mono(AudioData& audio_data);

}
}