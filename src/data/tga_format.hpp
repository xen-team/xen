#pragma once

namespace xen {
class FilePath;
class Image;

namespace TgaFormat {
/// Loads an image from a TGA file.
/// \param filepath File from which to load the image.
/// \param flip_vertically Flip vertically the image when loading.
/// \return Loaded image's data.
Image load(FilePath const& filepath, bool flip_vertically = false);
}
}