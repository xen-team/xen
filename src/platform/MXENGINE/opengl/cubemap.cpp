#include "cubemap.hpp"

#include "platform/opengl/gl_utils.hpp"
#include "platform/opengl/image/image_loader.hpp"
#include "debug/log.hpp"

namespace xen {
void CubeMap::free()
{
    if (id != 0) {
        GL_CALL(glDeleteTextures(1, &id));
        Log::debug("OpenGL::CubeMap", " freed cubemap with id: ", id);
    }
    id = 0;
    active_id = 0;
}

CubeMap::CubeMap()
{
    GL_CALL(glGenTextures(1, &id));
    Log::debug("OpenGL::CubeMap", " created cubemap with id: ", id);
}

void CubeMap::load(std::string_view filepath)
{
    // TODO: support floating point textures
    bool flipImage = false;
    Image img = ImageLoader::load_image(filepath, flipImage);
    if (img.get_raw_data() == nullptr) {
        Log::warning("OpenGL::CubeMap", " file with name '", filepath, "' was not found");
        return;
    }
    auto images = ImageLoader::create_cubemap(img);

    this->filepath = std::filesystem::proximate(filepath);
    std::ranges::replace(this->filepath, '\\', '/');

    channels = img.get_channel_count();
    width = img.get_width();
    height = img.get_height();

    GL_CALL(glBindTexture(GL_TEXTURE_CUBE_MAP, id));
    for (size_t i = 0; i < 6; i++) {
        GL_CALL(glTexImage2D(
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + static_cast<GLenum>(i), 0, GL_RGB, static_cast<GLsizei>(images[i].size()),
            static_cast<GLsizei>(images.size()) / static_cast<GLsizei>(channels), 0, GL_RGBA, GL_UNSIGNED_BYTE,
            images[i].data()
        ));
    }

    GL_CALL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    GL_CALL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    GL_CALL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));

    generate_mipmaps();
}

void CubeMap::load(
    std::string_view right, std::string_view left, std::string_view top, std::string_view bottom,
    std::string_view front, std::string_view back
)
{
    constexpr bool flip = true;
    std::array<Image, 6> images = {
        ImageLoader::load_image(right, flip), ImageLoader::load_image(left, flip),
        ImageLoader::load_image(top, flip),   ImageLoader::load_image(bottom, flip),
        ImageLoader::load_image(front, flip), ImageLoader::load_image(back, flip),
    };
    load(images);
}

CubeMap::CubeMap(CubeMap&& other) noexcept :
    width(other.width), height(other.height), channels(other.channels), id(other.id), active_id(other.active_id)
{
    other.id = 0;
    other.active_id = 0;
    other.width = 0;
    other.height = 0;
    other.channels = 0;
}

CubeMap& CubeMap::operator=(CubeMap&& other) noexcept
{
    free();

    id = other.id;
    active_id = other.active_id;
    width = other.width;
    height = other.height;
    channels = other.channels;

    other.id = 0;
    other.active_id = 0;
    other.width = 0;
    other.height = 0;
    other.channels = 0;
    return *this;
}

CubeMap::~CubeMap()
{
    free();
}

void CubeMap::bind() const
{
    GL_CALL(glActiveTexture(GL_TEXTURE0 + active_id));
    GL_CALL(glBindTexture(GL_TEXTURE_CUBE_MAP, id));
}

void CubeMap::unbind() const
{
    GL_CALL(glActiveTexture(GL_TEXTURE0 + active_id));
    GL_CALL(glBindTexture(GL_TEXTURE_CUBE_MAP, 0));
}

CubeMap::BindableId CubeMap::get_native_handle() const
{
    return id;
}

void CubeMap::bind(CubeMapBindId id) const
{
    active_id = id;
    bind();
}

CubeMap::CubeMapBindId CubeMap::get_bound_id() const
{
    return active_id;
}

void CubeMap::load(std::array<Image, 6> const& images)
{
    this->width = images.front().get_width();
    this->height = images.front().get_height();
    this->channels = images.front().get_channel_count();
    // this->filepath = MXENGINE_MAKE_INTERNAL_TAG("raw");

    GLenum pixel_type = GL_UNSIGNED_BYTE;
    GLenum pixel_format = GL_RGBA;
    switch (channels) {
    case 1:
        pixel_format = GL_RED;
        break;
    case 2:
        pixel_format = GL_RG;
        break;
    case 3:
        pixel_format = GL_RGB;
        break;
    case 4:
        pixel_format = GL_RGBA;
        break;
    default:
        Log::error("OpenGL::Texture", " invalid channel count: ", channels);
        break;
    }

    GL_CALL(glBindTexture(GL_TEXTURE_CUBE_MAP, id));
    for (size_t i = 0; i < images.size(); i++) {
        GL_CALL(glTexImage2D(
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + static_cast<GLenum>(i), 0, GL_RGB, static_cast<GLsizei>(width),
            static_cast<GLsizei>(height), 0, pixel_format, pixel_type, images[i].get_raw_data()
        ));
    }

    GL_CALL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    GL_CALL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    GL_CALL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));

    if (std::ranges::any_of(images, [](Image const& slice) { return slice.get_raw_data() != nullptr; })) {
        generate_mipmaps();
    }
    else {
        GL_CALL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
        GL_CALL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    }
}

