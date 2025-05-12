#pragma once

namespace xen::PerlinNoise {
/// Computes the 1D Perlin noise at the given coordinate.
/// \param x X coordinate.
/// \param octave_count Amount of octaves to apply for the [fractional Brownian
/// motion](https://en.wikipedia.org/wiki/Fractional_Brownian_motion) computation.
/// \param normalize Remap the value between [0; 1]. If false, the original [-1; 1] range is preserved.
/// \return 1D Perlin noise value. May be slightly below or above the expected range.
float compute_1d(float x, uint8_t octave_count = 1, bool normalize = false);

/// Computes the 2D Perlin noise at the given coordinates.
/// \param x X coordinate.
/// \param y Y coordinate.
/// \param octave_count Amount of octaves to apply for the [fractional Brownian
/// motion](https://en.wikipedia.org/wiki/Fractional_Brownian_motion) computation.
/// \param normalize Remap the value between [0; 1]. If false, the original [-1; 1] range is preserved.
/// \return 2D Perlin noise value. May be slightly below or above the expected range.
float compute_2d(Vector2f const& pos, uint8_t octave_count = 1, bool normalize = false);

/// Computes the 3D Perlin noise at the given coordinates.
/// \param x X coordinate.
/// \param y Y coordinate.
/// \param z Z coordinate.
/// \param octave_count Amount of octaves to apply for the [fractional Brownian
/// motion](https://en.wikipedia.org/wiki/Fractional_Brownian_motion) computation.
/// \param normalize Remap the value between [0; 1]. If false, the original [-1; 1] range is preserved.
/// \return 3D Perlin noise value. May be slightly below or above the expected range.
float compute_3d(Vector3f const& pos, uint8_t octave_count = 1, bool normalize = false);
}