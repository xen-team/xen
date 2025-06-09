#include "renderer.hpp"

#include <GL/glew.h>

#include <tracy/Tracy.hpp>
#include <tracy/TracyOpenGL.hpp>

namespace xen {
namespace {
#if !defined(XEN_IS_PLATFORM_MAC) && !defined(USE_OPENGL_ES)
inline void GLAPIENTRY log_callback(
    GLenum source, GLenum type, uint32_t id, GLenum severity, int /* length */, const char* message,
    const void* /* userParam */
)
{
    if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) {
        return;
    }

    if (id == 131218) {
        return; // "Shader is being recompiled based on GL state". May be avoidable, but disabled for now
    }

    std::string error_msg = "[OpenGL]\n\t";

    switch (source) {
    case GL_DEBUG_SOURCE_API:
        error_msg += "Source: OpenGL\n\t";
        break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
        error_msg += "Source: Window system\n\t";
        break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER:
        error_msg += "Source: Shader compiler\n\t";
        break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:
        error_msg += "Source: Third party\n\t";
        break;
    case GL_DEBUG_SOURCE_APPLICATION:
        error_msg += "Source: Application\n\t";
        break;
    case GL_DEBUG_SOURCE_OTHER:
        error_msg += "Source: Other\n\t";
        break;
    default:
        break;
    }

    switch (type) {
    case GL_DEBUG_TYPE_ERROR:
        error_msg += "Type: Error\n\t";
        break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        error_msg += "Type: Deprecated behavior\n\t";
        break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        error_msg += "Type: Undefined behavior\n\t";
        break;
    case GL_DEBUG_TYPE_PORTABILITY:
        error_msg += "Type: Portability\n\t";
        break;
    case GL_DEBUG_TYPE_PERFORMANCE:
        error_msg += "Type: Performance\n\t";
        break;
    case GL_DEBUG_TYPE_OTHER:
        error_msg += "Type: Other\n\t";
        break;
    default:
        break;
    }

    error_msg += "ID: " + std::to_string(id) + "\n\t";

    switch (severity) {
    case GL_DEBUG_SEVERITY_HIGH:
        error_msg += "Severity: High\n\t";
        break;
    case GL_DEBUG_SEVERITY_MEDIUM:
        error_msg += "Severity: Medium\n\t";
        break;
    case GL_DEBUG_SEVERITY_LOW:
        error_msg += "Severity: Low\n\t";
        break;
    // Messages with a GL_DEBUG_SEVERITY_NOTIFICATION severity are ignored at the beginning of this function
    default:
        break;
    }

    error_msg += "Message: " + std::string(message);

    Log::error(error_msg);
}
#endif

inline constexpr const char* recover_gl_error_str(uint32_t error_code)
{
    switch (error_code) {
    case GL_INVALID_ENUM:
        return "Unrecognized error code (Invalid enum)";
    case GL_INVALID_VALUE:
        return "Numeric argument out of range (Invalid value)";
    case GL_INVALID_OPERATION:
        return "Operation illegal in current state (Invalid operation)";
    case GL_INVALID_FRAMEBUFFER_OPERATION:
        return "Framebuffer object is incomplete (Invalid framebuffer operation)";
    case GL_STACK_OVERFLOW:
        return "Stack overflow";
    case GL_STACK_UNDERFLOW:
        return "Stack underflow";
    case GL_OUT_OF_MEMORY:
        return "Not enough memory left (Out of memory)";
    case GL_CONTEXT_LOST:
        return "OpenGL context has been lost due to a graphics card reset (Context lost)";
    case GL_NO_ERROR:
        return "No error";
    default:
        return "Unknown error";
    }
}
}

void Renderer::init()
{
    ZoneScopedN("Renderer::initialize");

    if (initialized) {
        return;
    }

    Log::debug("[Renderer] Initializing...");

    glewExperimental = GL_TRUE;

    if (glewInit() != GLEW_OK) {
        Log::error("Failed to initialize GLEW.");
        return;
    }

    initialized = true;

    TracyGpuContext

        get_parameter(StateParameter::MAJOR_VERSION, &major_version);
    get_parameter(StateParameter::MINOR_VERSION, &minor_version);

    // Recovering supported extensions
    {
        int ext_count{};
        get_parameter(StateParameter::EXTENSION_COUNT, &ext_count);

        extensions.reserve(static_cast<size_t>(ext_count));

        for (int ext_index = 0; ext_index < ext_count; ++ext_index) {
            extensions.emplace(get_extension(static_cast<uint32_t>(ext_index)));
        }
    }

#if !defined(NDEBUG) || defined(XEN_FORCE_DEBUG_LOG)
    {
        std::string ext_msg = "[Renderer] Available extensions:";
        for (std::string const& extension : extensions) {
            ext_msg += "\n    - " + extension;
        }
        Log::debug(ext_msg);
    }
#endif

    recover_default_framebuffer_color_format();
    recover_default_framebuffer_depth_format();

#if !defined(XEN_IS_PLATFORM_MAC) &&                                                                                   \
    !defined(USE_OPENGL_ES                                                                                             \
    ) // Setting the debug message callback provokes a crash on macOS & isn't available on OpenGL ES
    if (check_version(4, 3)) {
        enable(Capability::DEBUG_OUTPUT);
        enable(Capability::DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(&log_callback, nullptr);
    }
#endif

    Log::vdebug(
        "[Renderer] Initialized; using OpenGL",
#if defined(USE_OPENGL_ES)
        "ES {}.{}",
#endif
        major_version, minor_version
    );
}

void Renderer::enable(Capability capability)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    glEnable(static_cast<uint32_t>(capability));

    print_conditional_errors();
}

void Renderer::disable(Capability capability)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    glDisable(static_cast<uint32_t>(capability));

    print_conditional_errors();
}

bool Renderer::is_enabled(Capability capability)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    bool const is_enabled = (glIsEnabled(static_cast<uint32_t>(capability)) == GL_TRUE);

    print_conditional_errors();

    return is_enabled;
}

std::string Renderer::get_context_info(ContextInfo info)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    std::string res = reinterpret_cast<char const*>(glGetString(static_cast<uint32_t>(info)));

    print_conditional_errors();

    return res;
}

std::string Renderer::get_extension(uint32_t ext_index)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");
#if defined(XEN_CONFIG_DEBUG)
    int ext_count{};
    get_parameter(StateParameter::EXTENSION_COUNT, &ext_count);
    Log::rt_assert(
        "Error: Extension index must be less than the total extension count." && static_cast<int>(ext_index) < ext_count
    );
#endif

    std::string extension = reinterpret_cast<const char*>(glGetStringi(GL_EXTENSIONS, ext_index));

    print_conditional_errors();

    return extension;
}

void Renderer::get_parameter(StateParameter parameter, unsigned char* values)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    glGetBooleanv(static_cast<uuint32_tint>(parameter), values);

    print_conditional_errors();
}

void Renderer::get_parameter(StateParameter parameter, int* values)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    glGetIntegerv(static_cast<uint32_t>(parameter), values);

    print_conditional_errors();
}

void Renderer::get_parameter(StateParameter parameter, int64_t* values)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    glGetInteger64v(static_cast<uint32_t>(parameter), values);

    print_conditional_errors();
}

void Renderer::get_parameter(StateParameter parameter, float* values)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    glGetFloatv(static_cast<uint32_t>(parameter), values);

    print_conditional_errors();
}

void Renderer::get_parameter(StateParameter parameter, double* values)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    glGetDoublev(static_cast<uint32_t>(parameter), values);

    print_conditional_errors();
}

void Renderer::get_parameter(StateParameter parameter, uint32_t index, unsigned char* values)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    glGetBooleani_v(static_cast<uint32_t>(parameter), index, values);

    print_conditional_errors();
}

