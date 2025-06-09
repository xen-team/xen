#include "xr_session.hpp"

#include <render/renderer.hpp>
#include <render/render_pass.hpp>

#include <xr/xr_context.hpp>

#if defined(XR_OS_WINDOWS)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Unknwn.h>
#endif

#include <GL/glew.h>
#if defined(XR_OS_WINDOWS)
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#elif defined(XR_OS_LINUX)
#define GLFW_EXPOSE_NATIVE_X11
#define GLFW_EXPOSE_NATIVE_GLX
#endif
#include "GLFW/glfw3.h"
#include "GLFW/glfw3native.h"

#include "openxr/openxr.h"
#include "openxr/openxr_platform.h"

#include <tracy/Tracy.hpp>
#include <tracy/TracyOpenGL.hpp>

namespace xen {
namespace {
constexpr std::string_view swapchain_copy_source = R"(
      in vec2 fragTexcoords;
    
      uniform sampler2D uniFinalColorBuffer;
      uniform sampler2D uniFinalDepthBuffer;
    
      layout(location = 0) out vec4 fragColor;
    
      void main() {
        fragColor     = texture(uniFinalColorBuffer, fragTexcoords).rgba;
        // Gamma uncorrection, as the swapchain seems to apply it itself
        fragColor.rgb = pow(fragColor.rgb, vec3(2.2));
        gl_FragDepth  = texture(uniFinalDepthBuffer, fragTexcoords).r;
      }
    )";

char const* get_result_str(XrInstance instance, XrResult result)
{
    static std::array<char, XR_MAX_RESULT_STRING_SIZE> error_str{};
    xrResultToString(instance, result, error_str.data());
    return error_str.data();
}

std::string get_error_str(std::string const& error_msg, XrResult result, XrInstance instance)
{
    return "[XrSession] " + error_msg + ": " + get_result_str(instance, result) + " (" + std::to_string(result) + ')';
}

void check_log(XrResult result, std::string const& error_msg, XrInstance instance)
{
    if (XR_SUCCEEDED(result)) {
        return;
    }

    Log::error(get_error_str(error_msg, result, instance));
}

void check_throw(XrResult result, std::string const& error_msg, XrInstance instance)
{
    if (XR_SUCCEEDED(result)) {
        return;
    }

    throw std::runtime_error(get_error_str(error_msg, result, instance));
}

#if defined(XR_USE_PLATFORM_WIN32)
using GraphicsBinding = XrGraphicsBindingOpenGLWin32KHR;
#elif defined(XR_USE_PLATFORM_XLIB)
using GraphicsBinding = XrGraphicsBindingOpenGLXlibKHR;
#endif

GraphicsBinding get_graphics_binding()
{
    GLFWwindow* window = glfwGetCurrentContext();
    GraphicsBinding graphics_binding{};

#if defined(XR_USE_PLATFORM_WIN32)
    graphics_binding.type = XR_TYPE_GRAPHICS_BINDING_OPENGL_WIN32_KHR;
    graphics_binding.hDC = GetDC(glfwGetWin32Window(window));
    graphics_binding.hGLRC = glfwGetWGLContext(window);
#elif defined(XR_USE_PLATFORM_XLIB)
    Display* x11_display = glfwGetX11Display();

    // TODO: some fields can't be directly filled with what GLFW exposes; see https://github.com/glfw/glfw/issues/2129

    // TODO: unless there's a way to easily recover the FBConfig from a GLXContext (related GLFW PR:
    // https://github.com/glfw/glfw/pull/1925), it has to be done
    //  manually; see
    //  https://github.com/KhronosGroup/OpenXR-SDK-Source/blob/dfe3ad67f11ab71a64b2c75d6b7a97034b9766fd/src/common/gfxwrapper_opengl.c#L1016-L1077
    GLXFBConfig fb_config{};
    graphics_binding.type = XR_TYPE_GRAPHICS_BINDING_OPENGL_XLIB_KHR;
    graphics_binding.xDisplay = x11_display;
    graphics_binding.visualid = static_cast<uint32_t>(glXGetVisualFromFBConfig(x11_display, fb_config)->visualid);
    graphics_binding.glxFBConfig = fb_config;
    graphics_binding.glxDrawable = glXGetCurrentDrawable();
    graphics_binding.glxContext = glfwGetGLXContext(window);
#endif

    return graphics_binding;
}

int64_t select_color_swapchain_format(std::vector<int64_t> const& formats)
{
    constexpr std::array<int64_t, 4> supported_color_swapchain_formats = {
        GL_RGB10_A2, GL_RGBA16F,
        // The following values should only be used as a fallback, as they are linear color formats without enough bits
        // for color depth, thus leading to banding
        GL_RGBA8, GL_RGBA8_SNORM
    };

    auto const format_it = std::find_first_of(
        formats.cbegin(), formats.cend(), supported_color_swapchain_formats.cbegin(),
        supported_color_swapchain_formats.cend()
    );

    if (format_it == formats.cend()) {
        return 0;
    }

    return *format_it;
}

int64_t select_depth_swapchain_format(std::vector<int64_t> const& formats)
{
    constexpr std::array<int64_t, 4> supported_depth_swapchain_formats = {
        GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT32, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT16
    };

    auto const format_it = std::find_first_of(
        formats.cbegin(), formats.cend(), supported_depth_swapchain_formats.cbegin(),
        supported_depth_swapchain_formats.cend()
    );

    if (format_it == formats.cend()) {
        return 0;
    }

    return *format_it;
}
}
struct XrSession::RenderLayerInfo {
    XrTime predicted_display_time{};
    std::vector<XrCompositionLayerBaseHeader*> layers;
    XrCompositionLayerProjection layer_projection{XR_TYPE_COMPOSITION_LAYER_PROJECTION};
    std::vector<XrCompositionLayerProjectionView> layer_projection_views;
};

