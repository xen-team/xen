#pragma once

#include <system.hpp>

#include <xr/xr_context.hpp>
#include <xr/xr_session.hpp>

struct XrEventDataSessionStateChanged;
struct XrViewConfigurationView;

namespace xen {

class XrSystem final : public System {
    friend class RenderSystem;

public:
    explicit XrSystem(std::string const& app_name);

    ~XrSystem() override = default;

    Vector2ui get_optimal_view_size() const { return optimal_view_size; }

    bool update(FrameTimeInfo const&) override;

private:
    XrContext context;
    XrSession session;

    std::vector<uint> view_config_types;
    uint view_config_type{0};
    std::vector<XrViewConfigurationView> view_config_views;
    Vector2ui optimal_view_size;

    std::vector<uint> environment_blend_modes;
    uint environment_blend_mode{0};

private:
    void recover_view_configurations();

    void recover_environment_blend_modes();

    void initialize_session();

    bool render_frame(ViewRenderFunc const& view_render_func) const;

    bool process_session_state_changed(XrEventDataSessionStateChanged const& session_state_changed);
};
}