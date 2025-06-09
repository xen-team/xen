#pragma once

#include <data/owner_value.hpp>

namespace xen {
class Color;
class Image;
using TexturePtr = std::shared_ptr<class Texture>;
#if !defined(USE_OPENGL_ES)
using Texture1DPtr = std::shared_ptr<class Texture1D>;
#endif
using Texture2DPtr = std::shared_ptr<class Texture2D>;
using Texture3DPtr = std::shared_ptr<class Texture3D>;
enum class TextureType : uint32_t;

enum class TextureColorspace { INVALID = -1, GRAY = 0, RG, RGB, RGBA, SRGB, SRGBA, DEPTH };

enum class TextureDataType : uint8_t { BYTE, FLOAT16, FLOAT32 };

enum class TextureFilter : uint8_t { NEAREST, LINEAR };

enum class TextureWrapping : uint8_t { REPEAT, CLAMP };

/// Texture class, representing images or buffers to be used in the rendering process.
class Texture {
public:
    Texture(Texture const&) = delete;
    Texture(Texture&&) noexcept = default;

    Texture& operator=(Texture const&) = delete;
    Texture& operator=(Texture&&) noexcept = default;

    virtual ~Texture();

    [[nodiscard]] uint32_t get_index() const { return index; }

    [[nodiscard]] TextureColorspace get_colorspace() const { return colorspace; }

    [[nodiscard]] TextureDataType get_data_type() const { return data_type; }

    /// Binds the current texture.
    void bind() const;

    /// Unbinds the current texture.
    void unbind() const;

    void set_filter(TextureFilter filter) const { set_filter(filter, filter); }

    void set_filter(TextureFilter minify, TextureFilter magnify) const;

    void set_filter(TextureFilter minify, TextureFilter mipmapMinify, TextureFilter magnify) const;

    void set_wrapping(TextureWrapping wrapping) const;

    /// Sets the texture's colorspace & data type; the latter will be deduced from the former (float if depth
    /// colorspace, byte otherwise).
    /// \param colorspace New colorspace.
    void set_colorspace(TextureColorspace colorspace);

    /// Sets the texture's colorspace & data type.
    /// \param colorspace New colorspace.
    /// \param data_type New data type.
    void set_colorspace(TextureColorspace colorspace, TextureDataType data_type);

protected:
    OwnerValue<uint32_t> index{};
    TextureType type{};

    TextureColorspace colorspace = TextureColorspace::INVALID;
    TextureDataType data_type{};

protected:
    explicit Texture(TextureType type);

    /// Assigns default parameters after image loading. Must be called after having loaded the images' data in order to
    /// properly create the mipmaps.
    /// \param create_mipmaps True to generate texture mipmaps, false otherwise.
    void set_loaded_parameters(bool create_mipmaps) const;

    /// Generates mipmaps for the current texture.
    void generate_mipmaps() const;

    virtual void load() const = 0;
};

#if !defined(USE_OPENGL_ES)
class Texture1D final : public Texture {
public:
    Texture1D();
    explicit Texture1D(TextureColorspace colorspace) : Texture1D() { set_colorspace(colorspace); }
    Texture1D(TextureColorspace colorspace, TextureDataType data_type) : Texture1D()
    {
        set_colorspace(colorspace, data_type);
    }
    Texture1D(uint32_t width, TextureColorspace colorspace) : Texture1D(colorspace) { resize(width); }
    Texture1D(uint32_t width, TextureColorspace colorspace, TextureDataType data_type);
    /// Constructs a plain colored texture.
    /// \param color Color to fill the texture with.
    /// \param width Width of the texture to create.
    explicit Texture1D(Color const& color, uint32_t width = 1);

    uint32_t get_width() const { return width; }

    template <typename... Args>
    static Texture1DPtr create(Args&&... args)
    {
        return std::make_shared<Texture1D>(std::forward<Args>(args)...);
    }

    /// Resizes the texture.
    /// \param width New texture width.
    void resize(uint32_t width);

    /// Fills the texture with a single color.
    /// \param color Color to fill the texture with.
    void fill(Color const& color);

private:
    uint32_t width = 0;

private:
    void load() const override;
};
#endif

class Texture2D final : public Texture {
public:
    Texture2D();

    explicit Texture2D(TextureColorspace colorspace) : Texture2D() { set_colorspace(colorspace); }