enum class XrSession::SwapchainType : uint8_t { COLOR, DEPTH };

XrSession::XrSession(XrContext const& context) : instance{context.instance}
{
    if (instance == XR_NULL_HANDLE) {
        throw std::runtime_error("[XrSession] The XR instance must be valid");
    }
}

void XrSession::begin(uint32_t view_config_type)
{
    Log::debug("[XrSession] Beginning session...");

    XrSessionBeginInfo session_begin_info{};
    session_begin_info.type = XR_TYPE_SESSION_BEGIN_INFO;
    session_begin_info.primaryViewConfigurationType = static_cast<XrViewConfigurationType>(view_config_type);
    check_log(xrBeginSession(handle, &session_begin_info), "Failed to begin session", instance);

    Log::debug("[XrSession] Began session");
}

void XrSession::end()
{
    Log::debug("[XrSession] Ending session...");
    check_log(xrEndSession(handle), "Failed to end session", instance);
    Log::debug("[XrSession] Ended session");
}

bool XrSession::render_frame(
    std::vector<XrViewConfigurationView> const& view_config_views, uint32_t view_config_type,
    uint32_t environment_blend_mode, ViewRenderFunc const& view_render_func
) const
{
    ZoneScopedN("XrSession::render_frame");

    if (!running) {
        return false;
    }

    XrFrameWaitInfo frame_wait_info{};
    frame_wait_info.type = XR_TYPE_FRAME_WAIT_INFO;
    XrFrameState frame_state{};
    frame_state.type = XR_TYPE_FRAME_STATE;
    check_log(xrWaitFrame(handle, &frame_wait_info, &frame_state), "Failed to wait for the XR frame", instance);

    XrFrameBeginInfo frame_begin_info{};
    frame_begin_info.type = XR_TYPE_FRAME_BEGIN_INFO;
    check_log(xrBeginFrame(handle, &frame_begin_info), "Failed to begin the XR frame", instance);

    RenderLayerInfo render_layer_info;
    // TODO: either the application should use this display time, or the application's global & delta times should be
    // used here somehow
    //  See:
    //  - https://registry.khronos.org/OpenXR/specs/1.0/man/html/xrWaitFrame.html#_description
    //  - https://registry.khronos.org/OpenXR/specs/1.0/man/html/xrEndFrame.html#_description
    //  - https://registry.khronos.org/OpenXR/specs/1.0/man/html/XrTime.html
    //  - https://registry.khronos.org/OpenXR/specs/1.0/man/html/XR_KHR_convert_timespec_time.html
    render_layer_info.predicted_display_time = frame_state.predictedDisplayTime;

    bool const is_session_active =
        (state == XR_SESSION_STATE_SYNCHRONIZED || state == XR_SESSION_STATE_VISIBLE ||
         state == XR_SESSION_STATE_FOCUSED);
    if (is_session_active && frame_state.shouldRender &&
        render_layer(render_layer_info, view_config_views, view_config_type, view_render_func))
        render_layer_info.layers.emplace_back(
            reinterpret_cast<XrCompositionLayerBaseHeader*>(&render_layer_info.layer_projection)
        );

    {
        ZoneNamedN(endFrameZone, "xrEndFrame", true);
        TracyGpuZone("xrEndFrame")

            XrFrameEndInfo frame_end_info{};
        frame_end_info.type = XR_TYPE_FRAME_END_INFO;
        frame_end_info.displayTime = frame_state.predictedDisplayTime;
        frame_end_info.environmentBlendMode = static_cast<XrEnvironmentBlendMode>(environment_blend_mode);
        frame_end_info.layerCount = static_cast<uint32_t>(render_layer_info.layers.size());
        frame_end_info.layers = render_layer_info.layers.data();
        check_log(xrEndFrame(handle, &frame_end_info), "Failed to end the XR frame", instance);
    }

    return !render_layer_info.layers.empty();
}

