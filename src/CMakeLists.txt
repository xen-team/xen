list(APPEND xen_header_files
    debug/log.hpp

    engine/module.hpp
    engine/engine.hpp

    system/monitor.hpp
    system/window.hpp
    system/window_manager.hpp

    units/math.hpp
    units/vector2.hpp
    units/vector2.inl
    units/elapsed_time.hpp
    units/time.hpp
    units/time.inl

    utils/classes.hpp
)

list(APPEND xen_source_files
    main.cpp

    debug/log.cpp

    engine/engine.cpp

    system/monitor.cpp
    system/window.cpp
    system/window_manager.cpp

    units/math.cpp
    units/vector2.cpp
    units/elapsed_time.cpp
)

list(APPEND CMAKE_PREFIX_PATH ${CMAKE_SOURCE_DIR}/depends)

# find_package(charo CONFIG REQUIRED)

add_executable(${PROJECT_NAME} ${xen_header_files} ${xen_source_files})

target_link_libraries(${PROJECT_NAME} ${OPENGL_LIBRARIES} glad glfw glm imgui stb_image rocket magic_enum)

target_include_directories(${PROJECT_NAME} PUBLIC src/)

# target_link_libraries(sfec PRIVATE charo::charo)

# if(MSVC)
#     get_target_property(charo_dll charo::charo IMPORTED_LOCATION_RELEASE)
#     add_custom_command(TARGET sfec POST_BUILD 
#         COMMAND "${CMAKE_COMMAND}" -E copy 
#         ${charo_dll}
#         ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG> )
# endif()