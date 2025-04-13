// #pragma once

// #include "image.hpp"

// #include <string>

// namespace xen {
// enum class ImageType {
//     PNG,
//     BMP,
//     TGA,
//     JPG,
//     HDR,
// };

// class ImageManager {
// public:
//     static void save_image(std::string_view filepath, Image const& image, ImageType type);

//     static void save_texture(std::string_view filepath, TextureHandle const& texture, ImageType type);

//     static void save_texture(std::string_view filepath, TextureHandle const& texture);

//     static void take_screenshot(std::string_view filepath, ImageType type);

//     static void take_screenshot(std::string_view filepath);

//     static void flip_image(Image& image);

//     // write order:
//     // 7 8 9
//     // 4 5 6
//     // 1 2 3
//     static Image combine_images(std::span<Image> images, size_t images_per_raw);
//     static Image combine_images(std::vector<std::vector<Image>>& images);
// };
// }