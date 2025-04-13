#include "image_loader.hpp"

#include "debug/log.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace xen {
Image ImageLoader::load_image(std::string_view filepath, bool flip)
{
    // MAKE_SCOPE_PROFILER("ImageLoader::LoadImage");
    // MAKE_SCOPE_TIMER("MxEngine::ImageLoader", "ImageLoader::LoadImage()");
    Log::info("MxEngine::ImageLoader", " loading image from file: ", filepath);

    stbi_set_flip_vertically_on_load(flip);
    int width = 0;
    int height = 0;
    int channels = 0;
    uint8_t* data = stbi_load(filepath.data(), &width, &height, &channels, STBI_rgb_alpha);
    if (data == nullptr) {
        width = height = 0;
    }
    channels = 4;
    return {data, static_cast<size_t>(width), static_cast<size_t>(height), static_cast<size_t>(channels)};
}

Image ImageLoader::load_image_from_memory(uint8_t const* memory, size_t byte_size, bool flip)
{
    // MAKE_SCOPE_PROFILER("ImageLoader::LoadImage");
    // MAKE_SCOPE_TIMER("MxEngine::ImageLoader", "ImageLoader::LoadImage()");
    Log::info("MxEngine::ImageLoader", " loading image from memory");

    stbi_set_flip_vertically_on_load(flip);
    int width = 0;
    int height = 0;
    int channels = 0;
    uint8_t* data =
        stbi_load_from_memory(memory, static_cast<int>(byte_size), &width, &height, &channels, STBI_rgb_alpha);
    if (data == nullptr) {
        width = height = 0;
    }
    channels = 4;
    return {data, static_cast<size_t>(width), static_cast<size_t>(height), static_cast<size_t>(channels)};
}

/*
    0X00
    XXXX -> format of input
    0X00
*/

/*
    result[0] = right
    result[1] = left
    result[2] = top
    result[3] = bottom
    result[4] = front
    result[5] = back
*/
ImageLoader::ImageArray ImageLoader::create_cubemap(Image const& image)
{
    ImageArray result;
    size_t channels = 4;
    size_t width = image.get_width() / 4; //-V112
    size_t height = image.get_height() / 3;
    if (width != height) {
        Log::warning("MxEngine::ImageLoader", " image size is invalid, it will be reduced to fit skybox cubemap");
        width =
            height = static_cast<size_t>(1)
                     << static_cast<size_t>(std::log2(std::min(image.get_width() / 4, image.get_height() / 3))); //-V112
    }
    for (auto& arr : result) {
        arr.resize(height);

        for (auto& arr2 : arr) {
            arr.resize(width * channels);
        }
    }

    auto copy_side = [&image, &width, &height,
                      &channels](std::vector<std::vector<unsigned char>>& dst, size_t slice_x, size_t slice_y) {
        for (size_t i = 0; i < height; i++) {
            size_t y = i + slice_y * height;
            size_t x = slice_x * width;
            size_t bytesInRow = width * channels;

            memcpy(dst[i].data(), &image.get_raw_data()[(y * image.get_width() + x) * channels], bytesInRow);
        }
    };

    copy_side(result[0], 2, 1);
    copy_side(result[1], 0, 1);
    copy_side(result[2], 1, 0);
    copy_side(result[3], 1, 2);
    copy_side(result[4], 1, 1);
    copy_side(result[5], 3, 1);
    return result;
}
}