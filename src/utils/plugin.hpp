#pragma once

namespace xen {
class FilePath;

class Plugin {
public:
    Plugin() = default;
    explicit Plugin(FilePath const& plugin_path) { load(plugin_path); }
    ~Plugin() { close(); }

    void load(FilePath const& plugin_path);

    bool is_loaded() const { return (plugin_handle != nullptr); }

    template <typename FuncT>
    FuncT load_function(std::string const& function_name)
    {
        static_assert(sizeof(FuncT) == sizeof(void*));

        FuncT func{};

        void* func_ptr = load_function_pointer(function_name);
        std::memcpy(&func, &func_ptr, sizeof(FuncT));

        return func;
    }
    void close();

private:
    void* plugin_handle{};

private:
    void* load_function_pointer(std::string const& function_name);
};
}