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

constexpr auto msaa_sample_amount = 4;
constexpr auto anisotropic_filtering_level = 16.f;
constexpr auto reflection_probe_min_count = 5;

constexpr auto shadowmap_resolution_x_default = 2048;
constexpr auto shadowmap_resolution_y_default = 2048;
constexpr auto shadowmap_near_plane_default = 1.0f;
constexpr auto shadowmap_far_plane_default = 200.0f;
constexpr auto shadowmap_bias_default = 0.007f;

constexpr auto parallax_min_steps = 1;
constexpr auto parallax_max_steps = 20;

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