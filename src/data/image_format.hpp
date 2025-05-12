#pragma once

namespace xen {
class FilePath;
class Image;

namespace ImageFormat {
/// Loads an image from a file.
/// \param filepath File from which to load the image.
/// \param flip_vertically Flip vertically the image when loading.
/// \return Loaded image's data.
Image load(FilePath const& filepath, bool flip_vertically = false);

/// Loads an image from a byte array.
/// \param image_data Data to be loaded as image.
/// \param flip_vertically Flip vertically the image when loading.
/// \return Loaded image's data.
Image load_from_data(std::vector<uint8_t> const& image_data, bool flip_vertically = false);

/// Loads an image from a byte array.
/// \param image_data Data to be loaded as image.
/// \param data_size Size of the data to be loaded.
/// \param flip_vertically Flip vertically the image when loading.
/// \return Loaded image's data.
Image load_from_data(uint8_t const* image_data, size_t data_size, bool flip_vertically = false);

/// Saves an image to a file.
/// \param filepath File to which to save the image.
/// \param flip_vertically Flip vertically the image when saving.
/// \param image Image to export data from.
void save(FilePath const& filepath, Image const& image, bool flip_vertically = false);
}
}