XrSession::~XrSession()
{
    if (handle == XR_NULL_HANDLE) {
        return;
    }

    Log::debug("[XrSession] Destroying session...");

    destroy_swapchains();
    destroy_reference_space();
    check_log(xrDestroySession(handle), "Failed to destroy session", instance);

    Log::debug("[XrSession] Destroyed session");
}

void XrSession::init(uint64_t system_id)
{
    ZoneScopedN("XrSession::initialize");

    Log::debug("[XrSession] Initializing...");

    if (!Renderer::is_initialized()) {
        throw std::runtime_error("[XrSession] The renderer must be initialized");
    }

    PFN_xrGetOpenGLGraphicsRequirementsKHR xrGetOpenGLGraphicsRequirementsKHR{};
    check_log(
        xrGetInstanceProcAddr(
            instance, "xrGetOpenGLGraphicsRequirementsKHR",
            reinterpret_cast<PFN_xrVoidFunction*>(&xrGetOpenGLGraphicsRequirementsKHR)
        ),
        "Failed to get OpenGL graphics requirements get function", instance
    );

    XrGraphicsRequirementsOpenGLKHR graphics_requirements{};
    graphics_requirements.type = XR_TYPE_GRAPHICS_REQUIREMENTS_OPENGL_KHR;
    check_log(
        xrGetOpenGLGraphicsRequirementsKHR(instance, system_id, &graphics_requirements),
        "Failed to get graphics requirements for OpenGL", instance
    );

    XrVersion const graphics_api_version =
        XR_MAKE_VERSION(Renderer::get_major_version(), Renderer::get_minor_version(), 0);
    if (graphics_requirements.minApiVersionSupported > graphics_api_version) {
        uint16_t const required_major_version = XR_VERSION_MAJOR(graphics_requirements.minApiVersionSupported);
        uint16_t const required_minor_version = XR_VERSION_MINOR(graphics_requirements.minApiVersionSupported);
        throw std::runtime_error(
            "[XrSession] The current OpenGL version " + std::to_string(Renderer::get_major_version()) + '.' +
            std::to_string(Renderer::get_minor_version()) + " does not meet the minimum required version " +
            std::to_string(required_major_version) + '.' + std::to_string(required_minor_version) + " for OpenXR"
        );
    }

    GraphicsBinding const graphics_binding = get_graphics_binding();
    XrSessionCreateInfo session_create_info{};
    session_create_info.type = XR_TYPE_SESSION_CREATE_INFO;
    session_create_info.next = &graphics_binding;
    session_create_info.createFlags = 0;
    session_create_info.systemId = system_id;
    check_throw(xrCreateSession(instance, &session_create_info, &handle), "Failed to create session", instance);

    create_reference_space();

    Log::debug("[XrSession] Initialized");
}

void XrSession::create_reference_space()
{
    ZoneScopedN("XrSession::create_reference_space");

    Log::debug("[XrSession] Creating reference space...");

    XrReferenceSpaceCreateInfo reference_space_create_info{};
    reference_space_create_info.type = XR_TYPE_REFERENCE_SPACE_CREATE_INFO;
    reference_space_create_info.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_LOCAL;
    reference_space_create_info.poseInReferenceSpace = {XrQuaternionf{0.f, 0.f, 0.f, 1.f}, XrVector3f{0.f, 0.f, 0.f}};
    check_log(
        xrCreateReferenceSpace(handle, &reference_space_create_info, &local_space), "Failed to create reference space",
        instance
    );

    Log::debug("[XrSession] Created reference space");
}

void XrSession::destroy_reference_space()
{
    Log::debug("[XrSession] Destroying reference space...");
    check_log(xrDestroySpace(local_space), "Failed to destroy space", instance);
    Log::debug("[XrSession] Destroyed reference space");
}

