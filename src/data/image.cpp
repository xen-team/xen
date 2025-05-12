#include "image.hpp"

namespace xen {
bool ImageDataB::operator==(ImageData const& image_data) const
{
    Log::rt_assert(
        image_data.get_data_type() == ImageDataType::BYTE,
        "Error: Image data equality check requires having data of the same type."
    );

    auto const& image_data_b = static_cast<ImageDataB const&>(image_data);

    if (data_.size() != image_data_b.data_.size()) {
        return false;
    }

    return std::equal(data_.cbegin(), data_.cend(), image_data_b.data_.cbegin());
}

bool ImageDataF::operator==(ImageData const& image_data) const
{
    Log::rt_assert(
        image_data.get_data_type() == ImageDataType::FLOAT,
        "Error: Image data equality check requires having data of the same type."
    );

    auto const& image_data_f = static_cast<ImageDataF const&>(image_data);

    if (data_.size() != image_data_f.data_.size())
        return false;

    return std::equal(data_.cbegin(), data_.cend(), image_data_f.data_.cbegin());
}

Image::Image(ImageColorspace colorspace, ImageDataType data_type) : colorspace{colorspace}, data_type{data_type}
{
    Log::rt_assert(
        colorspace != ImageColorspace::SRGB || colorspace != ImageColorspace::SRGBA || data_type == ImageDataType::BYTE,
        "Error: An sRGB(A) image must have a byte data type."
    );

    switch (colorspace) {
    case ImageColorspace::GRAY:
        channel_count = 1;
        break;

    case ImageColorspace::GRAY_ALPHA:
        channel_count = 2;
        break;

    case ImageColorspace::RGB:
    case ImageColorspace::SRGB:
        channel_count = 3;
        break;

    case ImageColorspace::RGBA:
    case ImageColorspace::SRGBA:
        channel_count = 4;
        break;

    default:
        throw std::invalid_argument("Error: Invalid colorspace to create an image with");
    }
}

Image::Image(Vector2ui const& size, ImageColorspace colorspace, ImageDataType data_type) : Image(colorspace, data_type)
{
    this->size = size;

    size_t const image_data_size = this->size.x * this->size.y * channel_count;

    if (data_type == ImageDataType::FLOAT) {
        data_ = ImageDataF::create(image_data_size);
    }
    else {
        data_ = ImageDataB::create(image_data_size);
    }
}

Image::Image(Image const& image) :
    size{image.size}, colorspace{image.colorspace}, data_type{image.data_type}, channel_count{image.channel_count}
{
    if (image.data_ == nullptr) {
        return;
    }

    switch (image.data_type) {
    case ImageDataType::BYTE:
        data_ = ImageDataB::create(*static_cast<ImageDataB*>(image.data_.get()));
        break;

    case ImageDataType::FLOAT:
        data_ = ImageDataF::create(*static_cast<ImageDataF*>(image.data_.get()));
        break;
    }
}

uint8_t Image::recover_byte_value(size_t width_index, size_t height_index, uint8_t channel_index) const
{
    Log::rt_assert(
        data_type == ImageDataType::BYTE, "Error: Getting a byte value requires the image to be of a byte type."
    );
    return recover_value<uint8_t>(width_index, height_index, channel_index);
}

float Image::recover_float_value(size_t width_index, size_t height_index, uint8_t channel_index) const
{
    Log::rt_assert(
        data_type == ImageDataType::FLOAT, "Error: Getting a float value requires the image to be of a float type."
    );
    return recover_value<float>(width_index, height_index, channel_index);
}

void Image::set_byte_value(size_t width_index, size_t height_index, uint8_t channel_index, uint8_t val)
{
    Log::rt_assert(
        data_type == ImageDataType::BYTE, "Error: Setting a byte value requires the image to be of a byte type."
    );
    set_value(width_index, height_index, channel_index, val);
}

void Image::set_float_value(size_t width_index, size_t height_index, uint8_t channel_index, float val)
{
    Log::rt_assert(
        data_type == ImageDataType::FLOAT, "Error: Setting a float value requires the image to be of a float type."
    );
    set_value(width_index, height_index, channel_index, val);
}

Image& Image::operator=(Image const& image)
{
    size = image.size;
    colorspace = image.colorspace;
    data_type = image.data_type;
    channel_count = image.channel_count;

    if (image.data_) {
        switch (image.data_type) {
        case ImageDataType::BYTE:
            data_ = ImageDataB::create(*static_cast<ImageDataB*>(image.data_.get()));
            break;

        case ImageDataType::FLOAT:
            data_ = ImageDataF::create(*static_cast<ImageDataF*>(image.data_.get()));
            break;
        }
    }
    else {
        data_.reset();
    }

    return *this;
}

bool Image::operator==(Image const& image) const
{
    if (data_ != image.data_ || channel_count != image.channel_count || colorspace != image.colorspace ||
        data_type != image.data_type || data_ == nullptr || image.data_ == nullptr) {
        return false;
    }

    return (*data_ == *image.data_);
}

}