void Renderer::get_parameter(StateParameter parameter, uint32_t index, int* values)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    glGetIntegeri_v(static_cast<uint32_t>(parameter), index, values);

    print_conditional_errors();
}

void Renderer::get_parameter(StateParameter parameter, uint32_t index, int64_t* values)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    glGetInteger64i_v(static_cast<uint32_t>(parameter), index, values);

    print_conditional_errors();
}

uint32_t Renderer::get_active_texture()
{
    int texture{};
    get_parameter(StateParameter::ACTIVE_TEXTURE, &texture);

    return static_cast<uint32_t>(texture - GL_TEXTURE0);
}

uint32_t Renderer::get_current_program()
{
    int program{};
    get_parameter(StateParameter::CURRENT_PROGRAM, &program);

    return static_cast<uint32_t>(program);
}

void Renderer::clear_color(Color const& color)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    glClearColor(color.r, color.g, color.b, color.a);

    print_conditional_errors();
}

void Renderer::clear(MaskType mask)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    TracyGpuZone("Renderer::clear")

        glClear(static_cast<uint32_t>(mask));

    print_conditional_errors();
}

void Renderer::set_depth_function(DepthStencilFunction func)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    TracyGpuZone("Renderer::set_depth_function")

        glDepthFunc(static_cast<uint32_t>(func));

    print_conditional_errors();
}

void Renderer::set_stencil_function(DepthStencilFunction func, int ref, uint32_t mask, FaceOrientation orientation)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    glStencilFuncSeparate(static_cast<uint32_t>(orientation), static_cast<uint32_t>(func), ref, mask);

    print_conditional_errors();
}

void Renderer::set_stencil_operations(
    StencilOperation stencil_fail_op, StencilOperation depth_fail_op, StencilOperation success_op,
    FaceOrientation orientation
)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    glStencilOpSeparate(
        static_cast<uint32_t>(stencil_fail_op), static_cast<uint32_t>(depth_fail_op), static_cast<uint32_t>(success_op),
        static_cast<uint32_t>(orientation)
    );

    print_conditional_errors();
}

void Renderer::set_stencil_mask(uint32_t mask, FaceOrientation orientation)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    glStencilMaskSeparate(static_cast<uint32_t>(orientation), mask);

    print_conditional_errors();
}

void Renderer::set_blend_function(BlendFactor source, BlendFactor destination)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    glBlendFunc(static_cast<uint32_t>(source), static_cast<uint32_t>(destination));

    print_conditional_errors();
}

void Renderer::set_face_cull(FaceOrientation orientation)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    glCullFace(static_cast<uint32_t>(orientation));

    print_conditional_errors();
}

#if !defined(USE_OPENGL_ES)
void Renderer::set_polygon_mode(FaceOrientation orientation, PolygonMode mode)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    glPolygonMode(static_cast<uint32_t>(orientation), static_cast<uint32_t>(mode));

    print_conditional_errors();
}

void Renderer::set_clip_control(ClipOrigin origin, ClipDepth depth)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");
    Log::rt_assert(
        check_version(4, 5) || is_extension_supported("GL_ARB_clip_control"),
        "Error: Setting clip control requires OpenGL 4.5+ or the 'GL_ARB_clip_control' extension."
    );

    glClipControl(static_cast<uint32_t>(origin), static_cast<uint32_t>(depth));

    print_conditional_errors();
}

void Renderer::set_patch_vertex_count(int value)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");
    Log::rt_assert(
        check_version(4, 0) || is_extension_supported("GL_ARB_tessellation_shader"),
        "Error: Setting patch vertices requires OpenGL 4.0+ or the 'GL_ARB_tessellation_shader' extension."
    );

    Log::rt_assert(value > 0, "Error: A patch needs at least one vertex.");

    glPatchParameteri(GL_PATCH_VERTICES, value);

    print_conditional_errors();
}

void Renderer::set_patch_parameter(PatchParameter param, float const* values)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");
    Log::rt_assert(
        check_version(4, 0) || is_extension_supported("GL_ARB_tessellation_shader"),
        "Error: Setting a patch parameter requires OpenGL 4.0+ or the 'GL_ARB_tessellation_shader' extension."
    );

    glPatchParameterfv(static_cast<uint32_t>(param), values);

    print_conditional_errors();
}
#endif

void Renderer::set_pixel_storage(PixelStorage storage, uint32_t value)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    glPixelStorei(static_cast<uint32_t>(storage), static_cast<int>(value));

#if !defined(NDEBUG) && !defined(XEN_SKIP_RENDERER_ERRORS)
    const ErrorCodes error_codes = Renderer::recover_errors();

    if (error_codes[ErrorCode::INVALID_VALUE])
        Log::verror(
            "Renderer::set_pixel_storage - {} is not a valid alignment value. Only 1, 2, 4 & 8 are accepted.", value
        );
#endif
}

void Renderer::recover_frame(Vector2ui const& size, TextureFormat format, PixelDataType data_type, void* data)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    TracyGpuZone("Renderer::recover_frame")

        glReadPixels(
            0, 0, static_cast<int>(size.x), static_cast<int>(size.y), static_cast<uint32_t>(format),
            static_cast<uint32_t>(data_type), data
        );

    print_conditional_errors();
}

void Renderer::generate_vertex_arrays(uint32_t count, uint32_t* indices)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    glGenVertexArrays(static_cast<int>(count), indices);

    print_conditional_errors();
}

void Renderer::bind_vertex_array(uint32_t index)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    glBindVertexArray(index);

    print_conditional_errors();
}

void Renderer::enable_vertex_attrib_array(uint32_t index)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    glEnableVertexAttribArray(index);

    print_conditional_errors();
}

void Renderer::set_vertex_attrib(
    uint32_t index, AttribDataType data_type, uint8_t size, uint32_t stride, uint32_t offset, bool normalize
)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    glVertexAttribPointer(
        index, size, static_cast<uint32_t>(data_type), normalize, static_cast<int>(stride),
        reinterpret_cast<void const*>(offset)
    );

    print_conditional_errors();
}

void Renderer::set_vertex_attribDivisor(uint32_t index, uint32_t divisor)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    glVertexAttribDivisor(index, divisor);

    print_conditional_errors();
}

void Renderer::delete_vertex_arrays(uint32_t count, uint32_t* indices)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    glDeleteVertexArrays(static_cast<int>(count), indices);

    print_conditional_errors();
}

void Renderer::generate_buffers(uint32_t count, uint32_t* indices)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    glGenBuffers(static_cast<int>(count), indices);

    print_conditional_errors();
}

void Renderer::bind_buffer(BufferType type, uint32_t index)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    glBindBuffer(static_cast<uint32_t>(type), index);

    print_conditional_errors();
}

void Renderer::bind_buffer_base(BufferType type, uint32_t binding_index, uint32_t buffer_index)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    glBindBufferBase(static_cast<uint32_t>(type), binding_index, buffer_index);

    print_conditional_errors();
}

void Renderer::bind_buffer_range(
    BufferType type, uint32_t binding_index, uint32_t buffer_index, std::ptrdiff_t offset, std::ptrdiff_t size
)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    glBindBufferRange(static_cast<uint32_t>(type), binding_index, buffer_index, offset, size);

    print_conditional_errors();
}

void Renderer::send_buffer_data(BufferType type, std::ptrdiff_t size, void const* data, BufferDataUsage usage)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    glBufferData(static_cast<uint32_t>(type), size, data, static_cast<uint32_t>(usage));

    print_conditional_errors();
}

void Renderer::send_buffer_sub_data(BufferType type, std::ptrdiff_t offset, std::ptrdiff_t data_size, void const* data)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    glBufferSubData(static_cast<uint32_t>(type), offset, data_size, data);

    print_conditional_errors();
}

void Renderer::delete_buffers(uint32_t count, uint32_t* indices)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    glDeleteBuffers(static_cast<int>(count), indices);

    print_conditional_errors();
}