void XrSession::create_swapchains(std::vector<XrViewConfigurationView> const& view_config_views)
{
    ZoneScopedN("XrSession::create_swapchains");

    Log::debug("[XrSession] Creating swapchains...");

    if (handle == nullptr) {
        throw std::runtime_error("[XrSession] The session has not been initialized");
    }

    uint32_t format_count{};
    check_log(
        xrEnumerateSwapchainFormats(handle, 0, &format_count, nullptr), "Failed to get swapchain format count", instance
    );

    std::vector<int64_t> formats(format_count);
    check_log(
        xrEnumerateSwapchainFormats(handle, format_count, &format_count, formats.data()),
        "Failed to enumerate swapchain formats", instance
    );

    if (select_depth_swapchain_format(formats) == 0) {
        Log::error("[XrSession] Failed to find a supported depth swapchain format");
    }

    color_swapchains.resize(view_config_views.size());
    depth_swapchains.resize(view_config_views.size());
    swapchain_images.reserve(view_config_views.size());

    for (size_t view_index = 0; view_index < view_config_views.size(); ++view_index) {
        XrSwapchain& color_swapchain = color_swapchains[view_index];
        XrSwapchain& depth_swapchain = depth_swapchains[view_index];

        XrSwapchainCreateInfo swapchain_create_info{};
        swapchain_create_info.type = XR_TYPE_SWAPCHAIN_CREATE_INFO;

        swapchain_create_info.createFlags = 0;
        swapchain_create_info.usageFlags =
            XR_SWAPCHAIN_USAGE_SAMPLED_BIT | XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT; // Technically ignored with OpenGL
        swapchain_create_info.format = select_color_swapchain_format(formats);
        swapchain_create_info.sampleCount = view_config_views[view_index].recommendedSwapchainSampleCount;
        swapchain_create_info.width = view_config_views[view_index].recommendedImageRectWidth;
        swapchain_create_info.height = view_config_views[view_index].recommendedImageRectHeight;
        swapchain_create_info.faceCount = 1;
        swapchain_create_info.arraySize = 1;
        swapchain_create_info.mipCount = 1;
        check_log(
            xrCreateSwapchain(handle, &swapchain_create_info, &color_swapchain), "Failed to create color swapchain",
            instance
        );

        swapchain_create_info.createFlags = 0;
        swapchain_create_info.usageFlags =
            XR_SWAPCHAIN_USAGE_SAMPLED_BIT |
            XR_SWAPCHAIN_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT; // Technically ignored with OpenGL
        swapchain_create_info.format = select_depth_swapchain_format(formats);
        swapchain_create_info.sampleCount = view_config_views[view_index].recommendedSwapchainSampleCount;
        swapchain_create_info.width = view_config_views[view_index].recommendedImageRectWidth;
        swapchain_create_info.height = view_config_views[view_index].recommendedImageRectHeight;
        swapchain_create_info.faceCount = 1;
        swapchain_create_info.arraySize = 1;
        swapchain_create_info.mipCount = 1;
        check_log(
            xrCreateSwapchain(handle, &swapchain_create_info, &depth_swapchain), "Failed to create depth swapchain",
            instance
        );

        create_swapchain_images(color_swapchain, SwapchainType::COLOR);
        create_swapchain_images(depth_swapchain, SwapchainType::DEPTH);
    }

    Log::debug("[XrSession] Created swapchains");
}

void XrSession::destroy_swapchains()
{
    ZoneScopedN("XrSession::destroy_swapchains");

    Log::debug("[XrSession] Destroying swapchains...");

    for (size_t swapchain_index = 0; swapchain_index < color_swapchains.size(); ++swapchain_index) {
        check_log(xrDestroySwapchain(color_swapchains[swapchain_index]), "Failed to destroy color swapchain", instance);
        check_log(xrDestroySwapchain(depth_swapchains[swapchain_index]), "Failed to destroy depth swapchain", instance);
    }

    swapchain_images.clear();

    Log::debug("[XrSession] Destroyed swapchains");
}

