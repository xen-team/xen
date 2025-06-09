#pragma once

namespace xen {
class FilePath;

struct ImageData;
using ImageDataPtr = std::unique_ptr<ImageData>;

struct ImageDataB;
using ImageDataBPtr = std::unique_ptr<ImageDataB>;

struct ImageDataF;
using ImageDataFPtr = std::unique_ptr<ImageDataF>;

enum class ImageColorspace { GRAY = 0, GRAY_ALPHA, RGB, RGBA, SRGB, SRGBA };

enum class ImageDataType { BYTE, FLOAT };

/// ImageData class, representing data held by an Image.
struct ImageData {
    ImageData() = default;
    ImageData(ImageData const&) = default;
    ImageData(ImageData&&) noexcept = default;
    ImageData& operator=(ImageData const&) = default;
    ImageData& operator=(ImageData&&) noexcept = default;
    virtual ~ImageData() = default;

    virtual ImageDataType get_data_type() const = 0;

    virtual void const* data() const = 0;
    virtual void* data() = 0;

    /// Checks if the image doesn't contain data.
    /// \return True if the image has no data, false otherwise.
    virtual bool empty() const = 0;

    /// Resizes the data container.
    /// \param size New data size.
    virtual void resize(size_t size) = 0;

    /// Checks if the current image data is equal to another given one.
    /// Datas' types must be the same.
    /// \param image_data Image data to be compared with.
    /// \return True if data are equal, false otherwise.
    virtual bool operator==(ImageData const& image_data) const = 0;

    /// Checks if the current image data is different from another given one.
    /// Datas' types must be the same.
    /// \param image_data Image data to be compared with.
    /// \return True if data are different, false otherwise.
    bool operator!=(ImageData const& image_data) const { return !(*this == image_data); }
};

/// ImageData in bytes.
struct ImageDataB final : public ImageData {
    std::vector<uint8_t> data_;

public:
    explicit ImageDataB(size_t data_size) { resize(data_size); }

    ImageDataType get_data_type() const override { return ImageDataType::BYTE; }

    void const* data() const override { return data_.data(); }
    void* data() override { return data_.data(); }

    template <typename... Args>
    static ImageDataBPtr create(Args&&... args)
    {
        return std::make_unique<ImageDataB>(std::forward<Args>(args)...);
    }

    /// Checks if the image doesn't contain data.
    /// \return True if the image has no data, false otherwise.
    bool empty() const override { return data_.empty(); }

    /// Resizes the data container.
    /// \param size New data size.
    void resize(size_t size) override { data_.resize(size); }

    /// Checks if the current byte image data is equal to another given one.
    /// The given data must be of byte type as well.
    /// \param image_data Image data to be compared with.
    /// \return True if data are equal, false otherwise.
    bool operator==(ImageData const& image_data) const override;
};

/// ImageData in floating point values (for High Dynamic Range (HDR) images).
struct ImageDataF final : public ImageData {
    std::vector<float> data_;

public:
    explicit ImageDataF(size_t data_size) { resize(data_size); }

    ImageDataType get_data_type() const override { return ImageDataType::FLOAT; }

    void const* data() const override { return data_.data(); }
    void* data() override { return data_.data(); }

    template <typename... Args>
    static ImageDataFPtr create(Args&&... args)
    {
        return std::make_unique<ImageDataF>(std::forward<Args>(args)...);
    }

    /// Checks if the image doesn't contain data.
    /// \return True if the image has no data, false otherwise.
    bool empty() const override { return data_.empty(); }

    /// Resizes the data container.
    /// \param size New data size.
    void resize(size_t size) override { data_.resize(size); }

    /// Checks if the current floating point image data is equal to another given one.
    /// The given data must be of floating point type as well.
    /// \param image_data Image data to be compared with.
    /// \return True if data are equal, false otherwise.
    bool operator==(ImageData const& image_data) const override;
};

/// Image class, handling images of different formats.
class Image {
public:
    Image() = default;
    Image(Image&&) noexcept = default;
    Image& operator=(Image const& image);
    Image& operator=(Image&&) noexcept = default;

    explicit Image(ImageColorspace colorspace, ImageDataType data_type = ImageDataType::BYTE);

    Image(Vector2ui const& size, ImageColorspace colorspace, ImageDataType data_type = ImageDataType::BYTE);

    Image(Image const& image);

    Vector2ui get_size() const { return size; }

    uint32_t get_width() const { return size.x; }

    uint32_t get_height() const { return size.y; }

    ImageColorspace get_colorspace() const { return colorspace; }

    ImageDataType get_data_type() const { return data_type; }

    uint8_t get_channel_count() const { return channel_count; }

    void const* data() const { return data_->data(); }
    void* data() { return data_->data(); }