bool Renderer::is_texture(uint32_t index)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    bool const is_texture = (glIsTexture(index) == GL_TRUE);

    print_conditional_errors();

    return is_texture;
}

void Renderer::generate_textures(uint32_t count, uint32_t* indices)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    glGenTextures(static_cast<int>(count), indices);

    print_conditional_errors();
}

void Renderer::bind_texture(TextureType type, uint32_t index)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    glBindTexture(static_cast<uint32_t>(type), index);

    print_conditional_errors();
}

#if !defined(USE_WEBGL)
void Renderer::bind_image_texture(
    uint32_t image_unit_index, uint32_t texture_index, int texture_level, bool is_layered, int layer,
    ImageAccess image_access, ImageInternalFormat image_format
)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");
#if !defined(USE_OPENGL_ES)
    Log::rt_assert(check_version(4, 2), "Error: Binding an image texture requires OpenGL 4.2+.");
#else
    Log::rt_assert(check_version(3, 1), "Error: Binding an image texture requires OpenGL ES 3.1+.");
#endif

    glBindImageTexture(
        image_unit_index, texture_index, texture_level, is_layered, layer, static_cast<uint32_t>(image_access),
        static_cast<uint32_t>(image_format)
    );

    print_conditional_errors();
}
#endif

void Renderer::activate_texture(uint32_t index)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    glActiveTexture(GL_TEXTURE0 + index);

    print_conditional_errors();
}

void Renderer::set_texture_parameter(TextureType type, TextureParam param, int value)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    glTexParameteri(static_cast<uint32_t>(type), static_cast<uint32_t>(param), value);

    print_conditional_errors();
}

void Renderer::set_texture_parameter(TextureType type, TextureParam param, float value)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    glTexParameterf(static_cast<uint32_t>(type), static_cast<uint32_t>(param), value);

    print_conditional_errors();
}

void Renderer::set_texture_parameter(TextureType type, TextureParam param, int const* values)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    glTexParameteriv(static_cast<uint32_t>(type), static_cast<uint32_t>(param), values);

    print_conditional_errors();
}

void Renderer::set_texture_parameter(TextureType type, TextureParam param, float const* values)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    glTexParameterfv(static_cast<uint32_t>(type), static_cast<uint32_t>(param), values);

    print_conditional_errors();
}

#if !defined(USE_OPENGL_ES)
void Renderer::set_texture_parameter(uint32_t texture_index, TextureParam param, int value)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");
    Log::rt_assert(check_version(4, 5), "Error: OpenGL 4.5+ is needed to set a parameter with a texture index.");

    glTextureParameteri(texture_index, static_cast<uint32_t>(param), value);

    print_conditional_errors();
}

void Renderer::set_texture_parameter(uint32_t texture_index, TextureParam param, float value)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");
    Log::rt_assert(check_version(4, 5), "Error: OpenGL 4.5+ is needed to set a parameter with a texture index.");

    glTextureParameterf(texture_index, static_cast<uint32_t>(param), value);

    print_conditional_errors();
}

void Renderer::set_texture_parameter(uint32_t texture_index, TextureParam param, int const* values)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");
    Log::rt_assert(check_version(4, 5), "Error: OpenGL 4.5+ is needed to set a parameter with a texture index.");

    glTextureParameteriv(texture_index, static_cast<uint32_t>(param), values);

    print_conditional_errors();
}

void Renderer::set_texture_parameter(uint32_t texture_index, TextureParam param, float const* values)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");
    Log::rt_assert(check_version(4, 5), "Error: OpenGL 4.5+ is needed to set a parameter with a texture index.");

    glTextureParameterfv(texture_index, static_cast<uint32_t>(param), values);

    print_conditional_errors();
}

void Renderer::send_image_data_1d(
    TextureType type, uint32_t mipmap_level, TextureInternalFormat internal_format, uint32_t width,
    TextureFormat format, PixelDataType data_type, void const* data
)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    TracyGpuZone("Renderer::send_image_data_1d")

        glTexImage1D(
            static_cast<uint32_t>(type), static_cast<int>(mipmap_level), static_cast<int>(internal_format),
            static_cast<int>(width), 0, static_cast<uint32_t>(format), static_cast<uint32_t>(data_type), data
        );

    print_conditional_errors();
}

void Renderer::send_image_sub_data_1d(
    TextureType type, uint32_t mipmap_level, uint32_t offsetX, uint32_t width, TextureFormat format,
    PixelDataType data_type, void const* data
)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    TracyGpuZone("Renderer::send_image_sub_data_1d")

        glTexSubImage1D(
            static_cast<uint32_t>(type), static_cast<int>(mipmap_level), static_cast<int>(offsetX),
            static_cast<int>(width), static_cast<uint32_t>(format), static_cast<uint32_t>(data_type), data
        );

    print_conditional_errors();
}
#endif

void Renderer::send_image_data_2d(
    TextureType type, uint32_t mipmap_level, TextureInternalFormat internal_format, Vector2ui const& size,
    TextureFormat format, PixelDataType data_type, const void* data
)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    TracyGpuZone("Renderer::send_image_data_2d")

        glTexImage2D(
            static_cast<uint32_t>(type), static_cast<int>(mipmap_level), static_cast<int>(internal_format),
            static_cast<int>(size.x), static_cast<int>(size.y), 0, static_cast<uint32_t>(format),
            static_cast<uint32_t>(data_type), data
        );

    print_conditional_errors();
}

void Renderer::send_image_sub_data_2d(
    TextureType type, uint32_t mipmap_level, Vector2ui const& offset, Vector2ui const& size, TextureFormat format,
    PixelDataType data_type, void const* data
)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    TracyGpuZone("Renderer::send_image_sub_data_2d")

        glTexSubImage2D(
            static_cast<uint32_t>(type), static_cast<int>(mipmap_level), static_cast<int>(offset.x),
            static_cast<int>(offset.y), static_cast<int>(size.x), static_cast<int>(size.y),
            static_cast<uint32_t>(format), static_cast<uint32_t>(data_type), data
        );

    print_conditional_errors();
}

void Renderer::send_image_data_3d(
    TextureType type, uint32_t mipmap_level, TextureInternalFormat internal_format, Vector3ui const& size,
    TextureFormat format, PixelDataType data_type, void const* data
)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    TracyGpuZone("Renderer::send_image_data_3d")

        glTexImage3D(
            static_cast<uint32_t>(type), static_cast<int>(mipmap_level), static_cast<int>(internal_format),
            static_cast<int>(size.x), static_cast<int>(size.y), static_cast<int>(size.z), 0,
            static_cast<uint32_t>(format), static_cast<uint32_t>(data_type), data
        );

    print_conditional_errors();
}

void Renderer::send_image_sub_data_3d(
    TextureType type, uint32_t mipmap_level, Vector3ui const& offset, Vector3ui const& size, TextureFormat format,
    PixelDataType data_type, void const* data
)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    TracyGpuZone("Renderer::send_image_sub_data_3d")

        glTexSubImage3D(
            static_cast<uint32_t>(type), static_cast<int>(mipmap_level), static_cast<int>(offset.x),
            static_cast<int>(offset.y), static_cast<int>(offset.z), static_cast<int>(size.x), static_cast<int>(size.y),
            static_cast<int>(size.z), static_cast<uint32_t>(format), static_cast<uint32_t>(data_type), data
        );

    print_conditional_errors();
}

#if !defined(USE_OPENGL_ES)
void Renderer::recover_texture_attribute(
    TextureType type, uint32_t mipmap_level, TextureAttribute attribute, int* values
)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    glGetTexLevelParameteriv(
        static_cast<uint32_t>(type), static_cast<int>(mipmap_level), static_cast<uint32_t>(attribute), values
    );

    print_conditional_errors();
}

