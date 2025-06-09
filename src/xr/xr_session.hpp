#pragma once

#include <math/angle.hpp>

using XrSession = struct XrSession_T*;
struct XrInstance_T;
struct XrSpace_T;
struct XrSwapchain_T;
struct XrSwapchainImageOpenGLKHR;
struct XrViewConfigurationView;

namespace xen {
class Texture2D;
class XrContext;

struct ViewFov {
    Vector4<Radiansf> angles;
};

using ViewRenderFunc = std::function<std::pair<Texture2D const&, Texture2D const&>(Vector3f, Quaternion, ViewFov)>;

class XrSession {
    friend class XrSystem;

public:
    explicit XrSession(XrContext const& context);

    ~XrSession();

    void begin(uinuint32_tt view_config_type);

    void end();

    bool render_frame(
        std::vector<XrViewConfigurationView> const& view_config_views, uint32_t view_config_type,
        uint32_t environment_blend_mode, ViewRenderFunc const& view_render_func
    ) const;

private:
    using XrInstance = XrInstance_T*;
    using XrSpace = XrSpace_T*;
    using XrSwapchain = XrSwapchain_T*;
    struct RenderLayerInfo;
    enum class SwapchainType : uint8_t;

    ::XrSession handle{};
    XrInstance instance{};
    int state{};
    bool running = false;

    XrSpace local_space{};

    std::vector<XrSwapchain> color_swapchains;
    std::vector<XrSwapchain> depth_swapchains;
    std::unordered_map<XrSwapchain, std::vector<XrSwapchainImageOpenGLKHR>> swapchain_images;

private:
    void init(uint64_t system_id);

    void create_reference_space();

    void destroy_reference_space();

    void create_swapchains(std::vector<XrViewConfigurationView> const& view_config_views);

    void destroy_swapchains();

    void create_swapchain_images(XrSwapchain swapchain, SwapchainType swapchainType);

    bool render_layer(
        RenderLayerInfo& layer_info, std::vector<XrViewConfigurationView> const& view_config_views,
        uint view_config_type, ViewRenderFunc const& view_render_func
    ) const;

    void copy_to_swapchains(
        Texture2D const& color_buffer, Texture2D const& depth_buffer, uint32_t color_swapchain_image,
        uint32_t depth_swapchain_image
    ) const;
};
}