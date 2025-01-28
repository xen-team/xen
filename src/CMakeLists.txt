set(HEADERS
    core.hpp

    debug/log.hpp

    engine/module.hpp
    engine/engine.hpp

    system/monitor.hpp
    system/window.hpp
    system/windows.hpp

    units/math.hpp
    units/vector2.hpp
    units/vector2.inl
    units/elapsed_time.hpp
    units/time.hpp
    units/time.inl

    utils/classes.hpp
    utils/enumerate.hpp
    utils/json_factory.hpp

    files/json.hpp

    input/input_axis.hpp
    input/input_button.hpp
    input/input_scheme.hpp
    input/joysticks.hpp
    input/inputs.hpp
    input/input_delay.hpp
    input/buttons/axis_button.hpp
    input/buttons/combo_button.hpp
    input/buttons/joystick_button.hpp
    input/buttons/keyboard_button.hpp
    input/buttons/mouse_button.hpp
    input/axes/button_axis.hpp
    input/axes/combo_axis.hpp
    input/axes/joystick_axis.hpp
    input/axes/joystick_hat.hpp
    input/axes/mouse_axis.hpp

    prec.hpp
)

set(SOURCES
    debug/log.cpp

    engine/engine.cpp

    system/monitor.cpp
    system/window.cpp
    system/windows.cpp

    units/math.cpp
    units/vector2.cpp
    units/elapsed_time.cpp

    files/json.cpp

    input/input_scheme.cpp
    input/joysticks.cpp
    input/inputs.cpp
    input/input_delay.cpp
    input/buttons/axis_button.cpp
    input/buttons/combo_button.cpp
    input/buttons/joystick_button.cpp
    input/buttons/keyboard_button.cpp
    input/buttons/mouse_button.cpp
    input/axes/button_axis.cpp
    input/axes/combo_axis.cpp
    input/axes/joystick_axis.cpp
    input/axes/joystick_hat.cpp
    input/axes/mouse_axis.cpp
)

source_group(${SOURCES})
source_group(${HEADERS})

list(APPEND CMAKE_PREFIX_PATH ${CMAKE_SOURCE_DIR}/depends)

add_library(xen ${SOURCES} ${HEADERS})
target_link_libraries(${PROJECT_NAME} ${OPENGL_LIBRARIES} glad glfw glm imgui stb_image rocket magic_enum bitsery json)
target_include_directories(${PROJECT_NAME} PUBLIC src/)

# if(MSVC)
#     get_target_property(charo_dll charo::charo IMPORTED_LOCATION_RELEASE)
#     add_custom_command(TARGET sfec POST_BUILD 
#         COMMAND "${CMAKE_COMMAND}" -E copy 
#         ${charo_dll}
#         ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG> )
# endif()