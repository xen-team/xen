
#include "xr_context.hpp"

#if defined(XR_OS_WINDOWS)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Unknwn.h>
#elif defined(XR_OS_LINUX)
#include "GL/glx.h"
#endif

#include "openxr/openxr.h"
#include "openxr/openxr_platform.h"

#include <tracy/Tracy.hpp>

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
    return "[XrContext] " + error_msg + ": " + get_result_str(instance, result) + " (" + std::to_string(result) + ')';
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

XrBool32 log_callback(
    XrDebugUtilsMessageSeverityFlagsEXT severity, XrDebugUtilsMessageTypeFlagsEXT type,
    XrDebugUtilsMessengerCallbackDataEXT const* callback_data, void* /* userData */
)
{
    std::string severity_str;
    if (severity & XR_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
        severity_str += "INFO";
    }

    if (severity & XR_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        if (!severity_str.empty()) {
            severity_str += ',';
        }

        severity_str += "WARN";
    }

    if (severity & XR_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        if (!severity_str.empty()) {
            severity_str += ',';
        }

        severity_str += "ERROR";
    }

    constexpr auto get_type_str = [](XrDebugUtilsMessageTypeFlagsEXT message_type) {
        std::string msg_flags;

        if (message_type & XR_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT) {
            msg_flags += "GEN";
        }

        if (message_type & XR_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) {
            if (!msg_flags.empty()) {
                msg_flags += ',';
            }

            msg_flags += "SPEC";
        }

        if (message_type & XR_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) {
            if (!msg_flags.empty()) {
                msg_flags += ',';
            }

            msg_flags += "PERF";
        }

        return msg_flags;
    };

    std::string log_msg = "[OpenXR] ";

    if (callback_data->functionName) {
        log_msg += callback_data->functionName;
    }

    log_msg += '(' + severity_str + '/' + get_type_str(type) + "): ";

    if (callback_data->messageId) {
        log_msg += "ID: " + std::string(callback_data->messageId);
    }

    if (callback_data->message) {
        log_msg += " - " + std::string(callback_data->message);
    }

    Log::info(log_msg);

    // Returning false to indicate that the call should not be aborted
    // See: https://registry.khronos.org/OpenXR/specs/1.1/html/xrspec.html#PFN_xrDebugUtilsMessengerCallbackEXT
    return false;
}
}

XrContext::XrContext(std::string const& app_name)
{
    ZoneScopedN("XrContext::XrContext");

    Log::debug("[XrContext] Creating context...");

    recover_api_layers();
    recover_extensions();
    create_instance(app_name);

    XrInstanceProperties instance_properties{};
    instance_properties.type = XR_TYPE_INSTANCE_PROPERTIES;
    check_log(xrGetInstanceProperties(instance, &instance_properties), "Failed to get instance properties", instance);

    Log::vinfo(
        "[XrContext] OpenXR runtime: {} - {}.{}.{}", instance_properties.runtimeName,
        XR_VERSION_MAJOR(instance_properties.runtimeVersion), XR_VERSION_MINOR(instance_properties.runtimeVersion),
        XR_VERSION_PATCH(instance_properties.runtimeVersion)
    );

    create_debug_messenger();

    XrSystemGetInfo system_get_info{};
    system_get_info.type = XR_TYPE_SYSTEM_GET_INFO;
    system_get_info.formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY; // Always search for a VR headset for now
    check_throw(xrGetSystem(instance, &system_get_info, &system_id), "Failed to get system ID", instance);

    XrSystemProperties system_properties{};
    system_properties.type = XR_TYPE_SYSTEM_PROPERTIES;
    check_log(
        xrGetSystemProperties(instance, system_id, &system_properties), "Failed to get system properties", instance
    );

    Log::debug(
        "[XrContext] System properties:"
        "\n    - System ID:             " +
        std::to_string(system_properties.systemId) +
        "\n    - Vendor ID:             " + std::to_string(system_properties.vendorId) +
        "\n    - Name:                  " + system_properties.systemName + "\n    - Max. swapchain height: " +
        std::to_string(system_properties.graphicsProperties.maxSwapchainImageHeight) +
        "\n    - Max. swapchain width:  " +
        std::to_string(system_properties.graphicsProperties.maxSwapchainImageWidth) +
        "\n    - Max. layer count:      " + std::to_string(system_properties.graphicsProperties.maxLayerCount) +
        "\n    - Orientation tracking:  " +
        (system_properties.trackingProperties.orientationTracking == XR_TRUE ? "true" : "false") +
        "\n    - Position tracking:     " +
        (system_properties.trackingProperties.positionTracking == XR_TRUE ? "true" : "false")
    );

    Log::debug("[XrContext] Created context");
}