void Renderer::recover_texture_attribute(
    TextureType type, uint32_t mipmap_level, TextureAttribute attribute, float* values
)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    glGetTexLevelParameterfv(
        static_cast<uint32_t>(type), static_cast<int>(mipmap_level), static_cast<uint32_t>(attribute), values
    );

    print_conditional_errors();
}

int Renderer::recover_texture_width(TextureType type, uint32_t mipmap_level)
{
    int width{};
    recover_texture_attribute(type, mipmap_level, TextureAttribute::WIDTH, &width);

    return width;
}

int Renderer::recover_texture_height(TextureType type, uint32_t mipmap_level)
{
    int height{};
    recover_texture_attribute(type, mipmap_level, TextureAttribute::HEIGHT, &height);

    return height;
}

int Renderer::recover_texture_depth(TextureType type, uint32_t mipmap_level)
{
    int depth{};
    recover_texture_attribute(type, mipmap_level, TextureAttribute::DEPTH, &depth);

    return depth;
}

TextureInternalFormat Renderer::recover_texture_internal_format(TextureType type, uint32_t mipmap_level)
{
    int format{};
    recover_texture_attribute(type, mipmap_level, TextureAttribute::INTERNAL_FORMAT, &format);

    return static_cast<TextureInternalFormat>(format);
}

void Renderer::recover_texture_data(
    TextureType type, uint32_t mipmap_level, TextureFormat format, PixelDataType data_type, void* data
)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    TracyGpuZone("Renderer::recover_texture_data")

        glGetTexImage(
            static_cast<uint32_t>(type), static_cast<int>(mipmap_level), static_cast<uint32_t>(format),
            static_cast<uint32_t>(data_type), data
        );

    print_conditional_errors();
}
#endif

void Renderer::generate_mipmap(TextureType type)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    TracyGpuZone("Renderer::generate_mipmap")

        glGenerateMipmap(static_cast<uint32_t>(type));

    print_conditional_errors();
}

#if !defined(USE_OPENGL_ES)
void Renderer::generate_mipmap(uint32_t texture_index)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");
    Log::rt_assert(check_version(4, 5), "Error: OpenGL 4.5+ is needed to generate mipmap with a texture index");

    TracyGpuZone("Renderer::generate_mipmap")

        glGenerateTextureMipmap(texture_index);

    print_conditional_errors();
}
#endif

void Renderer::delete_textures(uint32_t count, uint32_t* indices)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    glDeleteTextures(static_cast<int>(count), indices);

    print_conditional_errors();
}

void Renderer::resize_viewport(Vector2ui const& position, Vector2ui const& size)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    glViewport(
        static_cast<int>(position.x), static_cast<int>(position.y), static_cast<int>(size.x), static_cast<int>(size.y)
    );

    print_conditional_errors();
}

uint32_t Renderer::create_program()
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    uint32_t const program_index = glCreateProgram();

    print_conditional_errors();

    return program_index;
}

void Renderer::get_program_parameter(uint32_t index, ProgramParameter parameter, int* parameters)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    glGetProgramiv(index, static_cast<uint32_t>(parameter), parameters);

    print_conditional_errors();
}

bool Renderer::is_program_linked(uint32_t index)
{
    int linkStatus{};
    get_program_parameter(index, ProgramParameter::LINK_STATUS, &linkStatus);

    return (linkStatus == GL_TRUE);
}

uint32_t Renderer::recover_active_uniform_count(uint32_t program_index)
{
    int uniformCount{};
    get_program_parameter(program_index, ProgramParameter::ACTIVE_UNIFORMS, &uniformCount);

    return static_cast<uint32_t>(uniformCount);
}

std::vector<uint32_t> Renderer::recover_attached_shaders(uint32_t program_index)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    int attached_shader_count{};
    get_program_parameter(program_index, ProgramParameter::ATTACHED_SHADERS, &attached_shader_count);

    if (attached_shader_count == 0) {
        return {};
    }

    std::vector<uint32_t> shader_indices(static_cast<size_t>(attached_shader_count));

    int recovered_shader_count{};
    glGetAttachedShaders(program_index, attached_shader_count, &recovered_shader_count, shader_indices.data());

    print_conditional_errors();

    if (recovered_shader_count == 0) {
        return {};
    }

    return shader_indices;
}

void Renderer::link_program(uint32_t index)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    TracyGpuZone("Renderer::link_program")

        glLinkProgram(index);

    if (!is_program_linked(index)) {
        char info_log[512];

        glGetProgramInfoLog(index, static_cast<int>(std::size(info_log)), nullptr, info_log);
        Log::verror("Shader program link failed (ID {}) {}", index, info_log);
    }

    print_conditional_errors();
}

void Renderer::use_program(uint32_t index)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    TracyGpuZone("Renderer::use_program")

        glUseProgram(index);

#if !defined(NDEBUG) && !defined(XEN_SKIP_RENDERER_ERRORS)
    const ErrorCodes error_codes = Renderer::recover_errors();

    if (error_codes[ErrorCode::INVALID_VALUE])
        Log::error("Renderer::use_program - Invalid shader program index (" + std::to_string(index) + ")");

    if (error_codes[ErrorCode::INVALID_OPERATION]) {
        std::string error_msg = "Renderer::use_program - ";

        if (!is_program_linked(index)) {
            error_msg += "A shader program must be linked before being defined as used.";
        }
        else {
            error_msg += "Unknown invalid operation.";
        }

        Log::error(error_msg);
    }
#endif
}

void Renderer::delete_program(uint32_t index)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    glDeleteProgram(index);

    print_conditional_errors();
}

uint32_t Renderer::create_shader(ShaderType type)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");
#if !defined(USE_OPENGL_ES)
    Log::rt_assert(
        (type != ShaderType::TESSELLATION_CONTROL && type != ShaderType::TESSELLATION_EVALUATION) ||
            check_version(4, 0) || is_extension_supported("GL_ARB_tessellation_shader"),
        "Error: Creating a tessellation shader requires OpenGL 4.0+ or the 'GL_ARB_tessellation_shader' extension."
    );
    Log::rt_assert(
        type != ShaderType::COMPUTE || check_version(4, 3) || is_extension_supported("GL_ARB_compute_shader"),
        "Error: Creating a compute shader requires OpenGL 4.3+ or the 'GL_ARB_compute_shader' extension."
    );
#else
    Log::rt_assert(type != ShaderType::GEOMETRY, "Error: Geometry shaders are unsupported with OpenGL ES.");
    Log::rt_assert(
        type != ShaderType::TESSELLATION_CONTROL && type != ShaderType::TESSELLATION_EVALUATION,
        "Error: Tessellation shaders are unsupported with OpenGL ES."
    );
    Log::rt_assert(
        type != ShaderType::COMPUTE || check_version(3, 1) || is_extension_supported("GL_ARB_compute_shader"),
        "Error: Creating a compute shader requires OpenGL ES 3.1+ or the 'GL_ARB_compute_shader' extension."
    );
#endif

    const uint32_t shader_index = glCreateShader(static_cast<uint32_t>(type));

    print_conditional_errors();

    return shader_index;
}

int Renderer::recover_shader_info(uint32_t index, ShaderInfo info)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    int res{};
    glGetShaderiv(index, static_cast<uint32_t>(info), &res);

    print_conditional_errors();

    return res;
}

void Renderer::send_shader_source(uint32_t index, char const* source, int length)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    TracyGpuZone("Renderer::send_shader_source")

        glShaderSource(index, 1, &source, &length);

    print_conditional_errors();
}

std::string Renderer::recover_shader_source(uint32_t index)
{
    TracyGpuZone("Renderer::recover_shader_source")

        const int source_length = recover_shader_info(index, ShaderInfo::SOURCE_LENGTH);

    if (source_length == 0) {
        return {};
    }

    std::string source;
    source.resize(static_cast<size_t>(source_length - 1)
    ); // The recovered length includes the null terminator, hence the -1

    glGetShaderSource(index, source_length, nullptr, source.data());

    print_conditional_errors();

    return source;
}

