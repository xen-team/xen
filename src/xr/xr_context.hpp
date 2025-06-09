#pragma once

struct XrInstance_T;
struct XrDebugUtilsMessengerEXT_T;

namespace xen {
class XrContext {
    friend class XrSession;
    friend class XrSystem;

public:
    explicit XrContext(std::string const& app_name);

    ~XrContext();

private:
    using XrInstance = XrInstance_T*;
    using XrDebugUtilsMessengerEXT = XrDebugUtilsMessengerEXT_T*;

    std::vector<std::string> api_layers;
    std::vector<char const*> active_api_layers;
    std::vector<std::string> extensions;
    std::vector<char const*> active_extensions;

    XrInstance instance{};
    XrDebugUtilsMessengerEXT debug_msgr{};
    uint64_t system_id{};

private:
    void recover_api_layers();
    void recover_extensions();
    void create_instance(std::string const& app_name);
    void destroy_instance();
    void create_debug_messenger();
    void destroy_debug_messenger();
};
}