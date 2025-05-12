project(OpenXR)

add_library(OpenXR STATIC)

target_compile_features(OpenXR PRIVATE cxx_std_14)

file(
    GLOB
    OPENXR_FILES

    src/xr_generated_dispatch_table_core.c
    src/common/*.cpp
    src/external/jsoncpp/src/lib_json/*.cpp
    src/loader/*.cpp

    src/*.h
    src/common/*.h
    src/common/*.hpp
    src/loader/*.h
    src/loader/*.hpp
    "include/openxr/*.h"
)

target_sources(OpenXR PRIVATE ${OPENXR_FILES})

if (WIN32)
    target_compile_definitions(OpenXR PUBLIC XR_OS_WINDOWS XR_USE_PLATFORM_WIN32 NOMINMAX)
elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_compile_definitions(OpenXR PUBLIC XR_OS_LINUX XR_USE_PLATFORM_XLIB)
endif ()

target_compile_definitions(OpenXR PUBLIC XR_USE_GRAPHICS_API_OPENGL)

# Check for the existence of the secure_getenv or __secure_getenv commands
include(CheckFunctionExists)
check_function_exists(secure_getenv HAVE_SECURE_GETENV)
check_function_exists(__secure_getenv HAVE___SECURE_GETENV)
configure_file(src/common_config.h.in "${CMAKE_CURRENT_BINARY_DIR}/common_config.h")
target_compile_definitions(OpenXR PRIVATE OPENXR_HAVE_COMMON_CONFIG)

target_include_directories(
    OpenXR
    SYSTEM

    PUBLIC
        "include"
    PRIVATE
        src
        src/common
        src/external/jsoncpp/include
        "${CMAKE_CURRENT_BINARY_DIR}"
)

# Disabling all compilers warnings
if (XEN_IS_COMPILER_MSVC)
    target_compile_options(OpenXR PRIVATE /w)
else ()
    target_compile_options(OpenXR PRIVATE -w)
endif ()
