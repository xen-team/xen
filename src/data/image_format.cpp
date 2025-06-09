#include "image_format.hpp"

#include <data/image.hpp>
#include <utils/filepath.hpp>
#include <utils/str_utils.hpp>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#define STBI_WINDOWS_UTF8
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBIW_WINDOWS_UTF8
#include "stb_image_write.h"

#include <tracy/Tracy.hpp>

namespace xen::ImageFormat {
namespace {
enum class FileFormat { UNKNOWN, BMP, PNG, JPG, TGA, HDR };

struct ImageDataDeleter {
    void operator()(void* data) { stbi_image_free(data); }
};

ImageColorspace recover_colorspace(int channel_count)
{
    switch (channel_count) {
    case 1:
        return ImageColorspace::GRAY;
    case 2:
        return ImageColorspace::GRAY_ALPHA;
    case 3:
        return ImageColorspace::RGB;
    case 4:
        return ImageColorspace::RGBA;
    default:
        throw std::invalid_argument("Error: Unsupported number of channels.");
    }
}

FileFormat recover_file_format(std::string const& file_extension)
{
    if (file_extension == "bmp") {
        return FileFormat::BMP;
    }
    if (file_extension == "hdr") {
        return FileFormat::HDR;
    }
    if (file_extension == "jpg" || file_extension == "jpeg") {
        return FileFormat::JPG;
    }
    if (file_extension == "png") {
        return FileFormat::PNG;
    }
    if (file_extension == "tga") {
        return FileFormat::TGA;
    }

    return FileFormat::UNKNOWN;
}

Image create_image_from_data(
    Vector2ui const& size, int channel_count, bool is_hdr, std::unique_ptr<void, ImageDataDeleter> const& data
)
{
    size_t const value_count = size.x * size.y * channel_count;

    Image image(size, recover_colorspace(channel_count), (is_hdr ? ImageDataType::FLOAT : ImageDataType::BYTE));

    if (is_hdr) {
        std::copy_n(static_cast<float*>(data.get()), value_count, static_cast<float*>(image.data()));
    }
    else {
        std::copy_n(static_cast<uint8_t*>(data.get()), value_count, static_cast<uint8_t*>(image.data()));
    }

    return image;
}
}

Image load(FilePath const& filepath, bool flip_vertically)
{
    ZoneScopedN("ImageFormat::load");
    ZoneTextF("Path: %s", filepath.to_utf8().c_str());

    Log::debug("[ImageFormat] Loading image '" + filepath + "'...");

    std::string const file_str = filepath.to_utf8();
    bool const is_hdr = (stbi_is_hdr(file_str.c_str()) != 0);

    stbi_set_flip_vertically_on_load(flip_vertically);

    int width{};
    int height{};
    int channel_count{};
    std::unique_ptr<void, ImageDataDeleter> data;

    if (is_hdr) {
        data.reset(stbi_loadf(file_str.c_str(), &width, &height, &channel_count, 0));
    }
    else {
        data.reset(stbi_load(file_str.c_str(), &width, &height, &channel_count, 0));
    }

    if (data == nullptr) {
        throw std::invalid_argument("[ImageFormat] Cannot load image '" + filepath + "': " + stbi_failure_reason());
    }

    Image image = create_image_from_data(Vector2ui(width, height), channel_count, is_hdr, data);

    Log::debug("[ImageFormat] Loaded image");

    return image;
}

Image load_from_data(std::vector<uint8_t> const& image_data, bool flip_vertically)
{
    return load_from_data(image_data.data(), image_data.size(), flip_vertically);
}

Image load_from_data(uint8_t const* image_data, size_t data_size, bool flip_vertically)
{
    ZoneScopedN("ImageFormat::load_from_data");

    Log::debug("[ImageFormat] Loading image from data...");

    stbi_set_flip_vertically_on_load(flip_vertically);

    bool const is_hdr = (stbi_is_hdr_from_memory(image_data, static_cast<int>(data_size)) != 0);

    int width{};
    int height{};
    int channel_count{};
    std::unique_ptr<void, ImageDataDeleter> data;

    if (is_hdr) {
        data.reset(stbi_loadf_from_memory(image_data, static_cast<int>(data_size), &width, &height, &channel_count, 0));
    }
    else {
        data.reset(stbi_load_from_memory(image_data, static_cast<int>(data_size), &width, &height, &channel_count, 0));
    }

    if (data == nullptr) {
        throw std::invalid_argument("[ImageFormat] Cannot load image from data: " + std::string(stbi_failure_reason()));
    }

    Image image = create_image_from_data(Vector2ui(width, height), static_cast<uint8_t>(channel_count), is_hdr, data);

    Log::debug("[ImageFormat] Loaded image from data");

    return image;
}

void save(FilePath const& filepath, Image const& image, bool flip_vertically)
{
    ZoneScopedN("ImageFormat::save");

    Log::debug("[ImageFormat] Saving image to '" + filepath + "'...");

    std::string const file_str = filepath.to_utf8();
    std::string const file_extension = StrUtils::to_lower_copy(filepath.recover_extension().to_utf8());

    stbi_flip_vertically_on_write(flip_vertically);

    auto const image_width = static_cast<int>(image.get_width());
    auto const image_height = static_cast<int>(image.get_height());
    auto const image_channel_count = static_cast<int>(image.get_channel_count());

    FileFormat const format = recover_file_format(file_extension);

    switch (format) {
    case FileFormat::BMP:
    case FileFormat::JPG:
    case FileFormat::PNG:
    case FileFormat::TGA:
        if (image.get_data_type() != ImageDataType::BYTE)
            throw std::invalid_argument("[ImageFormat] Saving a non-HDR image requires a byte data type.");
        break;

    case FileFormat::HDR:
        if (image.get_data_type() != ImageDataType::FLOAT)
            throw std::invalid_argument("[ImageFormat] Saving an HDR image requires a floating-point data type.");
        break;

    case FileFormat::UNKNOWN:
    default:
        break;
    }

    int result{};

    switch (format) {
    case FileFormat::BMP:
        result = stbi_write_bmp(file_str.c_str(), image_width, image_height, image_channel_count, image.data());
        break;

    case FileFormat::HDR:
        result = stbi_write_hdr(
            file_str.c_str(), image_width, image_height, image_channel_count, static_cast<float const*>(image.data())
        );
        break;

    case FileFormat::JPG:
        result = stbi_write_jpg(file_str.c_str(), image_width, image_height, image_channel_count, image.data(), 90);
        break;

    case FileFormat::PNG:
        result = stbi_write_png(
            file_str.c_str(), image_width, image_height, image_channel_count, image.data(),
            image_width * image_channel_count
        );
        break;

    case FileFormat::TGA:
        result = stbi_write_tga(file_str.c_str(), image_width, image_height, image_channel_count, image.data());
        break;

    case FileFormat::UNKNOWN:
    default:
        throw std::invalid_argument(
            "[ImageFormat] Unsupported image file extension '" + file_extension + "' for saving."
        );
    }

    if (result == 0) {
        throw std::invalid_argument("[ImageFormat] Failed to save image.");
    }

    Log::debug("[ImageFormat] Saved image");
}
}