XrContext::~XrContext()
{
    ZoneScopedN("XrContext::~XrContext");

    Log::debug("[XrContext] Destroying context...");
    destroy_debug_messenger();
    destroy_instance();
    Log::debug("[XrContext] Destroyed context");
}

void XrContext::recover_api_layers()
{
    ZoneScopedN("XrContext::recover_api_layers");

    Log::debug("[XrContext] Recovering API layers...");

    uint32_t api_layer_count{};
    std::vector<XrApiLayerProperties> api_layer_properties;
    check_log(
        xrEnumerateApiLayerProperties(0, &api_layer_count, nullptr), "Failed to get API layer property count", instance
    );

    api_layer_properties.resize(api_layer_count, {XR_TYPE_API_LAYER_PROPERTIES});
    check_log(
        xrEnumerateApiLayerProperties(api_layer_count, &api_layer_count, api_layer_properties.data()),
        "Failed to enumerate API layer properties", instance
    );

#if !defined(NDEBUG) || defined(XEN_FORCE_DEBUG_LOG)
    {
        std::string available_layers_msg = "[XrContext] Available layers:";
        for (XrApiLayerProperties const& layer_prop : api_layer_properties) {
            available_layers_msg +=
                "\n    " + std::string(layer_prop.layerName) + "\n     -> " + std::string(layer_prop.description);
        }
        Log::debug(available_layers_msg);
    }
#endif

    for (const std::string& requested_layer : api_layers) {
        auto const layer_prop_it = std::find_if(
            api_layer_properties.cbegin(), api_layer_properties.cend(),
            [&requested_layer](XrApiLayerProperties const& layer_prop) {
                return requested_layer == layer_prop.layerName;
            }
        );

        if (layer_prop_it != api_layer_properties.cend()) {
            active_api_layers.emplace_back(requested_layer.c_str());
        }
        else {
            Log::error("[XrContext] Failed to find OpenXR API layer: ", requested_layer);
        }
    }

#if !defined(NDEBUG) || defined(XEN_FORCE_DEBUG_LOG)
    if (!active_api_layers.empty()) {
        std::string active_layers_msg = "[XrContext] Active layers:";
        for (char const* activeLayer : active_api_layers) {
            active_layers_msg += "\n    " + std::string(activeLayer);
        }

        Log::debug(active_layers_msg);
    }
#endif

    Log::debug("[XrContext] Recovered API layers");
}

void XrContext::recover_extensions()
{
    ZoneScopedN("XrContext::recover_extensions");

    Log::debug("[XrContext] Recovering extensions...");

    extensions.emplace_back(XR_EXT_DEBUG_UTILS_EXTENSION_NAME);
    extensions.emplace_back(XR_KHR_OPENGL_ENABLE_EXTENSION_NAME);

    uint32_t extension_count{};
    std::vector<XrExtensionProperties> extension_properties;
    check_log(
        xrEnumerateInstanceExtensionProperties(nullptr, 0, &extension_count, nullptr),
        "Failed to get instance extension property count", instance
    );
    extension_properties.resize(extension_count, {XR_TYPE_EXTENSION_PROPERTIES});
    check_log(
        xrEnumerateInstanceExtensionProperties(nullptr, extension_count, &extension_count, extension_properties.data()),
        "Failed to enumerate instance extension properties", instance
    );

#if !defined(NDEBUG) || defined(XEN_FORCE_DEBUG_LOG)
    {
        std::string available_extensions_msg = "[XrContext] Available extensions:";
        for (XrExtensionProperties const& extension_prop : extension_properties) {
            std::string const extension_version_str =
                std::to_string(XR_VERSION_MAJOR(extension_prop.extensionVersion)) + '.' +
                std::to_string(XR_VERSION_MINOR(extension_prop.extensionVersion)) + '.' +
                std::to_string(XR_VERSION_PATCH(extension_prop.extensionVersion));
            available_extensions_msg +=
                "\n    " + std::string(extension_prop.extensionName) + " - " + extension_version_str;
        }
        Log::debug(available_extensions_msg);
    }
#endif

    for (const std::string& requested_extension : extensions) {
        auto const extension_it = std::find_if(
            extension_properties.cbegin(), extension_properties.cend(),
            [&requested_extension](XrExtensionProperties const& extension_prop) {
                return requested_extension == extension_prop.extensionName;
            }
        );

        if (extension_it != extension_properties.cend()) {
            active_extensions.emplace_back(requested_extension.c_str());
        }
        else {
            Log::error("[XrContext] Failed to find OpenXR instance extension: " + requested_extension);
        }
    }

#if !defined(NDEBUG) || defined(XEN_FORCE_DEBUG_LOG)
    if (!active_extensions.empty()) {
        std::string active_extensions_msg = "[XrContext] Active extensions:";
        for (char const* active_extension : active_extensions) {
            active_extensions_msg += "\n    " + std::string(active_extension);
        }

        Log::debug(active_extensions_msg);
    }
#endif

    Log::debug("[XrContext] Recovered extensions");
}

