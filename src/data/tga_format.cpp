#include "tga_format.hpp"

#include <data/image.hpp>
#include <utils/filepath.hpp>

#include <tracy/Tracy.hpp>

namespace xen::TgaFormat {
Image load(FilePath const& filepath, bool flip_vertically)
{
    ZoneScopedN("TgaFormat::load");
    ZoneTextF("Path: %s", filepath.to_utf8().c_str());

    Log::debug("[TgaFormat] Loading TGA file ('" + filepath + "')...");

    std::ifstream file(filepath, std::ios_base::binary);

    if (!file) {
        throw std::invalid_argument("Error: Could not open the PNG file '" + filepath + "'");
    }

    // Declaring a single array of unsigned char, reused everywhere later
    std::array<uint8_t, 2> bytes{};

    // ID length (identifies the number of bytes contained in field 6, the image ID Field. The maximum number
    //  of characters is 255. A value of zero indicates that no image ID field is included within the image) - TODO
    file.read(reinterpret_cast<char*>(bytes.data()), 1);

    // Colormap (0 - no colormap, 1 - colormap)
    file.read(reinterpret_cast<char*>(bytes.data()), 1);
    bool const has_colormap = (bytes[0] == 1);

    // Image type
    file.read(reinterpret_cast<char*>(bytes.data()), 1);

    bool run_length_encoding = false;
    uint8_t channel_count{};
    ImageColorspace colorspace{};

    switch (bytes[0]) {
    case 0: // No image data available
        throw std::runtime_error("Error: Invalid TGA image, no data available");

    case 9:  // RLE color-mapped
    case 10: // RLE true-color
        run_length_encoding = true;
        [[fallthrough]];
    case 1: // Uncompressed color-mapped
    case 2: // Uncompressed true-color
        channel_count = 3;
        colorspace = ImageColorspace::RGB;
        break;

    case 11: // RLE gray
        run_length_encoding = true;
        [[fallthrough]];
    case 3: // Uncompressed gray
        channel_count = 1;
        colorspace = ImageColorspace::GRAY;
        break;

    default:
        throw std::runtime_error("Error: TGA invalid image type");
    }

    // TODO: handle colormap
    if (has_colormap) {
        // Colormap specs (size 5)
        std::array<char, 5> color_specs{};
        file.read(color_specs.data(), 5);

        // First entry index (2 bytes)

        // Colormap length (2 bytes)

        // Colormap entry size (1 byte)
    }
    else {
        file.ignore(5);
    }

    // Image specs (10 bytes)

    // X- & Y-origin (2 bytes each) - TODO: handle origins
    // It is expected to have 0 for both origins
    uint16_t x_origin{};
    file.read(reinterpret_cast<char*>(&x_origin), 2);

    uint16_t y_origin{};
    file.read(reinterpret_cast<char*>(&y_origin), 2);

    // Width & height (2 bytes each)
    uint16_t width{};
    file.read(reinterpret_cast<char*>(&width), 2);

    uint16_t height{};
    file.read(reinterpret_cast<char*>(&height), 2);

    // Bit depth (1 byte)
    file.read(reinterpret_cast<char*>(bytes.data()), 1);
    [[maybe_unused]] uint8_t const bitDepth = bytes[0] / channel_count;

    // Image descriptor (1 byte) - TODO: handle image descriptor
    // Bits 3-0 give the alpha channel depth, bits 5-4 give direction
    file.read(reinterpret_cast<char*>(bytes.data()), 1);

    Image image(Vector2ui(width, height), colorspace, ImageDataType::BYTE);
    auto* image_data = static_cast<uint8_t*>(image.data());

    if (!run_length_encoding) {
        std::vector<uint8_t> values(width * height * channel_count);
        file.read(reinterpret_cast<char*>(values.data()), static_cast<std::streamsize>(values.size()));

        if (channel_count == 3) { // 3 channels, RGB
            for (size_t height_index = 0; height_index < height; ++height_index) {
                size_t const finalHeightIndex = (flip_vertically ? height_index : height - 1 - height_index);

                for (size_t width_index = 0; width_index < width; ++width_index) {
                    size_t const in_pixel_index = (height_index * width + width_index) * channel_count;
                    size_t const out_pixel_index = (finalHeightIndex * width + width_index) * channel_count;

                    // Values are laid out as BGR, they need to be reordered to RGB
                    image_data[out_pixel_index + 2] = values[in_pixel_index];
                    image_data[out_pixel_index + 1] = values[in_pixel_index + 1];
                    image_data[out_pixel_index] = values[in_pixel_index + 2];
                }
            }
        }
        else { // 1 channel, grayscale
            std::move(values.begin(), values.end(), image_data);
        }
    }
    else {
        throw std::runtime_error("Error: RLE on TGA images is not handled yet");
    }

    Log::debug("[TgaFormat] Loaded TGA file");

    return image;
}
}