void Renderer::compile_shader(uint32_t index)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    TracyGpuZone("Renderer::compile_shader")

        glCompileShader(index);

    if (!is_shader_compiled(index)) {
        char info_log[512];

        glGetShaderInfoLog(index, static_cast<int>(std::size(info_log)), nullptr, info_log);
        Log::verror("Shader compilation failed (ID {}): {}", index, info_log);
    }

    print_conditional_errors();
}

void Renderer::attach_shader(uint32_t program_index, uint32_t shader_index)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    glAttachShader(program_index, shader_index);

    print_conditional_errors();
}

void Renderer::detach_shader(uint32_t program_index, uint32_t shader_index)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    glDetachShader(program_index, shader_index);

    print_conditional_errors();
}

bool Renderer::is_shader_attached(uint32_t program_index, uint32_t shader_index)
{
    std::vector<uint32_t> const shader_indices = recover_attached_shaders(program_index);
    return (std::find(shader_indices.cbegin(), shader_indices.cend(), shader_index) != shader_indices.cend());
}

void Renderer::delete_shader(uint32_t index)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    glDeleteShader(index);

    print_conditional_errors();
}

int Renderer::recover_uniform_location(uint32_t program_index, char const* uniform_name)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    int const location = glGetUniformLocation(program_index, uniform_name);

#if !defined(NDEBUG) && !defined(XEN_SKIP_RENDERER_ERRORS)
    print_errors();

    if (location == -1) {
        Log::vwarning("Uniform '{}' unrecognized ", uniform_name);
    }
#endif

    return location;
}

void Renderer::recover_uniform_info(
    uint32_t program_index, uint32_t uniform_index, UniformType& type, std::string& name, int* size
)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    int name_length{};
    int uniform_size{};
    uint32_t uniform_type{};
    std::array<char, 256> uniform_name{};

    glGetActiveUniform(
        program_index, uniform_index, static_cast<int>(uniform_name.size()), &name_length, &uniform_size, &uniform_type,
        uniform_name.data()
    );

    type = static_cast<UniformType>(uniform_type);

    name.resize(static_cast<size_t>(name_length));
    std::copy(uniform_name.cbegin(), uniform_name.cbegin() + name_length, name.begin());

    if (size) {
        *size = uniform_size;
    }

#if !defined(NDEBUG) && !defined(XEN_SKIP_RENDERER_ERRORS)
    const ErrorCodes error_codes = recover_errors();

    if (error_codes.empty()) {
        return;
    }

    if (error_codes[ErrorCode::INVALID_OPERATION]) {
        Log::error("Renderer::recover_uniform_info - Tried to fetch program information from a non-program object.");
    }

    if (error_codes[ErrorCode::INVALID_VALUE]) {
        std::string error_msg = "Renderer::recover_uniform_info - ";

        uint32_t const uni_count = recover_active_uniform_count(program_index);

        if (uniform_index >= uni_count) {
            error_msg += "The given uniform index (" + std::to_string(uniform_index) +
                         ") "
                         "is greater than or equal to the program's active uniform count (" +
                         std::to_string(uni_count) + ").";
        }
        else {
            error_msg += "The given program index has not been created by OpenGL.";
        }

        Log::error(error_msg);
    }
#endif
}

UniformType Renderer::recover_uniform_type(uint32_t program_index, uint32_t uniform_index)
{
    UniformType type{};
    std::string name;
    recover_uniform_info(program_index, uniform_index, type, name);

    return type;
}

std::string Renderer::recover_uniform_name(uint32_t program_index, uint32_t uniform_index)
{
    UniformType type{};
    std::string name;
    recover_uniform_info(program_index, uniform_index, type, name);

    return name;
}

void Renderer::recover_uniform_data(uint32_t program_index, int uniform_index, int* data)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    glGetUniformiv(program_index, uniform_index, data);

    print_conditional_errors();
}

void Renderer::recover_uniform_data(uint32_t program_index, int uniform_index, uint32_t* data)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    glGetUniformuiv(program_index, uniform_index, data);

    print_conditional_errors();
}

void Renderer::recover_uniform_data(uint32_t program_index, int uniform_index, float* data)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    glGetUniformfv(program_index, uniform_index, data);

    print_conditional_errors();
}

#if !defined(USE_OPENGL_ES)
void Renderer::recover_uniform_data(uint32_t program_index, int uniform_index, double* data)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");
    Log::rt_assert(check_version(4, 0), "Error: Recovering uniform data of type double requires OpenGL 4.0+.");

    glGetUniformdv(program_index, uniform_index, data);

    print_conditional_errors();
}
#endif

void Renderer::bind_uniform_block(uint32_t program_index, uint32_t uniform_block_index, uint32_t binding_index)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    glUniformBlockBinding(program_index, uniform_block_index, binding_index);

    print_conditional_errors();
}

uint32_t Renderer::recover_uniform_block_index(uint32_t program_index, char const* uniformBlockName)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    uint32_t const index = glGetUniformBlockIndex(program_index, uniformBlockName);

    print_conditional_errors();

    return index;
}

void Renderer::send_uniform(int uniform_index, int value)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    glUniform1i(uniform_index, value);

    print_conditional_errors();
}

void Renderer::send_uniform(int uniform_index, uint32_t value)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    glUniform1ui(uniform_index, value);

    print_conditional_errors();
}

void Renderer::send_uniform(int uniform_index, float value)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    glUniform1f(uniform_index, value);

    print_conditional_errors();
}

void Renderer::send_uniform_vector1fi(int uniform_index, int const* values, int count)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    glUniform1iv(uniform_index, count, values);

    print_conditional_errors();
}

void Renderer::send_uniform_Vector2i(int uniform_index, int const* values, int count)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    glUniform2iv(uniform_index, count, values);

    print_conditional_errors();
}

void Renderer::send_uniform_Vector3i(int uniform_index, int const* values, int count)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    glUniform3iv(uniform_index, count, values);

    print_conditional_errors();
}

void Renderer::send_uniform_Vector4i(int uniform_index, int const* values, int count)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    glUniform4iv(uniform_index, count, values);

    print_conditional_errors();
}

void Renderer::send_uniform_vector1fui(int uniform_index, uint32_t const* values, int count)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    glUniform1uiv(uniform_index, count, values);

    print_conditional_errors();
}

void Renderer::send_uniform_Vector2ui(int uniform_index, uint32_t const* values, int count)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    glUniform2uiv(uniform_index, count, values);

    print_conditional_errors();
}

void Renderer::send_uniform_Vector3ui(int uniform_index, uint32_t const* values, int count)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    glUniform3uiv(uniform_index, count, values);

    print_conditional_errors();
}

void Renderer::send_uniform_Vector4ui(int uniform_index, uint32_t const* values, int count)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    glUniform4uiv(uniform_index, count, values);

    print_conditional_errors();
}

void Renderer::send_uniform_vector1f(int uniform_index, float const* values, int count)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    glUniform1fv(uniform_index, count, values);

    print_conditional_errors();
}

void Renderer::send_uniform_Vector2f(int uniform_index, float const* values, int count)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    glUniform2fv(uniform_index, count, values);

    print_conditional_errors();
}

void Renderer::send_uniform_Vector3f(int uniform_index, float const* values, int count)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    glUniform3fv(uniform_index, count, values);

    print_conditional_errors();
}

void Renderer::send_uniform_Vector4f(int uniform_index, float const* values, int count)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    glUniform4fv(uniform_index, count, values);

    print_conditional_errors();
}

void Renderer::send_uniform_Matrix2(int uniform_index, float const* values, int count, bool transpose)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    glUniformMatrix2fv(uniform_index, count, transpose, values);

    print_conditional_errors();
}

void Renderer::send_uniform_Matrix3(int uniform_index, float const* values, int count, bool transpose)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    glUniformMatrix3fv(uniform_index, count, transpose, values);

    print_conditional_errors();
}