void XrContext::create_instance(std::string const& app_name)
{
    ZoneScopedN("XrContext::create_instance");

    Log::debug("[XrContext] Creating instance...");

    XrApplicationInfo app_info{};
    app_name.copy(app_info.applicationName, XR_MAX_APPLICATION_NAME_SIZE - 1);
    app_info.applicationVersion = 1;
    std::string("xen").copy(app_info.engineName, XR_MAX_ENGINE_NAME_SIZE - 1);
    app_info.engineVersion = 1;
    app_info.apiVersion = XR_MAKE_VERSION(1, 0, 34); // XR_CURRENT_API_VERSION is too recent for the tested runtime

    XrInstanceCreateInfo instance_create_info{};
    instance_create_info.type = XR_TYPE_INSTANCE_CREATE_INFO;
    instance_create_info.createFlags = 0;
    instance_create_info.applicationInfo = app_info;
    instance_create_info.enabledApiLayerCount = static_cast<uint32_t>(active_api_layers.size());
    instance_create_info.enabledApiLayerNames = active_api_layers.data();
    instance_create_info.enabledExtensionCount = static_cast<uint32_t>(active_extensions.size());
    instance_create_info.enabledExtensionNames = active_extensions.data();
    check_throw(xrCreateInstance(&instance_create_info, &instance), "Failed to create instance", instance);

    Log::debug("[XrContext] Created instance");
}

void XrContext::destroy_instance()
{
    ZoneScopedN("XrContext::destroy_instance");

    Log::debug("[XrContext] Destroying instance...");
    check_log(xrDestroyInstance(instance), "Failed to destroy instance", instance);
    Log::debug("[XrContext] Destroyed instance");
}

void XrContext::create_debug_messenger()
{
    ZoneScopedN("XrContext::create_debug_messenger");

    if (std::find(
            active_extensions.cbegin(), active_extensions.cend(), std::string_view(XR_EXT_DEBUG_UTILS_EXTENSION_NAME)
        ) == active_extensions.cend()) {
        return; // Extension not found
    }

    Log::debug("[XrContext] Creating debug messenger...");

    XrDebugUtilsMessengerCreateInfoEXT debug_msgr_create_info{};
    debug_msgr_create_info.type = XR_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debug_msgr_create_info.messageSeverities =
        XR_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | XR_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        XR_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT; // Ignoring verbose severity
    debug_msgr_create_info.messageTypes =
        XR_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | XR_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        XR_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | XR_DEBUG_UTILS_MESSAGE_TYPE_CONFORMANCE_BIT_EXT;
    debug_msgr_create_info.userCallback = log_callback;
    debug_msgr_create_info.userData = nullptr;

    PFN_xrCreateDebugUtilsMessengerEXT xrCreateDebugUtilsMessengerEXT{};
    check_log(
        xrGetInstanceProcAddr(
            instance, "xrCreateDebugUtilsMessengerEXT",
            reinterpret_cast<PFN_xrVoidFunction*>(&xrCreateDebugUtilsMessengerEXT)
        ),
        "Failed to get debug messenger create function", instance
    );

    check_log(
        xrCreateDebugUtilsMessengerEXT(instance, &debug_msgr_create_info, &debug_msgr),
        "Failed to create debug messenger", instance
    );

    Log::debug("[XrContext] Created debug messenger");
}

void XrContext::destroy_debug_messenger()
{
    ZoneScopedN("XrContext::destroy_debug_messenger");

    if (debug_msgr == XR_NULL_HANDLE) {
        return;
    }

    if (std::find(
            active_extensions.cbegin(), active_extensions.cend(), std::string_view(XR_EXT_DEBUG_UTILS_EXTENSION_NAME)
        ) == active_extensions.cend()) {
        return; // Extension not found
    }

    Log::debug("[XrContext] Destroying debug messenger...");

    PFN_xrDestroyDebugUtilsMessengerEXT xrDestroyDebugUtilsMessengerEXT{};
    check_log(
        xrGetInstanceProcAddr(
            instance, "xrDestroyDebugUtilsMessengerEXT",
            reinterpret_cast<PFN_xrVoidFunction*>(&xrDestroyDebugUtilsMessengerEXT)
        ),
        "Failed to get debug messenger destroy function", instance
    );

    check_log(xrDestroyDebugUtilsMessengerEXT(debug_msgr), "Failed to destroy debug messenger", instance);

    Log::debug("[XrContext] Destroyed debug messenger");
}

}