    /// Checks if the image doesn't contain data.
    /// \return True if the image has no data, false otherwise.
    bool empty() const { return (!data_ || data_->empty()); }

    /// Gets a byte value from the image.
    /// \note The image must have a byte data type for this function to execute properly.
    /// \param width_index Width index of the value to be fetched.
    /// \param height_index Height index of the value to be fetched.
    /// \param channel_index Channel index of the value to be fetched.
    /// \return Value at the given location.
    uint8_t recover_byte_value(size_t width_index, size_t height_index, uint8_t channel_index) const;

    /// Gets a float value from the image.
    /// \note The image must have a float data type for this function to execute properly.
    /// \param width_index Width index of the value to be fetched.
    /// \param height_index Height index of the value to be fetched.
    /// \param channel_index Channel index of the value to be fetched.
    /// \return Value at the given location.
    float recover_float_value(size_t width_index, size_t height_index, uint8_t channel_index) const;

    /// Gets a single-component pixel from the image.
    /// \note This gets a single value, hence requires an image with a single channel.
    /// \tparam T Type of the pixel to be fetched.
    /// \param width_index Width index of the pixel to be fetched.
    /// \param height_index Height index of the pixel to be fetched.
    /// \return Pixel at the given location.
    template <typename T>
    T recover_pixel(size_t width_index, size_t height_index) const;

    // /// Gets a pixel from the image with multiple components.
    // /// \note The image requires having a channel count equal to the given value count.
    // /// \tparam T Type of the pixel to be fetched.
    // /// \tparam N Number of values to be fetched. Must be equal to the image's channel count.
    // /// \param width_index Width index of the pixel to be fetched.
    // /// \param height_index Height index of the pixel to be fetched.
    // /// \return Pixel at the given location.
    // template <typename T, size_t N>
    // Vector<T, N> recover_pixel(size_t width_index, size_t height_index) const;

    /// Sets a byte value in the image.
    /// \note The image must have a byte data type for this function to execute properly.
    /// \param width_index Width index of the value to be set.
    /// \param height_index Height index of the value to be set.
    /// \param channel_index Channel index of the value to be set.
    /// \param val Value to be set.
    void set_byte_value(size_t width_index, size_t height_index, uint8_t channel_index, uint8_t val);

    /// Sets a float value in the image.
    /// \note The image must have a float data type for this function to execute properly.
    /// \param width_index Width index of the value to be set.
    /// \param height_index Height index of the value to be set.
    /// \param channel_index Channel index of the value to be set.
    /// \param val Value to be set.
    void set_float_value(size_t width_index, size_t height_index, uint8_t channel_index, float val);

    /// Sets a pixel in the image.
    /// \note This sets a single value, hence requires an image with a single channel.
    /// \tparam T Type of the pixel to be set.
    /// \param width_index Width index of the pixel to be set.
    /// \param height_index Height index of the pixel to be set.
    /// \param val Value to be set.
    template <typename T>
    void set_pixel(size_t width_index, size_t height_index, T val);

    /// Checks if the current image is equal to another given one.
    /// Their inner data must be of the same type.
    /// \param image Image to be compared with.
    /// \return True if images are the same, false otherwise.
    bool operator==(Image const& image) const;

    /// Checks if the current image is different from another given one.
    /// Their inner data must be of the same type.
    /// \param image Image to be compared with.
    /// \return True if images are different, false otherwise.
    bool operator!=(Image const& image) const { return !(*this == image); }

private:
    Vector2ui size{};
    ImageColorspace colorspace{};
    ImageDataType data_type{};
    uint8_t channel_count{};

    ImageDataPtr data_{};

private:
    constexpr size_t compute_index(size_t width_index, size_t height_index) const
    {
        Log::rt_assert(width_index < size.x, "Error: The given width index is invalid.");
        Log::rt_assert(height_index < size.y, "Error: The given height index is invalid.");
        return height_index * size.x * channel_count + width_index * channel_count;
    }

    constexpr size_t compute_index(size_t width_index, size_t height_index, uint8_t channel_index) const
    {
        Log::rt_assert(channel_index < channel_count, "Error: The given channel index is invalid.");
        return compute_index(width_index, height_index) + channel_index;
    }

    template <typename T>
    T recover_value(size_t width_index, size_t height_index, uint8_t channel_index) const
    {
        size_t const value_index = compute_index(width_index, height_index, channel_index);
        return static_cast<T const*>(data_->data())[value_index];
    }

    template <typename T>
    void set_value(size_t width_index, size_t height_index, uint8_t channel_index, T val)
    {
        size_t const value_index = compute_index(width_index, height_index, channel_index);
        static_cast<T*>(data_->data())[value_index] = val;
    }
};
}

#include "image.inl"