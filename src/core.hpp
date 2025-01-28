#if !defined(_WIN32) && !defined(__linux__)
#error Platform not supported
#endif

#ifdef _WIN32
#define XEN_LIB_EXPORT __declspec(dllexport)
#define XEN_LIB_IMPORT __declspec(dllimport)
#define XEN_FORCE_INLINE __forceinline
#else
#define XEN_LIB_EXPORT __attribute__((visibility("default")))
#define XEN_LIB_IMPORT
#define XEN_FORCE_INLINE __attribute__((always_inline)) inline
#endif

#ifdef XEN_STATIC
#define XEN_API
#elif defined xen_EXPORTS
#define XEN_API XEN_LIB_EXPORT
#else
#define XEN_API XEN_LIB_IMPORT
#endif