    Texture2D(TextureColorspace colorspace, TextureDataType data_type) : Texture2D()
    {
        set_colorspace(colorspace, data_type);
    }

    Texture2D(Vector2ui const& size, TextureColorspace colorspace) : Texture2D(colorspace) { resize(size); }

    Texture2D(Vector2ui const& size, TextureColorspace colorspace, TextureDataType data_type);

    explicit Texture2D(Image const& image, bool create_mipmaps = true, bool should_use_srgb = false) : Texture2D()
    {
        load(image, create_mipmaps, should_use_srgb);
    }

    /// Constructs a plain colored texture. 
    /// \param color Color to fill the texture with.
    /// \param width Width of the texture to create.
    /// \param height Height of the texture to create.
    explicit Texture2D(Color const& color, Vector2ui size = Vector2ui(1));

    Vector2ui get_size() const { return size; }

    uint32_t get_width() const { return size.x; }

    uint32_t get_height() const { return size.y; }

    template <typename... Args>
    static Texture2DPtr create(Args&&... args)
    {
        return std::make_shared<Texture2D>(std::forward<Args>(args)...);
    }

    /// Resizes the texture.
    /// \param width New texture width.
    /// \param height New texture height.
    void resize(Vector2ui const& size);

    /// Loads the image's data onto the graphics card.
    /// \param image Image to load the data from.
    /// \param create_mipmaps True to generate texture mipmaps, false otherwise.
    /// \param should_use_srgb True to set an sRGB(A) colorspace if the image has an RGB(A) one, false to keep it as is.
    void load(Image const& image, bool create_mipmaps = true, bool should_use_srgb = false);

    /// Fills the texture with a single color.
    /// \param color Color to fill the texture with.
    void fill(Color const& color);

    /// Retrieves the texture's data from the GPU.
    /// \warning The pixel storage pack & unpack alignments should be set to 1 in order to recover actual pixels.
    /// \see Renderer::set_pixel_storage()
    /// \warning Retrieving an image from the GPU is slow; use this function with caution.
    /// \return Recovered image.
    Image recover_image() const;

private:
    Vector2ui size{};

private:
    void load() const override;
};

class Texture3D final : public Texture {
public:
    Texture3D();

    explicit Texture3D(TextureColorspace colorspace) : Texture3D() { set_colorspace(colorspace); }

    Texture3D(TextureColorspace colorspace, TextureDataType data_type) : Texture3D()
    {
        set_colorspace(colorspace, data_type);
    }

    Texture3D(Vector3ui const& size, TextureColorspace colorspace) : Texture3D(colorspace) { resize(size); }

    Texture3D(Vector3ui const& size, TextureColorspace colorspace, TextureDataType data_type);

    explicit Texture3D(
        std::vector<Image> const& image_slices, bool create_mipmaps = true, bool should_use_srgb = false
    ) : Texture3D()
    {
        load(image_slices, create_mipmaps, should_use_srgb);
    }

    /// Constructs a plain colored texture.
    /// \param color Color to fill the texture with.
    /// \param width Width of the texture to create.
    /// \param height Height of the texture to create.
    /// \param depth Depth of the texture to create.
    explicit Texture3D(Color const& color, Vector3ui size = Vector3ui(1));

    Vector3ui get_size() const { return size; }
    uint32_t get_width() const { return size.x; }
    uint32_t get_height() const { return size.y; }
    uint32_t get_depth() const { return size.z; }

    template <typename... Args>
    static Texture3DPtr create(Args&&... args)
    {
        return std::make_shared<Texture3D>(std::forward<Args>(args)...);
    }

    /// Resizes the texture.
    /// \param width New texture width.
    /// \param height New texture height.
    /// \param depth New texture depth.
    void resize(Vector3ui const& size);

    /// Loads the images' data onto the graphics card.
    /// \param image_slices Images along the depth to load the data from. All of them must have the same attributes
    /// (size, colorspace, ...)
    ///   and the number of images will become the texture's depth.
    /// \param create_mipmaps True to generate texture mipmaps, false otherwise.
    /// \param should_use_srgb True to set an sRGB(A) colorspace if the images have an RGB(A) one, false to keep it as
    /// is.
    void load(std::vector<Image> const& image_slices, bool create_mipmaps = true, bool should_use_srgb = false);

    /// Fills the texture with a single color.
    /// \param color Color to fill the texture with.
    void fill(Color const& color);

private:
    Vector3ui size{};

private:
    void load() const override;
};
}