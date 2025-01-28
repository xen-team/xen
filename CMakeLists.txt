cmake_minimum_required(VERSION 3.16 FATAL_ERROR)
project(xen VERSION 0.0.1 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_COLOR_DIAGNOSTICS ON)

option(XEN_WARNINGS               "XEN: Enable all compiler warnings"    ON)
option(XEN_WARNINGS_ERRORS        "XEN: Threat warnings as errors"       OFF)

option(XEN_OPTIMIZE               "XEN: Enable some optimizations"       OFF)

option(XEN_STATIC                 "XEN: Enable static libs"               OFF)

# option(XEN_TESTS                "XEN: enable tests"                    OFF)
option(XEN_EXAMPLES                "XEN: Build examples"                    OFF)

if(DEFINED ENV{WAYLAND_DISPLAY})
    message(STATUS "Detected Wayland")
    add_definitions(-DPLATFORM_WAYLAND)
else()
    message(STATUS "Detected X11")
    add_definitions(-DPLATFORM_X11)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Building in Debug mode")
    add_definitions(-DXEN_DEBUG)
else()
    message(STATUS "Building in Release mode")
endif()

if(XEN_STATIC)
    set(BUILD_SHARED_LIBS OFF)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
    set(CMAKE_LINK_SEARCH_END_STATIC ON)
    add_link_options(-static-libgcc -static-libstdc++)
    set(XEN_LIBTYPE STATIC)
else()
    set(XEN_LIBTYPE SHARED)
    if (MSVC)
        set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
    endif()
endif()

include(cmake/depends.cmake)

add_subdirectory(src)

set(DEFAULT_BUILD_TYPE "Release")
if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to '${DEFAULT_BUILD_TYPE}' as none was specified.")
    set(CMAKE_BUILD_TYPE "${DEFAULT_BUILD_TYPE}" CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "RelWithDebInfo")
endif ()

string(TOUPPER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE)

target_compile_definitions(xen PRIVATE XEN_${CMAKE_BUILD_TYPE})

# -ffunction-sections -fdata-sections -fno-unwind-tables -fno-asynchronous-unwind-tables ????
if(XEN_OPTIMIZE AND XEN_BUILD_TYPE EQUAL "Release")
    if (CMAKE_COMPILER_IS_GNUCC OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(xen PRIVATE -O2)
    elseif (MSVC)
        # using Visual Studio C++
    endif()
endif()

#target_compile_options(foo PUBLIC "$<$<CONFIG:DEBUG>:${MY_DEBUG_OPTIONS}>")
#target_compile_options(foo PUBLIC "$<$<CONFIG:RELEASE>:${MY_RELEASE_OPTIONS}>")

if (XEN_WARNINGS)
    if (CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(xen PRIVATE -Wall -Wextra -Wpedantic)
    else (MSVC)
        target_compile_options(xen PRIVATE /W4 /WX)
    endif()
endif()

message("Build Type: ${CMAKE_BUILD_TYPE}")

if(XEN_OPTIMIZE)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT supported OUTPUT error)

    if(supported)
        message(STATUS "IPO / LTO enabled")
        set_property(TARGET xen PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    else()
        message(STATUS "IPO / LTO not supported: <${error}>")
    endif()
endif()

target_precompile_headers(xen PUBLIC src/prec.hpp)

if(WIN32)
	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT xen)
endif()

if (XEN_EXAMPLES)
    include(examples/examples.cmake)
endif()

# set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
# set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
# set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")

# set(CPACK_PACKAGE_NAME "sfec")
# set(CPACK_PACKAGE_CONTACT https://github.com/nuclearbyte/sfec)

# set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)

# SET(CPACK_GENERATOR "TGZ")
# SET(CPACK_INCLUDE_TOPLEVEL_DIRECTORY 1)

# include(CPack)