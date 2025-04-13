// #include "image_manager.hpp"
// #include "image_converter.hpp"

// #include "debug/log.hpp"

// #include <fstream>

// namespace xen {
// void ImageManager::save_image(std::string_view filepath, Image const& image, ImageType type)
// {
//     std::ofstream f(filepath.data(), std::ios::binary);

//     ImageConverter::RawImageData image_data;
//     switch (type) {
//     case ImageType::PNG:
//         image_data = ImageConverter::convert_png(image);
//         break;
//     case ImageType::BMP:
//         image_data = ImageConverter::convert_bmp(image);
//         break;
//     case ImageType::TGA:
//         image_data = ImageConverter::convert_tga(image);
//         break;
//     case ImageType::JPG:
//         image_data = ImageConverter::convert_jpg(image);
//         break;
//     case ImageType::HDR:
//         // TODO: support HDR images
//         Log::warning(
//             "MxEngine::ImageManager",
//             " HDR texture format is not supported through ImageManager, use ImageConverter::ConvertImageHDR"
//         );
//         break;
//     }

//     f.write((char const*)image_data.data(), image_data.size());
// }

// void ImageManager::SaveTexture(std::string_view filepath, TextureHandle const& texture, ImageType type)
// {
//     ImageManager::save_image(filepath, texture->get_raw_texture_data(), type);
// }

// void ImageManager::save_texture(std::string_view filepath, TextureHandle const& texture)
// {
//     auto const path = std::filesystem::path(filepath);

//     auto ext = path.extension();
//     if (ext == ".png") {
//         ImageManager::save_texture(filepath, texture, ImageType::PNG);
//     }
//     else if (ext == ".jpg" || ext == ".jpeg") {
//         ImageManager::save_texture(filepath, texture, ImageType::JPG);
//     }
//     else if (ext == ".bmp") {
//         ImageManager::save_texture(filepath, texture, ImageType::BMP);
//     }
//     else if (ext == ".tga") {
//         ImageManager::save_texture(filepath, texture, ImageType::TGA);
//     }
//     else if (ext == ".hdr") {
//         ImageManager::save_texture(filepath, texture, ImageType::HDR);
//     }
//     else {
//         Log::warning("Xen::ImageManager", "image was not saved because extenstion was invalid: ", ext);
//     }
// }

// void ImageManager::take_screenshot(std::string_view filepath, ImageType type)
// {
//     auto screenshot = Rendering::GetRenderTexture();
//     if (!screenshot.IsValid()) {
//         MXLOG_WARNING("MxEngine::ImageManager", "cannot take screenshot at there is no viewport attached");
//         return;
//     }
//     ImageManager::save_texture(filepath, screenshot, type);
// }

// void ImageManager::take_screenshot(std::string_view filepath)
// {
//     auto ext = filePath.extension();
//     if (ext == ".png") {
//         ImageManager::TakeScreenShot(filePath, ImageType::PNG);
//     }
//     else if (ext == ".jpg" || ext == ".jpeg") {
//         ImageManager::TakeScreenShot(filePath, ImageType::JPG);
//     }
//     else if (ext == ".bmp") {
//         ImageManager::TakeScreenShot(filePath, ImageType::BMP);
//     }
//     else if (ext == ".tga") {
//         ImageManager::TakeScreenShot(filePath, ImageType::TGA);
//     }
//     else if (ext == ".hdr") {
//         ImageManager::TakeScreenShot(filePath, ImageType::HDR);
//     }
//     else {
//         MXLOG_WARNING(
//             "MxEngine::ImageManager",
//             "screenshots was not saved because extenstion was invalid: " + MxString(ext.string().c_str())
//         );
//     }
// }

// void ImageManager::flip_image(Image& image)
// {
//     auto imageByteRow = image.GetRawData();
//     auto rowByteSize = image.GetWidth() * image.GetPixelSize();
//     uint8_t* swapRow = (uint8_t*)std::malloc(rowByteSize);

//     for (size_t i = 0; i < image.GetHeight() * rowByteSize / 2; i += rowByteSize) {
//         auto currentRow = imageByteRow + i;
//         auto symmetricRow = imageByteRow + (rowByteSize * (image.GetHeight() - 1) - i);

//         // Copying current row to the swap memory allocated for it
//         std::memcpy(swapRow, currentRow, rowByteSize);

//         // Copying destination to source
//         std::memcpy(currentRow, symmetricRow, rowByteSize);

//         // Putting content of the swap (previously current row) to the symmetric row
//         std::memcpy(symmetricRow, swapRow, rowByteSize);
//     }
//     std::free((void*)swapRow);
// }

// Image ImageManager::combine_images(ArrayView<Image> images, size_t imagesPerRaw)
// {
// #if defined(MXENGINE_DEBUG)
//     for (size_t i = 1; i < images.size(); i++) {
//         MX_ASSERT(images[i - 1].GetWidth() == images[i].GetWidth());
//         MX_ASSERT(images[i - 1].GetHeight() == images[i].GetHeight());
//         MX_ASSERT(images[i - 1].GetPixelSize() == images[i].GetPixelSize());
//         MX_ASSERT(images[i - 1].IsFloatingPoint() == images[i].IsFloatingPoint());
//     }
// #endif
//     MX_ASSERT(images.size() > 1);

//     size_t width = images[0].GetWidth();
//     size_t height = images[0].GetHeight();
//     size_t pixelSize = images[0].GetPixelSize();
//     bool isFloatingPoint = images[0].IsFloatingPoint();

//     auto result = (uint8_t*)std::malloc(width * height * pixelSize * images.size());
//     MX_ASSERT(result != nullptr);

//     size_t const imagesPerColumn = images.size() / imagesPerRaw;
//     size_t const rawWidth = width * pixelSize;
//     size_t offset = 0;

//     for (size_t t1 = 0; t1 < imagesPerColumn; t1++) {
//         for (size_t i = 0; i < height; i++) {
//             for (size_t t2 = 0; t2 < imagesPerRaw; t2++) {
//                 auto& tex = images[t1 * imagesPerRaw + t2];
//                 auto currentRaw = tex.GetRawData() + i * rawWidth;
//                 std::copy(currentRaw, currentRaw + rawWidth, result + offset); //-V769
//                 offset += rawWidth;
//             }
//         }
//     }
//     return Image(result, width * imagesPerRaw, height * imagesPerColumn, pixelSize, isFloatingPoint);
// }

// Image ImageManager::combine_images(Array2D<Image>& images)
// {
//     return ImageManager::combine_images(ArrayView<Image>(images.data(), images.size()), images.width());
// }
// }