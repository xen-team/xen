#include "xr_system.hpp"

#include "openxr/openxr.h"

namespace xen {
namespace {
char const* get_result_str(XrInstance instance, XrResult result)
{
    static std::array<char, XR_MAX_RESULT_STRING_SIZE> error_str{};
    xrResultToString(instance, result, error_str.data());
    return error_str.data();
}

std::string get_error_str(std::string const& error_msg, XrResult result, XrInstance instance)
{
    return "[XrSystem] " + error_msg + ": " + get_result_str(instance, result) + " (" + std::to_string(result) + ')';
}

void check_log(XrResult result, std::string const& error_msg, XrInstance instance)
{
    if (XR_SUCCEEDED(result)) {
        return;
    }

    Log::error(get_error_str(error_msg, result, instance));
}

bool poll_next_event(XrInstance instance, XrEventDataBuffer& event_data)
{
    event_data = {};
    event_data.type = XR_TYPE_EVENT_DATA_BUFFER;

    return (xrPollEvent(instance, &event_data) == XR_SUCCESS);
}

void process_event_data(XrEventDataEventsLost const& events_lost)
{
    Log::vinfo("[XrSystem] {} events lost", events_lost.lostEventCount);
}

void process_event_data(XrEventDataInstanceLossPending const& instance_loss_pending)
{
    // After the period of time specified by lossTime, the application can try recreating an instance again
    // See: https://registry.khronos.org/OpenXR/specs/1.0/man/html/XrEventDataInstanceLossPending.html#_description
    Log::info("[XrSystem] Instance loss pending at: ", instance_loss_pending.lossTime);
}

void process_event_data(XrEventDataInteractionProfileChanged const& interaction_profile_changed, ::XrSession session)
{
    Log::info(
        "[XrSystem] Interaction profile changed for ",
        std::string(interaction_profile_changed.session != session ? "unknown" : "current") + " session"
    );
}

void process_event_data(
    XrEventDataReferenceSpaceChangePending const& reference_space_change_pending, ::XrSession session
)
{
    Log::info(
        "[XrSystem] Reference space changed pending for ",
        std::string(reference_space_change_pending.session != session ? "unknown" : "current") + " session"
    );
}
}

XrSystem::XrSystem(std::string const& app_name) : context(app_name), session(context)
{
    recover_view_configurations();

    optimal_view_size.x = view_config_views.front().recommendedImageRectWidth;
    optimal_view_size.y = view_config_views.front().recommendedImageRectHeight;

    if (view_config_views.size() > 1) {
        for (XrViewConfigurationView const& view_config_view : view_config_views) {
            if (view_config_view.recommendedImageRectWidth != optimal_view_size.x ||
                view_config_view.recommendedImageRectHeight != optimal_view_size.y) {
                Log::warning("[XrSystem] The optimal configuration view size is not the same for all views; rendering "
                             "may be altered");
            }
        }
    }

    recover_environment_blend_modes();
}

bool XrSystem::update(FrameTimeInfo const&)
{
    XrEventDataBuffer event_data{};

    while (poll_next_event(context.instance, event_data)) {
        switch (event_data.type) {
        case XR_TYPE_EVENT_DATA_EVENTS_LOST:
            process_event_data(*reinterpret_cast<XrEventDataEventsLost*>(&event_data));
            break;

        case XR_TYPE_EVENT_DATA_INSTANCE_LOSS_PENDING:
            process_event_data(*reinterpret_cast<XrEventDataInstanceLossPending*>(&event_data));
            session.running = false;
            return false;

        case XR_TYPE_EVENT_DATA_INTERACTION_PROFILE_CHANGED:
            process_event_data(*reinterpret_cast<XrEventDataInteractionProfileChanged*>(&event_data), session.handle);
            break;

        case XR_TYPE_EVENT_DATA_REFERENCE_SPACE_CHANGE_PENDING:
            process_event_data(*reinterpret_cast<XrEventDataReferenceSpaceChangePending*>(&event_data), session.handle);
            break;

        case XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED:
            process_session_state_changed(*reinterpret_cast<XrEventDataSessionStateChanged*>(&event_data));
            break;

        default:
            break;
        }
    }

    return true;
}

void XrSystem::recover_view_configurations()
{
    uint32_t view_config_count{};
    check_log(
        xrEnumerateViewConfigurations(context.instance, context.system_id, 0, &view_config_count, nullptr),
        "Failed to get view configuration count", context.instance
    );

    view_config_types.resize(view_config_count);

    check_log(
        xrEnumerateViewConfigurations(
            context.instance, context.system_id, view_config_count, &view_config_count,
            reinterpret_cast<XrViewConfigurationType*>(view_config_types.data())
        ),
        "Failed to enumerate view configurations", context.instance
    );

    for (XrViewConfigurationType const view_config_type :
         {XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO, XR_VIEW_CONFIGURATION_TYPE_PRIMARY_MONO}) {
        if (std::ranges::find(view_config_types, view_config_type) == view_config_types.cend()) {
            continue;
        }

        this->view_config_type = view_config_type;
        break;
    }

    if (view_config_type == 0) {
        Log::warning("[XrSystem] Failed to find a view configuration type; defaulting to "
                     "XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO");
        view_config_type = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
    }

    uint32_t view_config_view_count{};
    check_log(
        xrEnumerateViewConfigurationViews(
            context.instance, context.system_id, static_cast<XrViewConfigurationType>(view_config_type), 0,
            &view_config_view_count, nullptr
        ),
        "Failed to get view configuration view count", context.instance
    );

    view_config_views.resize(view_config_view_count, {XR_TYPE_VIEW_CONFIGURATION_VIEW});

    check_log(
        xrEnumerateViewConfigurationViews(
            context.instance, context.system_id, static_cast<XrViewConfigurationType>(view_config_type),
            view_config_view_count, &view_config_view_count, view_config_views.data()
        ),
        "Failed to enumerate view configuration views", context.instance
    );

#if !defined(NDEBUG) || defined(XEN_FORCE_DEBUG_LOG)
    {
        std::string view_config_views_msg = "[XrSystem] View configuration views:";
        for (XrViewConfigurationView const& view_config_view : view_config_views) {
            view_config_views_msg +=
                "\n    View:"
                "\n        Recom. image rect width:       " +
                std::to_string(view_config_view.recommendedImageRectWidth) +
                "\n        Max. image rect width:         " + std::to_string(view_config_view.maxImageRectWidth) +
                "\n        Recom. image rect height:      " +
                std::to_string(view_config_view.recommendedImageRectHeight) +
                "\n        Max. image rect height:        " + std::to_string(view_config_view.maxImageRectHeight) +
                "\n        Recom. swapchain sample count: " +
                std::to_string(view_config_view.recommendedSwapchainSampleCount) +
                "\n        Max. swapchain sample count:   " + std::to_string(view_config_view.maxSwapchainSampleCount);
        }
        Log::debug(view_config_views_msg);
    }
#endif
}

void XrSystem::recover_environment_blend_modes()
{
    uint32_t environment_blend_mode_count{};
    check_log(
        xrEnumerateEnvironmentBlendModes(
            context.instance, context.system_id, static_cast<XrViewConfigurationType>(view_config_type), 0,
            &environment_blend_mode_count, nullptr
        ),
        "Failed to get environment blend mode count", context.instance
    );

    environment_blend_modes.resize(environment_blend_mode_count);

    check_log(
        xrEnumerateEnvironmentBlendModes(
            context.instance, context.system_id, static_cast<XrViewConfigurationType>(view_config_type),
            environment_blend_mode_count, &environment_blend_mode_count,
            reinterpret_cast<XrEnvironmentBlendMode*>(environment_blend_modes.data())
        ),
        "Failed to enumerate environment blend modes", context.instance
    );

    for (XrEnvironmentBlendMode const environment_blend_mode :
         {XR_ENVIRONMENT_BLEND_MODE_OPAQUE, XR_ENVIRONMENT_BLEND_MODE_ADDITIVE}) {
        if (std::ranges::find(environment_blend_modes, environment_blend_mode) == environment_blend_modes.cend()) {
            continue;
        }

        this->environment_blend_mode = environment_blend_mode;
        break;
    }

    if (environment_blend_mode == 0) {
        Log::warning("Failed to find a compatible blend mode; defaulting to XR_ENVIRONMENT_BLEND_MODE_OPAQUE");
        environment_blend_mode = XR_ENVIRONMENT_BLEND_MODE_OPAQUE;
    }
}

void XrSystem::initialize_session()
{
    session.init(context.system_id);
    session.create_swapchains(view_config_views);
}

bool XrSystem::render_frame(ViewRenderFunc const& view_render_func) const
{
    return session.render_frame(view_config_views, view_config_type, environment_blend_mode, view_render_func);
}

bool XrSystem::process_session_state_changed(XrEventDataSessionStateChanged const& session_state_changed)
{
    if (session_state_changed.session != session.handle) {
        Log::info("[XrSystem] Data session state changed for unknown session");
        return true;
    }

    switch (session_state_changed.state) {
    case XR_SESSION_STATE_READY:
        session.begin(view_config_type);
        session.running = true;
        break;

    case XR_SESSION_STATE_STOPPING:
        session.end();
        session.running = false;
        break;

    case XR_SESSION_STATE_LOSS_PENDING:
        // TODO: "It's possible to try to reestablish an XrInstance and XrSession"
        session.running = false;
        return false;

    case XR_SESSION_STATE_EXITING:
        session.running = false;
        return false;

    default:
        break;
    }

    session.state = session_state_changed.state;

    return true;
}
}