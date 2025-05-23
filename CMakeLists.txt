cmake_minimum_required(VERSION 3.14...3.30.5)

project(
    xen
    VERSION 0.0.1
    DESCRIPTION "Modern & multiplatform 3D game engine"
    HOMEPAGE_URL https://github.com/xen-team/xen
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_COLOR_DIAGNOSTICS ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

option(XEN_USE_FBX              "XEN: Allows to import/export FBX models (requires the FBX SDK to be installed)"    ${FBX_FOUND})
option(XEN_USE_IMGUI            "XEN: Use ImGui"                                                                    ON)
option(XEN_USE_LUA              "XEN: Use Lua scripting (with Sol2)"                                                ON)
option(XEN_USE_PROFILING        "XEN: Use profiling capabilities (with Tracy)"                                      OFF)

option(XEN_OPTIMIZE             "XEN: Enable some optimizations"                                    OFF)
option(XEN_STATIC               "XEN: Enable static libs"                                           ON)
# option(XEN_TESTS              "XEN: Enable tests"                                                 OFF)
option(XEN_EXAMPLES             "XEN: Build examples"                                               OFF)
option(XEN_DOC                  "XEN: Generate documentation (requires Doxygen)"                    ${DOXYGEN_FOUND})
option(XEN_COVERAGE             "XEN: Enable code coverage (GCC only)"                              OFF)
option(XEN_DEBUG_INFO           "XEN: Creates a debug target which prints useful values"            OFF)
option(XEN_SANITIZERS           "XEN: Use compiler sanitizers"                                      OFF)
option(XEN_SKIP_RENDERER_ERRORS "XEN: Do not print errors from the Renderer"                        OFF)
option(XEN_FORCE_DEBUG_LOG      "XEN: Force the ouput of debug logging calls in non-Debug modes"    OFF)

if (XEN_STATIC)
    add_library(xen STATIC)
else ()
    add_library(xen SHARED)
endif ()

# Detect whether Emscripten is being used
if (CMAKE_CXX_COMPILER MATCHES "/em\\+\\+.*$")
    set(XEN_USE_EMSCRIPTEN ON)
else ()
    set(XEN_USE_EMSCRIPTEN OFF)
endif ()


if(DEFINED ENV{WAYLAND_DISPLAY})
    message(STATUS "Detected Wayland")
    add_definitions(-DPLATFORM_WAYLAND)
else()
    message(STATUS "Detected X11")
    add_definitions(-DPLATFORM_X11)
endif()


# if(XEN_STATIC)
#     set(BUILD_SHARED_LIBS OFF)
#     set(CMAKE_POSITION_INDEPENDENT_CODE ON)
#     set(CMAKE_LINK_SEARCH_END_STATIC ON)
#     add_link_options(-static-libgcc -static-libstdc++)
#     set(XEN_LIBTYPE STATIC)
# else()
#     set(XEN_LIBTYPE SHARED)
#     if (MSVC)
#         set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
#     endif()
# endif()


# Detect whether Emscripten is being used
if (CMAKE_CXX_COMPILER MATCHES "/em\\+\\+.*$")
    set(XEN_USE_EMSCRIPTEN ON)
else ()
    set(XEN_USE_EMSCRIPTEN OFF)
endif ()


if (MSVC AND NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang") # Finding exclusively MSVC, not clang-cl
    set(XEN_COMPILER "MSVC")
    set(XEN_IS_COMPILER_MSVC ON)
    target_compile_definitions(xen PUBLIC XEN_IS_COMPILER_MSVC)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    if (MSVC)
        # Using clang-cl, for which both MSVC & Clang are found
        set(XEN_COMPILER "Clang-cl")
        set(XEN_IS_COMPILER_CLANG_CL ON)
    else ()
        set(XEN_COMPILER "Clang")
    endif ()

    set(XEN_IS_COMPILER_CLANG ON)
    target_compile_definitions(xen PUBLIC XEN_IS_COMPILER_CLANG)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(XEN_COMPILER "GCC")
    set(XEN_COMPILER_GCC ON)
    target_compile_definitions(xen PUBLIC XEN_COMPILER_GCC)
endif ()


if (WIN32 OR CYGWIN OR MSYS)
    set(XEN_PLATFORM "Windows")
    set(XEN_IS_PLATFORM_WINDOWS ON)
    target_compile_definitions(xen PUBLIC XEN_IS_PLATFORM_WINDOWS)

    if (CYGWIN)
        set(XEN_PLATFORM "${XEN_PLATFORM} (Cygwin)")
        set(XEN_IS_PLATFORM_CYGWIN ON)
        target_compile_definitions(xen PUBLIC XEN_IS_PLATFORM_CYGWIN)
    endif ()

    # MSYS is defined only if the generator is "MSYS Makefiles"
    if (MSYS)
        set(XEN_PLATFORM "${XEN_PLATFORM} (MSYS)")
        set(XEN_IS_PLATFORM_MSYS ON)
        target_compile_definitions(xen PUBLIC XEN_IS_PLATFORM_MSYS)
    endif ()
elseif (APPLE)
    set(XEN_PLATFORM "macOS")
    set(XEN_IS_PLATFORM_MAC ON)
    target_compile_definitions(xen PUBLIC XEN_IS_PLATFORM_MAC)
elseif (XEN_USE_EMSCRIPTEN)
    set(XEN_PLATFORM "Emscripten")
    set(XEN_IS_PLATFORM_EMSCRIPTEN ON)
    target_compile_definitions(xen PUBLIC XEN_IS_PLATFORM_EMSCRIPTEN USE_OPENGL_ES USE_WEBGL)
elseif (UNIX)
    set(XEN_PLATFORM "Linux")
    set(XEN_IS_PLATFORM_LINUX ON)
    target_compile_definitions(xen PUBLIC XEN_IS_PLATFORM_LINUX)
endif ()


if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif ()


if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DXEN_DEBUG)
endif ()

if (XEN_IS_COMPILER_MSVC)
    set(XEN_CONFIG_DEBUG   "$<IF:$<CONFIG:Debug>,ON,OFF>")
    set(XEN_CONFIG_RELEASE "$<IF:$<CONFIG:Debug>,OFF,ON>")
    set(XEN_CONFIG_SHORT   "$<IF:$<CONFIG:Debug>,Debug,Release>")

    target_compile_definitions(xen PUBLIC $<IF:$<CONFIG:Debug>,XEN_CONFIG_DEBUG,XEN_CONFIG_RELEASE>)

    set(CONFIG_STR_UPPER $<UPPER_CASE:$<CONFIG>>)
else ()
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(XEN_CONFIG_DEBUG   ON)
        set(XEN_CONFIG_RELEASE OFF)
        set(XEN_CONFIG_SHORT   "Debug")

        target_compile_definitions(xen PUBLIC XEN_CONFIG_DEBUG)
    else ()
        set(XEN_CONFIG_DEBUG   OFF)
        set(XEN_CONFIG_RELEASE ON)
        set(XEN_CONFIG_SHORT   "Release")

        target_compile_definitions(xen PUBLIC XEN_CONFIG_RELEASE)
    endif ()

    string(TOUPPER ${CMAKE_BUILD_TYPE} CONFIG_STR_UPPER)
endif ()


if (ENABLE_DEBUG_INFO)
    cmake_host_system_information(RESULT PLATFORM_RELEASE QUERY OS_RELEASE)
    cmake_host_system_information(RESULT PLATFORM_VERSION QUERY OS_VERSION)

    # Trimming potential leading & trailing space characters (happens for Windows' values)
    string(STRIP ${PLATFORM_RELEASE} PLATFORM_RELEASE)
    string(STRIP ${PLATFORM_VERSION} PLATFORM_VERSION)

    add_custom_target(
        xen_PrintDebugInfo

        ${CMAKE_COMMAND} -E echo "" &&
        ${CMAKE_COMMAND} -E echo "####################" &&
        ${CMAKE_COMMAND} -E echo "# [xen] Debug info #" &&
        ${CMAKE_COMMAND} -E echo "####################" &&
        ${CMAKE_COMMAND} -E echo "" &&

        ${CMAKE_COMMAND} -E echo "--- Platform (found ${XEN_PLATFORM})" &&
        ${CMAKE_COMMAND} -E echo "  Name:     ${CMAKE_SYSTEM_NAME}" &&
        ${CMAKE_COMMAND} -E echo "  Release:  ${PLATFORM_RELEASE}" &&
        ${CMAKE_COMMAND} -E echo "  Version:  ${PLATFORM_VERSION}" &&
        ${CMAKE_COMMAND} -E echo "" &&

        ${CMAKE_COMMAND} -E echo "--- Compiler (found ${XEN_COMPILER})" &&
        ${CMAKE_COMMAND} -E echo "  ID:       ${CMAKE_CXX_COMPILER_ID}" &&
        ${CMAKE_COMMAND} -E echo "  Version:  ${CMAKE_CXX_COMPILER_VERSION}" &&
        ${CMAKE_COMMAND} -E echo "  Path:     ${CMAKE_CXX_COMPILER}" &&
        ${CMAKE_COMMAND} -E echo "" &&

        ${CMAKE_COMMAND} -E echo "--- Configuration" &&
        ${CMAKE_COMMAND} -E echo "  Name:               $<CONFIG>" &&
        ${CMAKE_COMMAND} -E echo "  XEN_CONFIG_DEBUG:   ${XEN_CONFIG_DEBUG}" &&
        ${CMAKE_COMMAND} -E echo "  XEN_CONFIG_RELEASE: ${XEN_CONFIG_RELEASE}" &&
        ${CMAKE_COMMAND} -E echo "  XEN_CONFIG_SHORT:   ${XEN_CONFIG_SHORT}" &&
        ${CMAKE_COMMAND} -E echo "" &&

        ${CMAKE_COMMAND} -E echo "--- Build flags: '${CMAKE_CXX_FLAGS_${CONFIG_STR_UPPER}}'" &&
        ${CMAKE_COMMAND} -E echo ""
    )
endif ()


if (XEN_USE_EMSCRIPTEN)
    target_compile_definitions(xen PUBLIC XEN_ROOT="/")
else ()
    target_compile_definitions(xen PUBLIC XEN_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/")
endif ()


if (XEN_SANITIZERS)
    set(SANITIZERS_OPTION USE_SANITIZERS)
endif ()


include(CompilerFlags)
add_compiler_flags(TARGET xen SCOPE PRIVATE ${SANITIZERS_OPTION})

if (XEN_COMPILER_GCC)
    if (XEN_CONFIG_DEBUG AND XEN_COVERAGE)
        target_compile_options(
            xen

            PRIVATE
                -g
                -O0
                -fno-inline
                -fno-inline-small-functions
                -fno-default-inline
                -fprofile-arcs
                -ftest-coverage
        )

        target_link_libraries(xen PRIVATE gcov)
    endif ()
endif ()


if (XEN_IS_PLATFORM_LINUX)
    list(APPEND XEN_LINKER_FLAGS pthread)
endif ()


if (XEN_USE_EMSCRIPTEN)
    target_link_options(xen PUBLIC "SHELL:-s USE_GLFW=3")
    list(APPEND XEN_LINKER_FLAGS glfw)
endif ()


if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to '${DEFAULT_BUILD_TYPE}' as none was specified.")
    set(CMAKE_BUILD_TYPE "${DEFAULT_BUILD_TYPE}" CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "RelWithDebInfo")
endif ()

# -ffunction-sections -fdata-sections -fno-unwind-tables -fno-asynchronous-unwind-tables ????
if(XEN_OPTIMIZE AND XEN_BUILD_TYPE EQUAL "Release")
    if (CMAKE_COMPILER_IS_GNUCC OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(xen PRIVATE -O2)
        target_compile_options(xen PRIVATE -fno-exceptions)
    elseif (MSVC)
        # using Visual Studio C++
    endif()
endif()


# if(XEN_OPTIMIZE)
#     include(CheckIPOSupported)
#     check_ipo_supported(RESULT supported OUTPUT error)

#     if(supported)
#         message(STATUS "IPO / LTO enabled")
#         set_property(TARGET xen PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
#     else()
#         message(STATUS "IPO / LTO not supported: <${error}>")
#     endif()
# endif()


add_subdirectory(depends)
add_subdirectory(src)

if(WIN32)
	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT xen)
endif()

if (XEN_EXAMPLES)
    add_subdirectory(examples)
endif()

# if (XEN_TESTS)
    # add_subdirectory(tests)
# endif()

find_package(Doxygen)
if (XEN_DOC)
    add_subdirectory(doc)
endif ()

if (XEN_IS_PLATFORM_WINDOWS)
    set(CMAKE_INSTALL_PREFIX "C:/xen")
endif ()

install(
    TARGETS xen
    ARCHIVE DESTINATION "lib/$<CONFIG>"
    LIBRARY DESTINATION "lib/$<CONFIG>"
    RUNTIME DESTINATION "bin/$<CONFIG>"
)

# Installing DLL files into the bin/ folder
if (XEN_IS_COMPILER_MSVC AND XEN_USE_AUDIO)
    install(FILES "${OPENAL_DLL}" RENAME "OpenAL32.dll"    DESTINATION "bin/$<CONFIG>")
    install(FILES "${OPENAL_DLL}" RENAME "libopenal-1.dll" DESTINATION "bin/$<CONFIG>")
endif ()

# set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
# set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
# set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")

# set(CPACK_PACKAGE_NAME "sfec")
# set(CPACK_PACKAGE_CONTACT https://github.com/nuclearbyte/sfec)

# set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)

# SET(CPACK_GENERATOR "TGZ")
# SET(CPACK_INCLUDE_TOPLEVEL_DIRECTORY 1)

# include(CPack)