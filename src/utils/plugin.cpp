#include "plugin.hpp"

#include <utils/filepath.hpp>

#if defined(XEN_IS_PLATFORM_WINDOWS)
#if defined(XEN_IS_COMPILER_MSVC)
struct IUnknown; // Workaround for "combaseapi.h(229): error C2187: syntax error: 'identifier' was unexpected here" when
                 // using /permissive-
#endif

#include <Windows.h>
#else
#include <dlfcn.h>
#endif

namespace xen {
void Plugin::load(FilePath const& plugin_path)
{
    close();

#if defined(XEN_IS_PLATFORM_WINDOWS)
    plugin_handle = LoadLibraryW(plugin_path.to_wide().c_str());
#else
    plugin_handle = dlopen(plugin_path.to_utf8().c_str(), RTLD_LAZY);
#endif
}

void Plugin::close()
{
    if (plugin_handle == nullptr) {
        return;
    }

#if defined(XEN_IS_PLATFORM_WINDOWS)
    FreeLibrary(static_cast<HMODULE>(plugin_handle));
#else
    dlclose(plugin_handle);
#endif

    plugin_handle = nullptr;
}

void* Plugin::load_function_pointer(std::string const& function_name)
{
#if defined(XEN_IS_PLATFORM_WINDOWS)
    static_assert(sizeof(void*) == sizeof(FARPROC));

    void* res{};

    FARPROC func_ptr = GetProcAddress(static_cast<HMODULE>(plugin_handle), function_name.c_str());
    std::memcpy(&res, &func_ptr, sizeof(void*));

    return res;
#else
    return dlsym(plugin_handle, function_name.c_str());
#endif
}
}