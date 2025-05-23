file(GLOB_RECURSE XEN_FILES **.cpp **.hpp **.inl)

find_package(OpenAL)

if (XEN_USE_EMSCRIPTEN)
    # Emscripten's FindOpenAL.cmake script historically sets the capitalized found variable, but always finds it anyway
    set(OpenAL_FOUND ON)

    # Emscripten's OpenAL implementation does not (yet?) support audio effects
    list(
        REMOVE_ITEM
        XEN_FILES

        "${PROJECT_SOURCE_DIR}/src/audio/sound_effect.cpp"
        "${PROJECT_SOURCE_DIR}/src/audio/sound_effect.hpp"
        "${PROJECT_SOURCE_DIR}/src/audio/sound_effect_slot.cpp"
        "${PROJECT_SOURCE_DIR}/src/audio/sound_effect_slot.hpp"
    )
endif ()

option(XEN_USE_AUDIO "Enable audio capabilities (requires OpenAL to be installed)" ${OpenAL_FOUND})
if (XEN_USE_AUDIO)
    if (NOT OpenAL_FOUND)
        message(SEND_ERROR "Audio required but OpenAL not found; please review its location")
    endif ()

    if (NOT XEN_USE_EMSCRIPTEN)
        # Emscripten manages OpenAL on its own; linking to it messes up predefined types
        target_link_libraries(xen PRIVATE OpenAL::OpenAL)
    endif ()
    target_compile_definitions(xen PUBLIC XEN_USE_AUDIO)

    message(STATUS "[xen] Audio ENABLED")
else ()
    file(GLOB_RECURSE XEN_AUDIO_FILES 
        "${PROJECT_SOURCE_DIR}/src/audio/*.cpp" 
        "${PROJECT_SOURCE_DIR}/src/audio/*.hpp" 
        "${PROJECT_SOURCE_DIR}/src/audio/*.inl" 
        "${PROJECT_SOURCE_DIR}/src/data/wav_load.cpp"
        "${PROJECT_SOURCE_DIR}/src/data/wav_save.cpp"
        "${PROJECT_SOURCE_DIR}/src/script/lua_audio.cpp"
        "${PROJECT_SOURCE_DIR}/src/data/wav_format.hpp"
    )

    list(REMOVE_ITEM XEN_FILES ${XEN_AUDIO_FILES})

    message(STATUS "[xen] Audio DISABLED")
endif ()

# Under Windows, Clang (not Clang-cl) uses Windows'/MSVC's libraries by default, and so should be compatible with the FBX SDK
# The SDK is unavailable with MinGW, which is considered as GCC
if (XEN_IS_COMPILER_MSVC OR XEN_IS_COMPILER_CLANG OR (XEN_COMPILER_GCC AND NOT MINGW))
    find_package(FBX)

    if (XEN_USE_FBX)
        if (NOT FBX_FOUND)
            message(SEND_ERROR "FBX required but SDK not found; please review its location")
        endif ()

        target_link_libraries(xen PRIVATE FBX::FBX)
        target_compile_definitions(xen PUBLIC XEN_USE_FBX)

        message(STATUS "[xen] FBX support ENABLED")
    else ()
        message(STATUS "[xen] FBX support DISABLED")
    endif ()
endif ()

# If FBX SDK not used or found, do not reference FBX files
if (NOT XEN_USE_FBX OR NOT FBX_FOUND)
    list(
        REMOVE_ITEM
        XEN_FILES

        "${PROJECT_SOURCE_DIR}/src/data/fbx_load.cpp"
        "${PROJECT_SOURCE_DIR}/src/data/fbx_format.hpp"
    )
endif ()

include(EmbedFiles)
embed_files(
    INPUT_PATTERN "${PROJECT_SOURCE_DIR}/shaders/*.*"
    OUTPUT_FOLDER "${CMAKE_BINARY_DIR}/shaders"
    MAIN_TARGET xen
    EMBED_TARGET_SUFFIX Shaders
)

target_include_directories(xen PUBLIC .)

