#pragma once

namespace xen {
struct AudioData;
class FilePath;

namespace WavFormat {
/// Loads audio data from a [WAV](https://en.wikipedia.org/wiki/WAV) file.
/// \param filepath File from which to load the audio.
/// \return Imported audio data.
AudioData load(FilePath const& filepath);

/// Saves audio data to a [WAV](https://en.wikipedia.org/wiki/WAV) file.
/// \param filepath File to which to save the sound.
/// \param data Audio data to export.
void save(FilePath const& filepath, AudioData const& data);
}
}