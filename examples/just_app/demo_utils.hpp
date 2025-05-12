#pragma once

namespace xen {
class Entity;
class FilePath;
class OverlayWindow;
class RenderSystem;
class Sound;
class Transform;
class Window;
class World;

}

namespace DemoUtils {
/// Adds callbacks onto a window to allow moving & rotating a camera.
/// \param camera_entity Camera for which to add controls.
/// \param window Window on which to add the callbacks.
void setup_camera_controls(xen::Entity& camera_entity, xen::Window& window);

/// Adds callbacks onto a window to allow moving, scaling & rotating a mesh.
/// \param mesh_entity Mesh for which to add controls.
/// \param window Window on which to add the callbacks.
void setup_mesh_controls(xen::Entity& mesh_entity, xen::Window& window);

/// Adds callbacks onto a window to allow moving a light & varying its energy.
/// \param light_entity Light for which to add controls.
/// \param window Window on which to add the callbacks.
void setup_light_controls(xen::Entity& light_entity, xen::RenderSystem const& render_system, xen::Window& window);

/// Adds a callback onto a window to allow adding a light on a transform's position.
/// \param transform Transform from which to apply the position to the new light.
/// \param world World in which to add the new light.
/// \param window Window on which to add the callback.
void setup_add_light(xen::Transform const& transform, xen::World& world, xen::Window& window);

/// Adds callbacks onto a window to allow playing, pausing & stopping a sound.
/// \param sound Sound for which to add controls.
/// \param window Window on which to add the callbacks.
void setup_sound_controls(xen::Sound const& sound, xen::Window& window);

/// Adds a callback onto a window to allow adding a sound on a transform's position.
/// \param transform Transform from which to apply the position to the new sound.
/// \param sound_path File path to the sound to be loaded.
/// \param world World in which to add the new sound.
/// \param window Window on which to add the callback.
void setup_add_sound(
    xen::Transform const& transform, xen::FilePath const& sound_path, xen::World& world, xen::Window& window
);

/// Inserts labels onto the overlay to display an help message about how to move around the scene.
/// \param overlay_window Overlay window on which to insert the labels.
void insert_overlay_camera_controls_help(xen::OverlayWindow& overlay_window);

/// Inserts a checkbox onto the overlay to allow toggling face culling's state.
/// \param window Window on which to toggle face culling.
/// \param overlay_window Overlay window on which to insert the checkbox.
void insert_overlay_culling_option(xen::Window const& window, xen::OverlayWindow& overlay_window);

/// Inserts a checkbox onto the overlay to allow toggling V-sync's state. Has no effect when using OpenGL ES (vertical
/// sync is always enabled).
/// \param window Window on which to toggle V-sync.
/// \param overlay_window Overlay window on which to insert the checkbox.
void insert_overlay_vertical_sync_option(xen::Window const& window, xen::OverlayWindow& overlay_window);

/// Inserts dynamic labels onto the overlay to display frame time & FPS.
/// \param overlay_window Overlay window on which to insert the labels.
void insert_overlay_frame_speed(xen::OverlayWindow& overlay_window);
}