void Renderer::send_uniform_Matrix4(int uniform_index, float const* values, int count, bool transpose)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    glUniformMatrix4fv(uniform_index, count, transpose, values);

    print_conditional_errors();
}

void Renderer::send_uniform_color(int uniform_index, float const* values, int count)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    glUniform3fv(uniform_index, count, values);

    print_conditional_errors();
}

void Renderer::generate_framebuffers(int count, uint32_t* indices)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    glGenFramebuffers(count, indices);

    print_conditional_errors();
}

void Renderer::bind_framebuffer(uint32_t index, FramebufferType type)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    glBindFramebuffer(static_cast<uint32_t>(type), index);

#if !defined(NDEBUG) && !defined(XEN_SKIP_RENDERER_ERRORS)
    const ErrorCodes error_codes = recover_errors();

    if (error_codes[ErrorCode::INVALID_OPERATION]) {
        Log::error("Renderer::bind_framebuffer - Bound object is not a valid framebuffer.");
    }
#endif
}

FramebufferStatus Renderer::get_framebuffer_status(FramebufferType type)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    uint32_t const status = glCheckFramebufferStatus(static_cast<uint32_t>(type));

    print_conditional_errors();

    return static_cast<FramebufferStatus>(status);
}

#if !defined(USE_OPENGL_ES)
void Renderer::set_framebuffer_texture(
    FramebufferAttachment attachment, uint32_t texture_index, uint32_t mipmap_level, FramebufferType type
)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    TracyGpuZone("Renderer::set_framebuffer_texture")

        glFramebufferTexture(
            static_cast<uint32_t>(type), static_cast<uint32_t>(attachment), texture_index,
            static_cast<int>(mipmap_level)
        );

    print_conditional_errors();
}

void Renderer::set_framebuffer_texture_1d(
    FramebufferAttachment attachment, uint32_t texture_index, uint32_t mipmap_level, FramebufferType type
)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    TracyGpuZone("Renderer::set_framebuffer_texture_1d")

        glFramebufferTexture1D(
            static_cast<uint32_t>(type), static_cast<uint32_t>(attachment),
            static_cast<uint32_t>(TextureType::TEXTURE_1D), texture_index, static_cast<int>(mipmap_level)
        );

    print_conditional_errors();
}
#endif

void Renderer::set_framebuffer_texture_2d(
    FramebufferAttachment attachment, uint32_t texture_index, uint32_t mipmap_level, TextureType textureType,
    FramebufferType type
)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    TracyGpuZone("Renderer::set_framebuffer_texture_2d")

        glFramebufferTexture2D(
            static_cast<uint32_t>(type), static_cast<uint32_t>(attachment), static_cast<uint32_t>(textureType),
            texture_index, static_cast<int>(mipmap_level)
        );

    print_conditional_errors();
}

#if !defined(USE_OPENGL_ES)
void Renderer::set_framebuffer_texture_3d(
    FramebufferAttachment attachment, uint32_t texture_index, uint32_t mipmap_level, uint32_t layer,
    FramebufferType type
)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    TracyGpuZone("Renderer::set_framebuffer_texture_3d")

        glFramebufferTexture3D(
            static_cast<uint32_t>(type), static_cast<uint32_t>(attachment),
            static_cast<uint32_t>(TextureType::TEXTURE_3D), texture_index, static_cast<int>(mipmap_level),
            static_cast<int>(layer)
        );

    print_conditional_errors();
}
#endif

void Renderer::recover_framebuffer_attachment_parameter(
    FramebufferAttachment attachment, FramebufferAttachmentParam param, int* values, FramebufferType type
)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    glGetFramebufferAttachmentParameteriv(
        static_cast<uint32_t>(type), static_cast<uint32_t>(attachment), static_cast<uint32_t>(param), values
    );

    print_conditional_errors();
}

void Renderer::set_read_buffer(ReadBuffer buffer)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    glReadBuffer(static_cast<uint32_t>(buffer));

    print_conditional_errors();
}

void Renderer::set_draw_buffers(uint32_t count, DrawBuffer const* buffers)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    glDrawBuffers(static_cast<int>(count), reinterpret_cast<uint32_t const*>(buffers));

    print_conditional_errors();
}

void Renderer::blit_framebuffer(
    int read_min_x, int read_min_y, int read_max_x, int read_max_y, int write_min_x, int write_min_y, int write_max_x,
    int write_max_y, MaskType mask, BlitFilter filter
)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    TracyGpuZone("Renderer::blit_framebuffer")

        glBlitFramebuffer(
            read_min_x, read_min_y, read_max_x, read_max_y, write_min_x, write_min_y, write_max_x, write_max_y,
            static_cast<uint32_t>(mask), static_cast<uint32_t>(filter)
        );

    print_conditional_errors();
}

void Renderer::delete_framebuffers(uint32_t count, uint32_t* indices)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    glDeleteFramebuffers(static_cast<int>(count), indices);

    print_conditional_errors();
}

void Renderer::draw_arrays(PrimitiveType type, uint32_t first, uint32_t count)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    TracyGpuZone("Renderer::draw_arrays")

        glDrawArrays(static_cast<uint32_t>(type), static_cast<int>(first), static_cast<int>(count));

    print_conditional_errors();
}

void Renderer::draw_arrays_instanced(PrimitiveType type, uint32_t first, uint primitive_count, uint instance_count)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    TracyGpuZone("Renderer::draw_arrays_instanced")

        glDrawArraysInstanced(
            static_cast<uint32_t>(type), static_cast<int>(first), static_cast<int>(primitive_count),
            static_cast<int>(instance_count)
        );

    print_conditional_errors();
}

void Renderer::draw_elements(PrimitiveType type, uint32_t count, ElementDataType data_type, void const* indices)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    TracyGpuZone("Renderer::draw_elements")

        glDrawElements(static_cast<uint32_t>(type), static_cast<int>(count), static_cast<uint32_t>(data_type), indices);

    print_conditional_errors();
}

void Renderer::draw_elements_instanced(
    PrimitiveType type, uint32_t primitive_count, ElementDataType data_type, void const* indices,
    uint32_t instance_count
)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    TracyGpuZone("Renderer::draw_elements_instanced")

        glDrawElementsInstanced(
            static_cast<uint32_t>(type), static_cast<int>(primitive_count), static_cast<uint32_t>(data_type), indices,
            static_cast<int>(instance_count)
        );

    print_conditional_errors();
}

void Renderer::dispatch_compute(Vector3ui group_content)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");
#if !defined(USE_OPENGL_ES)
    Log::rt_assert(
        check_version(4, 3) || is_extension_supported("GL_ARB_compute_shader"),
        "Error: Launching a compute operation requires OpenGL 4.3+ or the 'GL_ARB_compute_shader' extension."
    );
#else
    Log::rt_assert(
        check_version(3, 1) || is_extension_supported("GL_ARB_compute_shader"),
        "Error: Launching a compute operation requires OpenGL ES 3.1+ or the 'GL_ARB_compute_shader' extension."
    );
#endif

    TracyGpuZone("Renderer::dispatch_compute")

        glDispatchCompute(group_content.x, group_content.y, group_content.z);

    print_conditional_errors();
}

void Renderer::set_memory_barrier(BarrierType type)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");
#if !defined(USE_OPENGL_ES)
    Log::rt_assert(check_version(4, 2), "Error: Setting a memory barrier requires OpenGL 4.2+.");
#else
    Log::rt_assert(check_version(3, 1), "Error: Setting a memory barrier requires OpenGL ES 3.1+.");
#endif

    TracyGpuZone("Renderer::set_memory_barrier")

        glMemoryBarrier(static_cast<uint32_t>(type));

    print_conditional_errors();
}

void Renderer::set_memory_barrier_by_region(RegionBarrierType type)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");
#if !defined(USE_OPENGL_ES)
    Log::rt_assert(check_version(4, 5), "Error: Setting a memory barrier by region requires OpenGL 4.5+.");
