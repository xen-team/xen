project(GLEW)

add_library(GLEW OBJECT)

target_compile_features(GLEW PRIVATE c_std_11)

if (XEN_IS_PLATFORM_WINDOWS OR XEN_IS_PLATFORM_CYGWIN)
    set(GLEW_LINKER_FLAGS opengl32)
elseif (XEN_IS_PLATFORM_LINUX)
    set(GLEW_LINKER_FLAGS GL)
elseif (XEN_IS_PLATFORM_MAC)
    find_package(OpenGL REQUIRED)
    set(GLEW_LINKER_FLAGS OpenGL::GL)
endif ()

file(
    GLOB
    GLEW_FILES

    src/*.c
    "include/GL/*.h"
)

target_sources(GLEW PRIVATE ${GLEW_FILES})

if (XEN_IS_PLATFORM_CYGWIN)
    # Cygwin voluntarily removes the _WIN32 definition, which GLEW requires here
    target_compile_definitions(GLEW PRIVATE _WIN32)

    # Since ptrdiff_t is redefined with Clang, it needs some more tinkering...
    if (XEN_IS_COMPILER_CLANG)
        # ptrdiff_t is actually redefined by GLEW; _PTR_DIFF_T_ is added so that this doesn't happen
        # __need_ptrdiff_t is then defined so that the standard library can make the type available by itself
        target_compile_definitions(
            GLEW

            PRIVATE

            _PTRDIFF_T_
            __need_ptrdiff_t
        )
    endif ()
endif ()

target_include_directories(GLEW SYSTEM PUBLIC include)

target_compile_definitions(
    GLEW

    PUBLIC
        GLEW_STATIC
        GLEW_NO_GLU
)

# Disabling all compilers warnings
if (XEN_IS_COMPILER_MSVC)
    target_compile_options(GLEW PRIVATE /w)
else ()
    target_compile_options(GLEW PRIVATE -w)
endif ()

target_link_libraries(GLEW PUBLIC ${GLEW_LINKER_FLAGS})