void CubeMap::load(std::array<uint8_t*, 6> const& data, size_t width, size_t height)
{
    this->width = width;
    this->height = height;
    channels = 3;
    // filepath = MXENGINE_MAKE_INTERNAL_TAG("raw");

    GL_CALL(glBindTexture(GL_TEXTURE_CUBE_MAP, id));
    for (size_t i = 0; i < data.size(); i++) {
        GL_CALL(glTexImage2D(
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + static_cast<GLenum>(i), 0, GL_RGB, static_cast<GLsizei>(width),
            static_cast<GLsizei>(height), 0, GL_RGBA, GL_UNSIGNED_BYTE, data[i]
        ));
    }

    GL_CALL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    GL_CALL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    GL_CALL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));

    if (std::ranges::any_of(data, [](uint8_t const* slice) { return slice != nullptr; })) {
        generate_mipmaps();
    }
    else {
        GL_CALL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
        GL_CALL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    }
}

void CubeMap::load_depth(int width, int height)
{
    this->width = width;
    this->height = height;
    // this->filepath = MXENGINE_MAKE_INTERNAL_TAG("depth");
    this->channels = 1;

    GL_CALL(glBindTexture(GL_TEXTURE_CUBE_MAP, id));
    for (size_t i = 0; i < 6; i++) {
        GL_CALL(glTexImage2D(
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + static_cast<GLenum>(i), 0, GL_DEPTH_COMPONENT, width, height, 0,
            GL_DEPTH_COMPONENT, GL_FLOAT, nullptr
        ));
    }

    GL_CALL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER));
    GL_CALL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER));
    GL_CALL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER));

    float border[] = {1.0f, 1.0f, 1.0f, 1.0f};
    GL_CALL(glTexParameterfv(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BORDER_COLOR, border));

    GL_CALL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GL_CALL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
}

// bool CubeMap::IsInternalEngineResource() const
// {
//     return this->filepath.find(MXENGINE_INTERNAL_TAG_SYMBOL) == 0;
// }

void CubeMap::set_max_lod(size_t lod)
{
    bind(0);
    GL_CALL(glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LOD, static_cast<float>(lod)));
}

void CubeMap::set_min_lod(size_t lod)
{
    bind(0);
    GL_CALL(glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_LOD, static_cast<float>(lod)));
}

void CubeMap::generate_mipmaps()
{
    bind(0);
    GL_CALL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
    GL_CALL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    GL_CALL(glGenerateMipmap(GL_TEXTURE_CUBE_MAP));
}

// MXENGINE_REFLECT_TYPE
// {
//     using SetFilePath = void (CubeMap::*)(MxString const&);

//     rttr::registration::class_<CubeMap>("CubeMap")(
//         rttr::metadata(EditorInfo::HANDLE_EDITOR, GUI::HandleEditorExtra<CubeMap>)
//     )
//         .constructor<>()
//         .property_readonly("filepath", &CubeMap::GetFilePath)(rttr::metadata(MetaInfo::FLAGS, MetaInfo::EDITABLE))
//         .property("_filepath", &CubeMap::GetFilePath, (SetFilePath)&CubeMap::Load)(
//             rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE)
//         )
//         .property_readonly("width", &CubeMap::GetWidth)(rttr::metadata(MetaInfo::FLAGS, MetaInfo::EDITABLE))
//         .property_readonly("height", &CubeMap::GetHeight)(rttr::metadata(MetaInfo::FLAGS, MetaInfo::EDITABLE))
//         .property_readonly("channel count", &CubeMap::GetChannelCount)(
//             rttr::metadata(MetaInfo::FLAGS, MetaInfo::EDITABLE)
//         )
//         .property_readonly("native handle", &CubeMap::GetNativeHandle)(
//             rttr::metadata(MetaInfo::FLAGS, MetaInfo::EDITABLE)
//         )
//         .property_readonly("editor-preview", &CubeMap::GetBoundId)(
//             rttr::metadata(MetaInfo::FLAGS, MetaInfo::EDITABLE),
//             rttr::metadata(EditorInfo::CUSTOM_VIEW, GUI::EditorExtra<CubeMap>)
//         );
// }
}