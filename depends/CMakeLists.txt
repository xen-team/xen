list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}")

if (NOT XEN_USE_EMSCRIPTEN)
    option(XEN_USE_GLEW "Use GLEW" ON)
    if (XEN_USE_GLEW)
        add_subdirectory(glew)
    endif ()

    option(XEN_USE_WINDOW "Use windowing & input capabilities (with GLFW)" ON)
    if (XEN_USE_WINDOW)
        add_subdirectory(glfw)
    endif ()

    if (NOT XEN_IS_PLATFORM_MAC AND XEN_USE_WINDOW)
        # XR currently isn't available with macOS or Emscripten and requires windowing capabilities
        add_subdirectory(openxr)
    endif ()
endif ()

if (XEN_USE_IMGUI)
    add_subdirectory(imgui)
endif ()

if (XEN_USE_LUA)
    add_subdirectory(lua)
    add_subdirectory(sol2)
endif ()

add_subdirectory(tracy)

if (XEN_USE_PROFILING)
    target_compile_definitions(Tracy PUBLIC TRACY_ENABLE)
endif ()

# add_subdirectory(catch)
add_subdirectory(simdjson)
add_subdirectory(fastgltf)
add_subdirectory(stb)
add_subdirectory(bullet3)