#else
    Log::rt_assert(check_version(3, 1), "Error: Setting a memory barrier by region requires OpenGL ES 3.1+.");
#endif

    TracyGpuZone("Renderer::set_memory_barrier_by_region")

        glMemoryBarrierByRegion(static_cast<uint32_t>(type));

    print_conditional_errors();
}

void Renderer::generate_queries(uint32_t count, uint32_t* indices)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    glGenQueries(static_cast<int>(count), indices);

    print_conditional_errors();
}

void Renderer::begin_query(QueryType type, uint32_t index)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    glBeginQuery(static_cast<uint32_t>(type), index);

    print_conditional_errors();
}

void Renderer::end_query(QueryType type)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    glEndQuery(static_cast<uint32_t>(type));

    print_conditional_errors();
}

#if !defined(USE_OPENGL_ES)
void Renderer::recover_query_result(uint32_t index, int64_t& result)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    glGetQueryObjecti64v(index, GL_QUERY_RESULT, &result);

    print_conditional_errors();
}

void Renderer::recover_query_result(uint32_t index, uint64_t& result)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    glGetQueryObjectui64v(index, GL_QUERY_RESULT, &result);

    print_conditional_errors();
}
#endif

void Renderer::delete_queries(uint32_t count, uint32_t* indices)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");

    glDeleteQueries(static_cast<int>(count), indices);

    print_conditional_errors();
}

#if !defined(USE_OPENGL_ES)
void Renderer::set_label(RenderObjectType type, uint object_index, const char* label)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");
    Log::rt_assert(check_version(4, 3), "Error: Setting an object label requires OpenGL 4.3+.");

    glObjectLabel(static_cast<uint32_t>(type), object_index, -1, label);

    print_conditional_errors();
}

std::string Renderer::recover_label(RenderObjectType type, uint32_t object_index)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");
    Log::rt_assert(check_version(4, 3), "Error: Recovering an object label requires OpenGL 4.3+.");

    int label_length{};
    std::array<char, 256> label_name{};

    glGetObjectLabel(
        static_cast<uint32_t>(type), object_index, static_cast<int>(label_name.size()), &label_length, label_name.data()
    );

    std::string label;
    label.resize(static_cast<size_t>(label_length));
    std::copy(label_name.cbegin(), label_name.cbegin() + label_length, label.begin());

    print_conditional_errors();

    return label;
}

void Renderer::push_debug_group(std::string const& name)
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");
    Log::rt_assert(check_version(4, 3), "Error: Pushing a debug group requires OpenGL 4.3+.");

    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, static_cast<int>(name.size()), name.c_str());

    print_conditional_errors();
}

void Renderer::pop_debug_group()
{
    Log::rt_assert(is_initialized(), "Error: The Renderer must be initialized before calling its functions.");
    Log::rt_assert(check_version(4, 3), "Error: Popping a debug group requires OpenGL 4.3+.");

    glPopDebugGroup();

    print_conditional_errors();
}
#endif

ErrorCodes Renderer::recover_errors()
{
    static constexpr auto recover_error_code_index = [](ErrorCode code) constexpr -> uint8_t {
        return static_cast<uint8_t>(static_cast<uint32_t>(code) - static_cast<uint32_t>(ErrorCode::INVALID_ENUM));
    };

    ErrorCodes error_codes;

    while (true) {
        uint32_t const error_code = glGetError();

        if (error_code == GL_NO_ERROR) {
            break;
        }

        uint8_t const error_code_index = recover_error_code_index(static_cast<ErrorCode>(error_code));

        // An error code cannot be returned twice in a row; if it is, the error checking should be stopped
        if (error_codes.codes[error_code_index]) {
            break;
        }

        error_codes.codes[error_code_index] = true;
    }

    return error_codes;
}

void Renderer::print_errors()
{
    ErrorCodes const error_codes = recover_errors();

    if (error_codes.empty()) {
        return;
    }

    for (uint8_t error_index = 0; error_index < static_cast<uint8_t>(error_codes.codes.size()); ++error_index) {
        if (error_codes.codes[error_index]) {
            uint32_t const error_value = error_index + static_cast<uint32_t>(ErrorCode::INVALID_ENUM);
            Log::verror("[OpenGL] {} (code {})", recover_gl_error_str(error_value), error_value);
        }
    }
}

