#include "image_converter.hpp"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#include <algorithm>

namespace xen {
void copy_image_data(void* context, void* data, int size)
{
    auto* storage = (ImageConverter::RawImageData*)(context);
    auto membegin = (uint8_t*)data;
    auto memend = membegin + size; //-V104
    storage->insert(storage->end(), membegin, memend);
}

ImageConverter::RawImageData
ImageConverter::convert_png(uint8_t const* imagedata, int width, int height, int channels, bool flip)
{
    ImageConverter::RawImageData data;

    // MAKE_SCOPE_PROFILER("ImageWriter::convert_png");
    // MAKE_SCOPE_TIMER("MxEngine::ImageWriter", "ImageWriter::convert_png()");

    stbi_flip_vertically_on_write(flip);
    stbi_write_png_to_func(
        copy_image_data, (void*)&data, width, height, channels, (void const*)imagedata, width * channels
    );
    return data;
}

ImageConverter::RawImageData
ImageConverter::convert_bmp(uint8_t const* imagedata, int width, int height, int channels, bool flip)
{
    ImageConverter::RawImageData data;

    // MAKE_SCOPE_PROFILER("ImageWriter::convert_bmp");
    // MAKE_SCOPE_TIMER("MxEngine::ImageWriter", "ImageWriter::convert_bmp()");

    stbi_flip_vertically_on_write(flip);
    stbi_write_bmp_to_func(copy_image_data, (void*)&data, width, height, channels, (void const*)imagedata);
    return data;
}

ImageConverter::RawImageData
ImageConverter::convert_tga(uint8_t const* imagedata, int width, int height, int channels, bool flip)
{
    ImageConverter::RawImageData data;

    // MAKE_SCOPE_PROFILER("ImageWriter::convert_tga");
    // MAKE_SCOPE_TIMER("MxEngine::ImageWriter", "ImageWriter::convert_tga()");

    stbi_flip_vertically_on_write(flip);
    stbi_write_tga_to_func(copy_image_data, (void*)&data, width, height, channels, (void const*)imagedata);
    return data;
}

ImageConverter::RawImageData
ImageConverter::convert_jpg(uint8_t const* imagedata, int width, int height, int channels, int quality, bool flip)
{
    ImageConverter::RawImageData data;

    // MAKE_SCOPE_PROFILER("ImageWriter::convert_jpg");
    // MAKE_SCOPE_TIMER("MxEngine::ImageWriter", "ImageWriter::convert_jpg()");

    stbi_flip_vertically_on_write(flip);
    stbi_write_jpg_to_func(copy_image_data, (void*)&data, width, height, channels, (void const*)imagedata, quality);
    return data;
}

ImageConverter::RawImageData
ImageConverter::convert_hdr(float const* imagedata, int width, int height, int channels, bool flip)
{
    ImageConverter::RawImageData data;

    // MAKE_SCOPE_PROFILER("ImageWriter::convert_hdr");
    // MAKE_SCOPE_TIMER("MxEngine::ImageWriter", "ImageWriter::convert_hdr()");

    stbi_flip_vertically_on_write(flip);
    stbi_write_hdr_to_func(copy_image_data, (void*)&data, width, height, channels, (float const*)imagedata);
    return data;
}

ImageConverter::RawImageData ImageConverter::convert_png(Image const& image, bool flip)
{
    return ImageConverter::convert_png(
        image.get_raw_data(), (int)image.get_width(), (int)image.get_height(), (int)image.get_channel_count(), flip
    );
}

ImageConverter::RawImageData ImageConverter::convert_bmp(Image const& image, bool flip)
{
    return ImageConverter::convert_bmp(
        image.get_raw_data(), (int)image.get_width(), (int)image.get_height(), (int)image.get_channel_count(), flip
    );
}

ImageConverter::RawImageData ImageConverter::convert_tga(Image const& image, bool flip)
{
    return ImageConverter::convert_tga(
        image.get_raw_data(), (int)image.get_width(), (int)image.get_height(), (int)image.get_channel_count(), flip
    );
}

ImageConverter::RawImageData ImageConverter::convert_jpg(Image const& image, int quality, bool flip)
{
    return ImageConverter::convert_jpg(
        image.get_raw_data(), (int)image.get_width(), (int)image.get_height(), (int)image.get_channel_count(), quality,
        flip
    );
}

ImageConverter::RawImageData ImageConverter::convert_hdr(Image const& image, bool flip)
{
    return ImageConverter::convert_hdr(
        (float*)image.get_raw_data(), (int)image.get_width(), (int)image.get_height(), (int)image.get_channel_count(),
        flip
    );
}
}