void XrSession::create_swapchain_images(XrSwapchain swapchain, SwapchainType swapchainType)
{
    ZoneScopedN("XrSession::create_swapchain_images");

    std::string const type_str = (swapchainType == SwapchainType::DEPTH ? "depth" : "color");

    Log::debug("[XrSession] Creating " + type_str + " swapchain images...");

    uint32_t swapchain_image_count{};
    check_log(
        xrEnumerateSwapchainImages(swapchain, 0, &swapchain_image_count, nullptr),
        "Failed to get " + type_str + " swapchain image count", instance
    );

    std::vector<XrSwapchainImageOpenGLKHR>& images = swapchain_images[swapchain];
    images.resize(swapchain_image_count, {XR_TYPE_SWAPCHAIN_IMAGE_OPENGL_KHR});
    check_log(
        xrEnumerateSwapchainImages(
            swapchain, swapchain_image_count, &swapchain_image_count,
            reinterpret_cast<XrSwapchainImageBaseHeader*>(images.data())
        ),
        "Failed to enumerate " + type_str + " swapchain images", instance
    );

    Log::debug("[XrSession] Created " + type_str + " swapchain images");
}

bool XrSession::render_layer(
    RenderLayerInfo& layer_info, std::vector<XrViewConfigurationView> const& view_config_views,
    uint32_t view_config_type, ViewRenderFunc const& view_render_func
) const
{
    ZoneScopedN("XrSession::render_layer");

    std::vector<XrView> views(swapchain_images.size(), {XR_TYPE_VIEW});

    XrViewLocateInfo view_locate_info{};
    view_locate_info.type = XR_TYPE_VIEW_LOCATE_INFO;
    view_locate_info.viewConfigurationType = static_cast<XrViewConfigurationType>(view_config_type);
    view_locate_info.displayTime = layer_info.predicted_display_time;
    view_locate_info.space = local_space;

    XrViewState view_state{};
    view_state.type = XR_TYPE_VIEW_STATE;
    uint32_t view_count{};
    if (xrLocateViews(
            handle, &view_locate_info, &view_state, static_cast<uint32_t>(views.size()), &view_count, views.data()
        ) != XR_SUCCESS) {
        Log::error("[XrSession] Failed to locate views");
        return false;
    }

    // TODO: view state flags must be checked; see:
    // https://registry.khronos.org/OpenXR/specs/1.0/man/html/XrViewStateFlagBits.html#_description

    layer_info.layer_projection_views.resize(view_count, {XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW});

    for (uint32_t view_index = 0; view_index < view_count; ++view_index) {
        [[maybe_unused]] char const* eye_str =
            (view_count == 1 ? "Single view" : (view_index == 0 ? "Left eye" : "Right eye"));

        ZoneTransientN(cpuEyeZone, eye_str, true);

        XrSwapchain const color_swapchain = color_swapchains[view_index];
        XrSwapchain const depth_swapchain = depth_swapchains[view_index];

        XrSwapchainImageAcquireInfo acquire_info{};
        acquire_info.type = XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO;

        uint32_t color_image_index{};
        uint32_t depth_image_index{};
        check_log(
            xrAcquireSwapchainImage(color_swapchain, &acquire_info, &color_image_index),
            "Failed to acquire image from the color swapchain", instance
        );
        check_log(
            xrAcquireSwapchainImage(depth_swapchain, &acquire_info, &depth_image_index),
            "Failed to acquire image from the depth swapchain", instance
        );

        XrSwapchainImageWaitInfo wait_info{};
        wait_info.type = XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO;
        wait_info.timeout = XR_INFINITE_DURATION;
        check_log(
            xrWaitSwapchainImage(color_swapchain, &wait_info), "Failed to wait for image from the color swapchain",
            instance
        );
        check_log(
            xrWaitSwapchainImage(depth_swapchain, &wait_info), "Failed to wait for image from the depth swapchain",
            instance
        );

        uint32_t const width = view_config_views[view_index].recommendedImageRectWidth;
        uint32_t const height = view_config_views[view_index].recommendedImageRectHeight;

        XrView const& current_view = views[view_index];

        XrCompositionLayerProjectionView& layer_projection_view = layer_info.layer_projection_views[view_index];
        layer_projection_view.type = XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW;
        layer_projection_view.pose = current_view.pose;
        layer_projection_view.fov = current_view.fov;
        layer_projection_view.subImage.swapchain = color_swapchain;
        layer_projection_view.subImage.imageRect.offset = {0, 0};
        layer_projection_view.subImage.imageRect.extent = {static_cast<int32_t>(width), static_cast<int32_t>(height)};
        layer_projection_view.subImage.imageArrayIndex = 0; // Useful for multiview rendering

        TracyGpuZoneTransient(gpuEyeZone, eye_str, true)

#if defined(XEN_CONFIG_DEBUG)
            if (Renderer::check_version(4, 3))
        {
            Renderer::push_debug_group(eye_str);
        }
#endif

        const auto& [color_buffer, depth_buffer] = view_render_func(
            Vector3f(current_view.pose.position.x, current_view.pose.position.y, current_view.pose.position.z),
            Quaternion(
                current_view.pose.orientation.x, current_view.pose.orientation.y, current_view.pose.orientation.z,
                current_view.pose.orientation.w
            ),
            ViewFov{Vector4<Radiansf>{
                Radiansf(current_view.fov.angleRight), Radiansf(current_view.fov.angleLeft),
                Radiansf(current_view.fov.angleUp), Radiansf(current_view.fov.angleDown)
            }}
        );

        uint32_t const color_swapchain_image = swapchain_images.find(color_swapchain)->second[color_image_index].image;
        uint32_t const depth_swapchain_image = swapchain_images.find(depth_swapchain)->second[depth_image_index].image;
        copy_to_swapchains(color_buffer, depth_buffer, color_swapchain_image, depth_swapchain_image);

#if defined(XEN_CONFIG_DEBUG)
        if (Renderer::check_version(4, 3)) {
            Renderer::pop_debug_group();
        }
#endif

        XrSwapchainImageReleaseInfo release_info{};
        release_info.type = XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO;
        check_log(
            xrReleaseSwapchainImage(color_swapchain, &release_info),
            "Failed to release image back to the color swapchain", instance
        );
        check_log(
            xrReleaseSwapchainImage(depth_swapchain, &release_info),
            "Failed to release image back to the depth swapchain", instance
        );
    }

    layer_info.layer_projection.layerFlags =
        XR_COMPOSITION_LAYER_BLEND_TEXTURE_SOURCE_ALPHA_BIT |
        XR_COMPOSITION_LAYER_CORRECT_CHROMATIC_ABERRATION_BIT; // Should be deprecated and ignored
    layer_info.layer_projection.space = local_space;
    layer_info.layer_projection.viewCount = static_cast<uint32_t>(layer_info.layer_projection_views.size());
    layer_info.layer_projection.views = layer_info.layer_projection_views.data();

    return true;
}

