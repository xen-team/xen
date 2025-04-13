#pragma once

#include "image.hpp"

#include <vector>

namespace xen {
class ImageConverter {
public:
    using RawImageData = std::vector<uint8_t>;

    static RawImageData convert_png(uint8_t const* data, int width, int height, int channels, bool flip = true);
    static RawImageData convert_bmp(uint8_t const* data, int width, int height, int channels, bool flip = true);
    static RawImageData convert_tga(uint8_t const* data, int width, int height, int channels, bool flip = true);
    static RawImageData
    convert_jpg(uint8_t const* data, int width, int height, int channels, int quality = 90, bool flip = true);
    static RawImageData convert_hdr(float const* data, int width, int height, int channels, bool flip = true);

    static RawImageData convert_png(Image const& image, bool flip = true);
    static RawImageData convert_bmp(Image const& image, bool flip = true);
    static RawImageData convert_tga(Image const& image, bool flip = true);
    static RawImageData convert_jpg(Image const& image, int quality = 90, bool flip = true);
    static RawImageData convert_hdr(Image const& image, bool flip = true);
};
}