void Renderer::recover_default_framebuffer_color_format()
{
    struct ColorInfo {
        int red_bit_count{};
        int green_bit_count{};
        int blue_bit_count{};
        int alpha_bit_count{};
        int comp_type{};
        int encoding{};
    };

    struct ColorFormat {
        ColorInfo color_info{};
        TextureInternalFormat format{};
        std::string_view format_str;
    };

    constexpr std::array<ColorFormat, 26> formats = {
        {{ColorInfo{8, 8, 8, 0, GL_UNSIGNED_NORMALIZED, GL_LINEAR}, TextureInternalFormat::RGB8, "RGB8"},
         {ColorInfo{8, 8, 8, 8, GL_UNSIGNED_NORMALIZED, GL_LINEAR}, TextureInternalFormat::RGBA8, "RGBA8"},
         {ColorInfo{8, 8, 8, 0, GL_UNSIGNED_NORMALIZED, GL_SRGB}, TextureInternalFormat::SRGB8, "SRGB8"},
         {ColorInfo{8, 8, 8, 8, GL_UNSIGNED_NORMALIZED, GL_SRGB}, TextureInternalFormat::SRGBA8, "SRGBA8"},
         {ColorInfo{8, 8, 8, 0, GL_INT, GL_LINEAR}, TextureInternalFormat::RGB8I, "RGB8I"},
         {ColorInfo{8, 8, 8, 8, GL_INT, GL_LINEAR}, TextureInternalFormat::RGBA8I, "RGBA8I"},
         {ColorInfo{8, 8, 8, 0, GL_UNSIGNED_INT, GL_LINEAR}, TextureInternalFormat::RGB8UI, "RGB8UI"},
         {ColorInfo{8, 8, 8, 8, GL_UNSIGNED_INT, GL_LINEAR}, TextureInternalFormat::RGBA8UI, "RGBA8UI"},
         {ColorInfo{8, 8, 8, 0, GL_SIGNED_NORMALIZED, GL_LINEAR}, TextureInternalFormat::RGB8_SNORM, "RGB8_SNORM"},
         {ColorInfo{8, 8, 8, 8, GL_SIGNED_NORMALIZED, GL_LINEAR}, TextureInternalFormat::RGBA8_SNORM, "RGBA8_SNORM"},
         {ColorInfo{16, 16, 16, 16, GL_UNSIGNED_NORMALIZED, GL_LINEAR}, TextureInternalFormat::RGBA16, "RGBA16"},
         {ColorInfo{16, 16, 16, 0, GL_INT, GL_LINEAR}, TextureInternalFormat::RGB16I, "RGB16I"},
         {ColorInfo{16, 16, 16, 16, GL_INT, GL_LINEAR}, TextureInternalFormat::RGBA16I, "RGBA16I"},
         {ColorInfo{16, 16, 16, 0, GL_UNSIGNED_INT, GL_LINEAR}, TextureInternalFormat::RGB16UI, "RGB16UI"},
         {ColorInfo{16, 16, 16, 16, GL_UNSIGNED_INT, GL_LINEAR}, TextureInternalFormat::RGBA16UI, "RGBA16UI"},
         {ColorInfo{16, 16, 16, 0, GL_FLOAT, GL_LINEAR}, TextureInternalFormat::RGB16F, "RGB16F"},
         {ColorInfo{16, 16, 16, 16, GL_FLOAT, GL_LINEAR}, TextureInternalFormat::RGBA16F, "RGBA16F"},
         {ColorInfo{32, 32, 32, 0, GL_INT, GL_LINEAR}, TextureInternalFormat::RGB32I, "RGB32I"},
         {ColorInfo{32, 32, 32, 32, GL_INT, GL_LINEAR}, TextureInternalFormat::RGBA32I, "RGBA32I"},
         {ColorInfo{32, 32, 32, 0, GL_UNSIGNED_INT, GL_LINEAR}, TextureInternalFormat::RGB32UI, "RGB32UI"},
         {ColorInfo{32, 32, 32, 32, GL_UNSIGNED_INT, GL_LINEAR}, TextureInternalFormat::RGBA32UI, "RGBA32UI"},
         {ColorInfo{32, 32, 32, 0, GL_FLOAT, GL_LINEAR}, TextureInternalFormat::RGB32F, "RGB32F"},
         {ColorInfo{32, 32, 32, 32, GL_FLOAT, GL_LINEAR}, TextureInternalFormat::RGBA32F, "RGBA32F"},
         {ColorInfo{10, 10, 10, 2, GL_UNSIGNED_NORMALIZED, GL_LINEAR}, TextureInternalFormat::RGB10_A2, "RGB10_A2"},
         {ColorInfo{10, 10, 10, 2, GL_UNSIGNED_INT, GL_LINEAR}, TextureInternalFormat::RGB10_A2UI, "RGB10_A2UI"},
         {ColorInfo{11, 11, 10, 0, GL_FLOAT, GL_LINEAR}, TextureInternalFormat::R11F_G11F_B10F, "R11F_G11F_B10F"}}
    };

#if defined(USE_WEBGL)
    // WebGL requires getting a color attachment for the default framebuffer
    // See:
    // https://developer.mozilla.org/en-US/docs/Web/API/WebGLRenderingContext/getFramebufferAttachmentParameter#attachment
    constexpr FramebufferAttachment attachment = FramebufferAttachment::COLOR0;
#elif defined(USE_OPENGL_ES)
    constexpr FramebufferAttachment attachment = FramebufferAttachment::DEFAULT_BACK;
#else
    constexpr FramebufferAttachment attachment = FramebufferAttachment::DEFAULT_BACK_LEFT;
#endif
    ColorInfo color_info;
    Renderer::recover_framebuffer_attachment_parameter(
        attachment, FramebufferAttachmentParam::RED_SIZE, &color_info.red_bit_count
    );
    Renderer::recover_framebuffer_attachment_parameter(
        attachment, FramebufferAttachmentParam::GREEN_SIZE, &color_info.green_bit_count
    );
    Renderer::recover_framebuffer_attachment_parameter(
        attachment, FramebufferAttachmentParam::BLUE_SIZE, &color_info.blue_bit_count
    );
    Renderer::recover_framebuffer_attachment_parameter(
        attachment, FramebufferAttachmentParam::ALPHA_SIZE, &color_info.alpha_bit_count
    );
    Renderer::recover_framebuffer_attachment_parameter(
        attachment, FramebufferAttachmentParam::COMPONENT_TYPE, &color_info.comp_type
    );
    Renderer::recover_framebuffer_attachment_parameter(
        attachment, FramebufferAttachmentParam::COLOR_ENCODING, &color_info.encoding
    );

    auto const color_format_iter =
        std::find_if(formats.cbegin(), formats.cend(), [&color_info](ColorFormat const& format) {
            return (
                color_info.red_bit_count == format.color_info.red_bit_count &&
                color_info.green_bit_count == format.color_info.green_bit_count &&
                color_info.blue_bit_count == format.color_info.blue_bit_count &&
                color_info.alpha_bit_count == format.color_info.alpha_bit_count &&
                color_info.comp_type == format.color_info.comp_type && color_info.encoding == format.color_info.encoding
            );
        });

    if (color_format_iter == formats.cend()) {
        Log::verror(
            "[Renderer] Unknown default framebuffer color bits combination (red {}, green {}, blue {}, alpha {}, "
            "component type {}, encoding {})",
            color_info.red_bit_count, color_info.green_bit_count, color_info.blue_bit_count, color_info.alpha_bit_count,
            color_info.comp_type, color_info.encoding
        );
        return;
    }

    default_framebuffer_color = color_format_iter->format;

    Log::vdebug(
        "[Renderer] Found default framebuffer color format (value {}; red {}, green {}, blue {}, alpha {}, "
        "component type {}, encoding {})",
        color_format_iter->format_str, color_info.red_bit_count, color_info.green_bit_count, color_info.blue_bit_count,
        color_info.alpha_bit_count, color_info.comp_type, color_info.encoding
    );
}

void Renderer::recover_default_framebuffer_depth_format()
{
    struct DepthInfo {
        int depth_bit_count{};
        int stencil_bit_count{};
        int comp_type{};
    };

    struct DepthFormat {
        DepthInfo depth_info{};
        TextureInternalFormat format{};
        std::string_view format_str;
    };

    constexpr std::array<DepthFormat, 6> formats = {
        {{DepthInfo{16, 0, GL_UNSIGNED_NORMALIZED}, TextureInternalFormat::DEPTH16, "DEPTH16"},
         {DepthInfo{24, 0, GL_UNSIGNED_NORMALIZED}, TextureInternalFormat::DEPTH24, "DEPTH24"},
         {DepthInfo{24, 8, GL_UNSIGNED_NORMALIZED}, TextureInternalFormat::DEPTH24_STENCIL8, "DEPTH24_STENCIL8"},
         {DepthInfo{32, 0, GL_UNSIGNED_NORMALIZED}, TextureInternalFormat::DEPTH32, "DEPTH32"},
         {DepthInfo{32, 0, GL_FLOAT}, TextureInternalFormat::DEPTH32F, "DEPTH32F"},
         {DepthInfo{32, 8, GL_FLOAT}, TextureInternalFormat::DEPTH32F_STENCIL8, "DEPTH32F_STENCIL8"}}
    };

#if defined(USE_WEBGL)
    // WebGL requires getting explicitly the depth attachment for the default framebuffer
    // See:
    // https://developer.mozilla.org/en-US/docs/Web/API/WebGLRenderingContext/getFramebufferAttachmentParameter#attachment
    constexpr FramebufferAttachment attachment = FramebufferAttachment::DEPTH;
#else
    constexpr FramebufferAttachment attachment = FramebufferAttachment::DEFAULT_DEPTH;
#endif
    DepthInfo depth_info{};
    Renderer::recover_framebuffer_attachment_parameter(
        attachment, FramebufferAttachmentParam::DEPTH_SIZE, &depth_info.depth_bit_count
    );
    Renderer::recover_framebuffer_attachment_parameter(
        attachment, FramebufferAttachmentParam::STENCIL_SIZE, &depth_info.stencil_bit_count
    );
    Renderer::recover_framebuffer_attachment_parameter(
        attachment, FramebufferAttachmentParam::COMPONENT_TYPE, &depth_info.comp_type
    );

    auto const depth_format_iter =
        std::find_if(formats.cbegin(), formats.cend(), [&depth_info](DepthFormat const& format) {
            return (
                depth_info.depth_bit_count == format.depth_info.depth_bit_count &&
                depth_info.stencil_bit_count == format.depth_info.stencil_bit_count &&
                depth_info.comp_type == format.depth_info.comp_type
            );
        });

    if (depth_format_iter == formats.cend()) {
        Log::verror(
            "[Renderer] Unknown default framebuffer depth bits combination (depth {}, stencil {}, component_type {})",
            depth_info.depth_bit_count, depth_info.stencil_bit_count, depth_info.comp_type
        );
        return;
    }

    default_framebuffer_depth = depth_format_iter->format;

    Log::vdebug(
        "[Renderer] Found default framebuffer depth format (value {}; depth {}, stencil {}, component_type {})",
        depth_format_iter->format_str, depth_info.depth_bit_count, depth_info.stencil_bit_count, depth_info.comp_type
    );
}

}
