set(HEADERS
    core.hpp

    debug/log.hpp

    engine/module.hpp
    engine/engine.hpp

    animation/animation_clip.hpp
    animation/animation_data.hpp
    animation/bone.hpp
    animation/pose_animator.hpp

    physics/collision_object.hpp
    physics/force.hpp
    physics/frustum.hpp
    physics/kinematic_character.hpp
    physics/physics.hpp
    physics/ray.hpp
    physics/rigidbody.hpp
    physics/bounding_objects/aabb.hpp
    physics/bounding_objects/bounding_box.hpp
    physics/bounding_objects/bounding_sphere.hpp
    physics/bounding_objects/capsule.hpp
    physics/colliders/collider.hpp
    physics/colliders/capsule_collider.hpp
    physics/colliders/cone_collider.hpp
    physics/colliders/convex_hull_collider.hpp
    physics/colliders/cube_collider.hpp
    physics/colliders/cylinder_collider.hpp
    physics/colliders/heightfield_collider.hpp
    physics/colliders/sphere_collider.hpp

    platform/opengl/buffer.hpp
    platform/opengl/framebuffer.hpp
    platform/opengl/gbuffer.hpp
    platform/opengl/gl_utils.hpp
    platform/opengl/index_buffer.hpp
    platform/opengl/vertex_array.hpp
    platform/opengl/vertex_attribute.hpp
    platform/opengl/vertex_buffer.hpp

    system/monitor.hpp
    system/window.hpp
    system/windows.hpp

    render/render.hpp
    render/ibl/light_probe.hpp
    render/ibl/probe_manager.hpp
    render/ibl/reflection_probe.hpp
    render/lights/light_bindings.hpp
    render/lights/light_system.hpp
    render/mesh/common/cube.hpp
    render/mesh/common/quad.hpp
    render/mesh/common/sphere.hpp
    render/mesh/material.hpp
    render/mesh/mesh.hpp
    render/mesh/model.hpp
    render/renderer/renderpass/deferred/deferred_geometry_pass.hpp
    render/renderer/renderpass/deferred/deferred_light_pass.hpp
    render/renderer/renderpass/forward/forward_light_pass.hpp
    render/renderer/renderpass/forward/forward_probe_pass.hpp
    render/renderer/renderpass/editor_pass.hpp
    render/renderer/renderpass/post_process_pass.hpp
    render/renderer/renderpass/render_pass_type.hpp
    render/renderer/renderpass/render_pass.hpp
    render/renderer/renderpass/shadowmap_pass.hpp
    render/renderer/renderpass/water_pass.hpp
    render/renderer/debug_draw_3d.hpp
    render/renderer/gl_cache.hpp
    render/renderer/renderer.hpp
    render/terrain/terrain.hpp
    render/texture/cubemap.hpp
    render/texture/texture.hpp
    render/water/water_system.hpp
    render/shader.hpp
    render/skybox.hpp

    resource/resources.hpp

    scene/system.hpp
    scene/system_manager.hpp
    scene/entity_manager.hpp
    scene/entity.hpp
    scene/scene.hpp
    scene/scenes.hpp
    scene/debug_camera.hpp
    scene/component.hpp
    scene/components/camera.hpp
    scene/components/hierarchy.hpp
    scene/components/id.hpp
    scene/components/light.hpp
    scene/components/mesh.hpp
    scene/components/pose_animator.hpp
    scene/components/tag.hpp
    scene/components/transform.hpp
    scene/components/water.hpp
    scene/components/fps_player.hpp
    
    math/math.hpp
    math/vector2.hpp
    math/vector2.inl
    math/vector3.hpp
    math/vector3.inl
    math/vector4.hpp
    math/vector4.inl
    math/matrix2.hpp
    math/matrix2.inl
    math/matrix3.hpp
    math/matrix3.inl
    math/matrix4.hpp
    math/matrix4.inl
    math/quaternion.hpp
    math/quaternion.inl
    math/math_types.hpp
    math/math_serialization.hpp

    utils/color.hpp
    utils/elapsed_time.hpp
    utils/time.hpp
    utils/time.inl
    utils/classes.hpp
    utils/enumerate.hpp
    utils/json_factory.hpp
    utils/uuid.hpp
    utils/color.hpp
    utils/concepts.hpp
    utils/shader_loader.hpp
    utils/shader_preprocessor.hpp
    utils/texture_loader.hpp
    utils/timers.hpp
    utils/type_info.hpp
    utils/multithreading/lock_free_queue.hpp

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
    input/axes/scroll_axis.hpp

    prec.hpp
)