void XrSession::copy_to_swapchains(
    Texture2D const& color_buffer, Texture2D const& depth_buffer, uint32_t color_swapchain_image,
    uint32_t depth_swapchain_image
) const
{
    // https://docs.gl/gl4/glCopyImageSubData *could* be a viable and more direct solution, but expects both textures to
    // have compatible internal formats
    //  (https://registry.khronos.org/OpenGL/specs/gl/glspec46.core.pdf#page=295), which we simply cannot have any
    //  guarantee of

    ZoneScopedN("XrSession::copy_to_swapchains");
    TracyGpuZone("XrSession::copy_to_swapchains")

        static RenderPass swapchain_copy_pass = []() {
            RenderPass copy_pass(FragmentShader::load_from_source(swapchain_copy_source), "Swapchain copy pass");

            RenderShaderProgram& copy_program = copy_pass.get_program();
            copy_program.set_attribute(0, "uniFinalColorBuffer");
            copy_program.set_attribute(1, "uniFinalDepthBuffer");

            constexpr DrawBuffer draw_buffer = DrawBuffer::COLOR_ATTACHMENT0;
            Renderer::bind_framebuffer(copy_pass.get_framebuffer().get_index(), FramebufferType::DRAW_FRAMEBUFFER);
            Renderer::set_draw_buffers(1, &draw_buffer);
            Renderer::bind_framebuffer(0);

            return copy_pass;
        }();

    swapchain_copy_pass.get_program().use();
    Renderer::activate_texture(0);
    color_buffer.bind();
    Renderer::activate_texture(1);
    depth_buffer.bind();

    Renderer::bind_framebuffer(swapchain_copy_pass.get_framebuffer().get_index(), FramebufferType::DRAW_FRAMEBUFFER);
    Renderer::set_framebuffer_texture_2d(FramebufferAttachment::COLOR0, color_swapchain_image, 0);
    Renderer::set_framebuffer_texture_2d(FramebufferAttachment::DEPTH, depth_swapchain_image, 0);
    Renderer::clear(MaskType::COLOR | MaskType::DEPTH | MaskType::STENCIL);

    Renderer::set_depth_function(DepthStencilFunction::ALWAYS);
    swapchain_copy_pass.execute();
    Renderer::set_depth_function(DepthStencilFunction::LESS);
}
}