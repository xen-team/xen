project(ImGui)

# If not compiling with Emscripten, GLEW and GLFW need to be enabled for ImGui to be available
if (NOT XEN_USE_EMSCRIPTEN)
    if (NOT XEN_USE_GLEW OR NOT XEN_USE_WINDOW)
        message(FATAL_ERROR "[xen] ImGui can't be used with either GLEW or windowing disabled; either reenable both or disable ImGui")
    endif ()
endif ()

add_library(ImGui OBJECT)

target_compile_features(ImGui PRIVATE cxx_std_17)

# The needed Freetype header ft2build.h seems to be found only when calling aux_source_directory(); this call is thus kept instead of manually finding files
aux_source_directory(imgui IMGUI_SRC)

file(
    GLOB_RECURSE
    IMGUI_FILES

    ${IMGUI_SRC}
    misc/cpp/*.cpp
    imgui_draw.cpp
    imgui_impl_opengl3.cpp
    imgui_tables.cpp
    imgui_widgets.cpp
    imgui.cpp
    imgui_impl_glfw.cpp
    *.h
    ../implot/*.cpp
    ../implot/*.h
)

if (NOT XEN_USE_WINDOW AND NOT XEN_USE_EMSCRIPTEN)
    list(
        REMOVE_ITEM
        IMGUI_FILES

        imgui_impl_glfw.h
        imgui_impl_glfw.cpp
     )
endif ()

target_sources(ImGui PRIVATE ${IMGUI_FILES})

target_include_directories(
    ImGui
    SYSTEM

    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../implot
)

# Disabling all compilers warnings
if (XEN_IS_COMPILER_MSVC)
    target_compile_options(ImGui PRIVATE /w)
else ()
    target_compile_options(ImGui PRIVATE -w)

    if (XEN_USE_EMSCRIPTEN)
        target_compile_options(ImGui PRIVATE -DIMGUI_IMPL_OPENGL_ES3 -s USE_GLFW=3)
    endif ()
endif ()

target_link_libraries(ImGui PUBLIC GLEW)

if (XEN_USE_WINDOW AND NOT XEN_USE_EMSCRIPTEN)
    target_link_libraries(ImGui PUBLIC GLFW)
endif ()