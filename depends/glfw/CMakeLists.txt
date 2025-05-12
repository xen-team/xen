project(GLFW)

add_library(GLFW OBJECT)

target_compile_features(GLFW PRIVATE c_std_11)

# Defining preprocessor macros and selecting files to be removed
if (XEN_IS_PLATFORM_WINDOWS OR XEN_IS_PLATFORM_CYGWIN)
    set(
        GLFW_DEFINITIONS

        -D_GLFW_WIN32
    )

    set(
        GLFW_PLATFORM_SRC

        src/egl*
        src/osmesa*
        src/wgl*
        src/win32*
    )

    if (XEN_IS_PLATFORM_CYGWIN)
        # GLFW needs to be linked against the Windows Gdi32 library with Cygwin
        set(GLFW_LINKER_FLAGS Gdi32)
    endif ()
elseif (XEN_IS_PLATFORM_MAC)
    set(
        GLFW_DEFINITIONS

        -D_GLFW_COCOA
        -D_GLFW_USE_CHDIR
        -D_GLFW_USE_MENUBAR
    )

    set(
        GLFW_PLATFORM_SRC

        src/cocoa*
        src/egl*
        src/nsgl*
        src/osmesa*
        src/posix_thread*
    )

    set(
        GLFW_LINKER_FLAGS

        "-framework OpenGL"
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreVideo"
    )
elseif (XEN_IS_PLATFORM_LINUX)
    option(GLFW_USE_WAYLAND "Use Wayland instead of X11" OFF)

    if (GLFW_USE_WAYLAND)
        # Using Wayland
        set(
            GLFW_DEFINITIONS

            -D_GLFW_WAYLAND
        )

        set(
            GLFW_PLATFORM_SRC

            src/wl*
        )
    else ()
        # Using X11
        set(
            GLFW_DEFINITIONS

            -D_GLFW_X11
        )

        set(
            GLFW_PLATFORM_SRC

            src/glx*
            src/x11*
        )
    endif ()


    set(
        GLFW_PLATFORM_SRC

        ${GLFW_PLATFORM_SRC}
        src/egl*
        src/linux*
        src/osmesa*
        src/posix*
        src/xkb*
    )

    set(
        GLFW_LINKER_FLAGS

        dl
        pthread
        X11
        Xcursor
        Xinerama
        Xrandr
        Xxf86vm
    )
endif ()

file(
    GLOB
    GLFW_FILES

    # Common files
    src/context.c
    src/init.c
    src/input.c
    src/monitor.c
    src/vulkan.c
    src/window.c

    # Platform-specific files
    ${GLFW_PLATFORM_SRC}
)

target_sources(GLFW PRIVATE ${GLFW_FILES})

target_include_directories(
    GLFW
    SYSTEM

    PUBLIC
        "include"
)

target_compile_definitions(GLFW PUBLIC ${GLFW_DEFINITIONS})

# Disabling all compilers warnings
if (XEN_IS_COMPILER_MSVC)
    target_compile_options(GLFW PRIVATE /w)
else ()
    target_compile_options(GLFW PRIVATE -w)
endif ()

target_link_libraries(GLFW PUBLIC ${GLFW_LINKER_FLAGS})
