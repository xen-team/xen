#pragma once

#include "image.hpp"

#include <array>
#include <filesystem>

namespace xen {
class ImageLoader {
public:
    static Image load_image(std::string_view filepath, bool flip = true);

    static Image load_image_from_memory(uint8_t const* memory, size_t byte_size, bool flip = true);

    using ImageArray = std::array<std::vector<std::vector<unsigned char>>, 6>;

    /*!
    creates cubemap projections from its scan:
     X
    XXXX
     X
    \param image image from which cubemap will be created
    \returns 6 2d arrays of raw image data (can be passed as individual images to OpenGL)
    */
    static ImageArray create_cubemap(Image const& image);
};
}