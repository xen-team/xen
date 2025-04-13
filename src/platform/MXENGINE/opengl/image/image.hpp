#pragma once

#include "math/vector2.hpp"
#include "utils/color.hpp"

#include <cstddef>

namespace xen {
using Pixel = std::array<uint8_t, 4>;

class Image {
private:
    uint8_t* data;
    size_t width;
    size_t height;
    uint8_t channels;

public:
    constexpr Image();
    constexpr Image(uint8_t* data, size_t width, size_t height, size_t channels);
    constexpr ~Image();
    constexpr Image(Image const&) = delete;
    constexpr Image& operator=(Image const&) = delete;
    constexpr Image(Image&&) noexcept;
    constexpr Image& operator=(Image&&) noexcept;

    [[nodiscard]] constexpr uint8_t* get_raw_data() const { return data; }
    [[nodiscard]] constexpr size_t get_width() const { return width; }
    [[nodiscard]] constexpr size_t get_height() const { return height; }
    [[nodiscard]] constexpr size_t get_channel_count() const { return channels; }
    [[nodiscard]] static constexpr size_t get_channel_size() { return sizeof(uint8_t); }
    [[nodiscard]] constexpr size_t get_pixel_size() const { return get_channel_count() * get_channel_size(); }
    [[nodiscard]] constexpr size_t get_total_byte_size() const { return get_width() * get_height() * get_pixel_size(); }

    constexpr void set_pixel(size_t x, size_t y, Pixel const& pixel);
    [[nodiscard]] constexpr Pixel get_pixel(size_t x, size_t y) const;

private:
    constexpr void free();
};
}