if (XEN_SKIP_RENDERER_ERRORS)
    target_compile_definitions(xen PRIVATE XEN_SKIP_RENDERER_ERRORS)
endif ()

if (XEN_FORCE_DEBUG_LOG)
    target_compile_definitions(xen PUBLIC XEN_FORCE_DEBUG_LOG)
endif ()

target_link_libraries(xen PRIVATE ${XEN_LINKER_FLAGS})

# Cygwin's Clang needs to use GCC's standard library
if (CYGWIN AND XEN_IS_COMPILER_CLANG)
    target_compile_options(xen PRIVATE -stdlib=libstdc++)
    target_link_libraries(xen PRIVATE stdc++)
endif ()

if (XEN_USE_GLEW)
    target_link_libraries(xen PRIVATE GLEW)
endif ()

if (NOT XEN_USE_EMSCRIPTEN) # Emscripten handles GLFW on its own, but still needs the Window to render into
    if (XEN_USE_WINDOW)
        target_link_libraries(xen PRIVATE GLFW)
        message(STATUS "[xen] Windowing ENABLED")
    else ()
        list(
            REMOVE_ITEM
            XEN_FILES

            "${PROJECT_SOURCE_DIR}/src/render/window.cpp"
            "${PROJECT_SOURCE_DIR}/src/render/window.hpp"
            "${PROJECT_SOURCE_DIR}/src/script/lua_window.cpp"
        )

        target_compile_definitions(xen PUBLIC XEN_NO_WINDOW)

        message(STATUS "[xen] Windowing DISABLED")
    endif ()
endif ()

if (XEN_USE_IMGUI)
    target_link_libraries(xen PUBLIC ImGui)
    message(STATUS "[xen] Overlay ENABLED")
else ()
    list(
        REMOVE_ITEM
        XEN_FILES

        "${PROJECT_SOURCE_DIR}/src/render/overlay.cpp"
        "${PROJECT_SOURCE_DIR}/src/render/overlay.hpp"
    )

    target_compile_definitions(xen PUBLIC XEN_NO_OVERLAY)

    message(STATUS "[xen] Overlay DISABLED")
endif ()

if (XEN_USE_LUA)
    target_link_libraries(xen PRIVATE Sol2 Lua)
    message(STATUS "[xen] Lua scripting ENABLED")
else ()
    file(
        GLOB
        XEN_LUA_FILES

        "${PROJECT_SOURCE_DIR}/src/script/*"
    )
    list(REMOVE_ITEM XEN_FILES ${XEN_LUA_FILES})

    target_compile_definitions(xen PUBLIC XEN_NO_LUA)

    message(STATUS "[xen] Lua scripting DISABLED")
endif ()

if (NOT XEN_IS_PLATFORM_MAC AND NOT XEN_USE_EMSCRIPTEN AND XEN_USE_WINDOW)
    # XR currently isn't available with macOS or Emscripten and requires windowing capabilities
    # TODO: requiring windowing currently makes XR unavailable from the GUI editor, as it handles the windowing part on its own

    file(
        GLOB
        XEN_XR_FILES

        "${PROJECT_SOURCE_DIR}/src/xr/*.cpp"
        "${PROJECT_SOURCE_DIR}/src/xr/*.hpp"
    )
    list(APPEND XEN_FILES ${XEN_XR_FILES})

    target_link_libraries(xen PRIVATE OpenXR)
else ()
    list(REMOVE_ITEM XEN_FILES "${PROJECT_SOURCE_DIR}/src/script/lua_xr.cpp")
endif ()

# The target is always linked, as the include files & their macros should be unconditionally available for ease of use; Tracy macros do something only
#  if the expected definition is available, which only is if the related CMake option is set
# It also always compiles the implementation file, but making it conditional would be much more of a hassle. This may change in the future
target_link_libraries(xen PUBLIC Tracy)

target_link_libraries(xen PUBLIC fastgltf simdjson stb libbullet3)

target_sources(xen PRIVATE ${XEN_FILES})

target_precompile_headers(xen PUBLIC prec.hpp)