set(SOURCES
    debug/log.cpp

    engine/engine.cpp

    animation/animation_clip.cpp
    animation/bone.cpp
    animation/pose_animator.cpp

    physics/collision_object.cpp
    physics/force.cpp
    physics/frustum.cpp
    physics/kinematic_character.cpp
    physics/physics.cpp
    physics/ray.cpp
    physics/rigidbody.cpp
    physics/colliders/collider.cpp
    physics/colliders/capsule_collider.cpp
    physics/colliders/cone_collider.cpp
    physics/colliders/convex_hull_collider.cpp
    physics/colliders/cube_collider.cpp
    physics/colliders/cylinder_collider.cpp
    physics/colliders/heightfield_collider.cpp
    physics/colliders/sphere_collider.cpp

    platform/opengl/buffer.cpp
    platform/opengl/framebuffer.cpp
    platform/opengl/gbuffer.cpp
    platform/opengl/gl_utils.cpp
    platform/opengl/index_buffer.cpp
    platform/opengl/vertex_array.cpp
    platform/opengl/vertex_attribute.cpp
    platform/opengl/vertex_buffer.cpp

    system/monitor.cpp
    system/window.cpp
    system/windows.cpp

    render/render.cpp
    render/ibl/light_probe.cpp
    render/ibl/probe_manager.cpp
    render/ibl/reflection_probe.cpp
    render/lights/light_bindings.cpp
    render/lights/light_system.cpp
    render/mesh/common/cube.hpp
    render/mesh/common/quad.hpp
    render/mesh/common/sphere.hpp
    render/mesh/material.cpp
    render/mesh/mesh.cpp
    render/mesh/model.cpp
    render/renderer/renderpass/deferred/deferred_geometry_pass.cpp
    render/renderer/renderpass/deferred/deferred_light_pass.cpp
    render/renderer/renderpass/forward/forward_light_pass.cpp
    render/renderer/renderpass/forward/forward_probe_pass.cpp
    render/renderer/renderpass/editor_pass.cpp
    render/renderer/renderpass/post_process_pass.cpp
    render/renderer/renderpass/render_pass_type.hpp
    render/renderer/renderpass/render_pass.cpp
    render/renderer/renderpass/shadowmap_pass.cpp
    render/renderer/renderpass/water_pass.cpp
    render/renderer/debug_draw_3d.cpp
    render/renderer/gl_cache.cpp
    render/renderer/renderer.cpp
    render/terrain/terrain.cpp
    render/texture/cubemap.cpp
    render/texture/texture.cpp
    render/water/water_system.cpp
    render/shader.cpp
    render/skybox.cpp
    
    resource/resources.cpp

    scene/system_manager.cpp
    scene/entity.cpp
    scene/scene.cpp
    scene/scenes.cpp
    scene/debug_camera.cpp
    scene/components/transform.cpp
    scene/components/fps_player.cpp

    math/math.cpp
    math/vector2.cpp
    math/vector3.cpp
    math/vector4.cpp
    math/quaternion.cpp

    utils/color.cpp
    utils/elapsed_time.cpp
    utils/shader_loader.cpp
    utils/shader_preprocessor.cpp
    utils/texture_loader.cpp
    utils/timers.cpp
    utils/uuid.cpp

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
    input/axes/scroll_axis.cpp
)

source_group(${SOURCES})
source_group(${HEADERS})

list(APPEND CMAKE_PREFIX_PATH ${CMAKE_SOURCE_DIR}/depends)

add_library(xen ${SOURCES} ${HEADERS})
target_link_libraries(${PROJECT_NAME} ${OPENGL_LIBRARIES} glad glfw glm imgui libbullet3 stb_image assimp rocket magic_enum cereal json EnTT)
target_include_directories(${PROJECT_NAME} PUBLIC src/)

# if(MSVC)
#     get_target_property(charo_dll charo::charo IMPORTED_LOCATION_RELEASE)
#     add_custom_command(TARGET sfec POST_BUILD 
#         COMMAND "${CMAKE_COMMAND}" -E copy 
#         ${charo_dll}
#         ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG> )
# endif()