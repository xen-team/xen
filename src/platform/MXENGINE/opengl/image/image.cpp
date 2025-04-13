#include "image.hpp"
#include <memory>
#include <cstdlib>

namespace xen {
enum PixelColor : uint8_t { RED = 0, GREEN, BLUE, ALPHA };

constexpr void Image::free()
{
    if (this->data != nullptr) {
        ::free(this->data);
    }
}

constexpr Image::Image() : Image(nullptr, 0, 0, 0) {}

constexpr Image::Image(uint8_t* data, size_t width, size_t height, size_t channels) :
    data(data), width(width), height(height), channels(static_cast<uint8_t>(channels))
{
}

constexpr Image::~Image()
{
    free();
}

constexpr Image::Image(Image&& other) noexcept :
    data(other.data), width(other.width), height(other.height), channels(other.channels)
{
}

constexpr Image& Image::operator=(Image&& other) noexcept
{
    this->free();

    data = other.data;
    width = other.width;
    height = other.height;
    channels = other.channels;
    return *this;
}
constexpr void Image::set_pixel(size_t x, size_t y, Pixel const& pixel)
{
    assert(x < width && y < height);
    switch (channels) {
    case 1:
        data[(x * height) + y] = pixel[RED];
        break;
    case 2:
        data[((x * height + y) * 2) + 0] = pixel[RED];
        data[((x * height + y) * 2) + 1] = pixel[GREEN];
        break;
    case 3:
        data[((x * height + y) * 3) + 0] = pixel[RED];
        data[((x * height + y) * 3) + 1] = pixel[GREEN];
        data[((x * height + y) * 3) + 2] = pixel[BLUE];
        break;
    case 4:
        data[((x * height + y) * 4) + 0] = pixel[RED];
        data[((x * height + y) * 4) + 1] = pixel[GREEN];
        data[((x * height + y) * 4) + 2] = pixel[BLUE];
        data[((x * height + y) * 4) + 3] = pixel[ALPHA];
        break;
    default:
        assert(false); // invalid channel count
        break;
    }
}

constexpr Pixel Image::get_pixel(size_t x, size_t y) const
{
    Pixel rgba{0, 0, 0, 255};

    assert(x < width && y < height);
    switch (channels) {
    case 1:
        rgba[0] = data[(x * height) + y];
        break;
    case 2:
        rgba[0] = data[((x * height + y) * 2) + 0];
        rgba[1] = data[((x * height + y) * 2) + 1];
        break;
    case 3:
        rgba[0] = data[((x * height + y) * 3) + 0];
        rgba[1] = data[((x * height + y) * 3) + 1];
        rgba[2] = data[((x * height + y) * 3) + 2];
        break;
    case 4:
        rgba[0] = data[((x * height + y) * 4) + 0];
        rgba[1] = data[((x * height + y) * 4) + 1];
        rgba[2] = data[((x * height + y) * 4) + 2];
        rgba[3] = data[((x * height + y) * 4) + 3];
        break;
    default:
        assert(false); // invalid channel count
        break;
    }
    return rgba;
}
}