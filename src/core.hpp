#pragma once

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

#define GL_CALL(x) x

// #if defined(USE_RENDERDOC) && defined(ARC_DEV_BUILD)
#define XEN_PUSH_RENDER_GROUP(name) glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, name);
#define XEN_POP_RENDER_GROUP() glPopDebugGroup();
// #else
// #define ARC_PUSH_RENDER_TAG(name)
// #define ARC_POP_RENDER_TAG()
// #endif

#if defined(_MSC_VER)
/// Declares a pragma directive, passing it the given argument.
/// \param Arg Argument to be passed to the pragma directive.
#define DO_PRAGMA(Arg) __pragma(Arg)

/// Saves the warnings' state to be reapplied later.
/// \see POP_WARNINGS_STATE.
#define PUSH_WARNINGS_STATE DO_PRAGMA(warning(push))

/// Reapplies the previously pushed warnings' state.
/// \see PUSH_WARNINGS_STATE.
#define POP_WARNINGS_STATE DO_PRAGMA(warning(pop))

/// Disables the given MSVC warning; must be a 4 digits number.
/// \note Make sure the warnings' state has been pushed before.
/// \param Warning ID of the warning to disable.
#define DISABLE_WARNING(Warning) DO_PRAGMA(warning(disable : Warning))

/// Disables the given MSVC warning; must be a 4 digits number.
/// \note Make sure the warnings' state has been pushed before.
/// \param Warning ID of the warning to disable.
#define DISABLE_WARNING_MSVC(Warning) DISABLE_WARNING(Warning)

#define DISABLE_WARNING_GCC(Warning)
#define DISABLE_WARNING_CLANG(Warning)
#elif defined(__GNUC__) || defined(__clang__)
/// Declares a pragma directive, passing it the given argument.
/// \param Arg Argument to be passed to the pragma directive.
#define DO_PRAGMA(Arg) _Pragma(#Arg)

/// Saves the warnings' state to be reapplied later.
/// \see POP_WARNINGS_STATE.
#define PUSH_WARNINGS_STATE DO_PRAGMA(GCC diagnostic push)

/// Reapplies the previously pushed warnings' state.
/// \see PUSH_WARNINGS_STATE.
#define POP_WARNINGS_STATE DO_PRAGMA(GCC diagnostic pop)

/// Disables the given GCC/Clang warning; must be an unquoted string starting with -W.
/// \note Make sure the warnings' state has been pushed before.
/// \param Warning Unquoted name of the warning to disable.
#define DISABLE_WARNING(Warning) DO_PRAGMA(GCC diagnostic ignored #Warning)

#if defined(__clang__)
/// Disables the given Clang warning; must be an unquoted string starting with -W.
/// \note Make sure the warnings' state has been pushed before.
/// \param Warning Unquoted name of the warning to disable.
#define DISABLE_WARNING_CLANG(Warning) DISABLE_WARNING(Warning)

#define DISABLE_WARNING_GCC(Warning)
#else
/// Disables the given GCC warning; must be an unquoted string starting with -W.
/// \note Make sure the warnings' state has been pushed before.
/// \param Warning Unquoted name of the warning to disable.
#define DISABLE_WARNING_GCC(Warning) DISABLE_WARNING(Warning)

#define DISABLE_WARNING_CLANG(Warning)
#endif

#define DISABLE_WARNING_MSVC(Warning)
#else
#define DO_PRAGMA(Arg)
#define PUSH_WARNINGS_STATE
#define POP_WARNINGS_STATE
#define DISABLE_WARNING(Warning)
#define DISABLE_WARNING_GCC(Warning)
#define DISABLE_WARNING_CLANG(Warning)
#define DISABLE_WARNING_MSVC(Warning)
#endif

constexpr auto msaa_sample_amount = 4;
constexpr auto anisotropic_filtering_level = 16.f;
constexpr auto reflection_probe_min_count = 5;

constexpr auto shadowmaresolution_x_default = 2048;
constexpr auto shadowmaresolution_y_default = 2048;
constexpr auto shadowmap_near_plane_default = 1.0f;
constexpr auto shadowmap_far_plane_default = 200.0f;
constexpr auto shadowmap_bias_default = 0.007f;

constexpr auto c_parallax_min_steps = 1;
constexpr auto c_parallax_max_steps = 20;

constexpr auto ssao_kernel_size = 32;

constexpr auto light_probe_resolution = 32;
constexpr auto reflection_probe_mip_count = 5;
constexpr auto reflection_probe_resolution = 128;
constexpr auto ibl_capture_resolution = 256; // Should always be greater than the light and reflection probe resolution
constexpr auto brdf_lut_resolution = 512;

constexpr auto water_reflection_near_plane_default = 0.3f;
constexpr auto water_reflection_far_plane_default = 100.0f;
constexpr auto water_refraction_near_plane_default = 0.3f;
constexpr auto water_refraction_far_plane_default = 500.0f;

template <typename T>
concept Integral = std::is_integral_v<T>;

namespace fs = std::filesystem;