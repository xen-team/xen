#pragma once

#include "image.hpp"

namespace xen {
template <typename T>
T Image::recover_pixel(size_t width_index, size_t height_index) const
{
    Log::rt_assert(
        channel_count == 1, "Error: Recovering a pixel of a single value requires an image having a single channel."
    );

    if constexpr (std::is_same_v<T, uint8_t>) {
        return recover_byte_value(width_index, height_index, 0);
    }
    else if constexpr (std::is_same_v<T, float>) {
        return recover_float_value(width_index, height_index, 0);
    }
    else {
        static_assert(!std::is_same_v<T, T>, "Error: The given pixel's type to be recovered is unsupported.");
    }
}

// template <typename T, size_t N>
// Vector<T, N> Image::recover_pixel(size_t width_index, size_t height_index) const
// {
//     static_assert(
//         std::is_same_v<T, uint8_t> || std::is_same_v<T, float>,
//         "Error: The given pixel's type to be recovered is unsupported."
//     );
//     Log::rt_assert(
//         channel_count == N,
//         "Error: Recovering multiple values for a pixel requires an image having the same channel count."
//     );
//     Log::rt_assert(
//         !std::is_same_v<T, uint8_t> || data_type == ImageDataType::BYTE,
//         "Error: Recovering a byte pixel requires the image to be of a byte type."
//     );
//     Log::rt_assert(
//         !std::is_same_v<T, float> || data_type == ImageDataType::FLOAT,
//         "Error: Recovering a float pixel requires the image to be of a float type."
//     );

//     T const* image_data = static_cast<T*>(data->data()) + compute_index(width_index, height_index);

//     Vector<T> res;

//     for (size_t i = 0; i < N; ++i) {
//         res[i] = image_data[i];
//     }

//     return res;
// }

template <typename T>
void Image::set_pixel(size_t width_index, size_t height_index, T val)
{
    Log::rt_assert(
        channel_count == 1, "Error: Recovering a pixel of a single value requires an image having a single channel."
    );

    if constexpr (std::is_same_v<T, uint8_t>) {
        set_byte_value(width_index, height_index, 0, val);
    }
    else if constexpr (std::is_same_v<T, float>) {
        set_float_value(width_index, height_index, 0, val);
    }
    else {
        static_assert(!std::is_same_v<T, T>, "Error: The given pixel's type to be set is unsupported.");
    }
}
}