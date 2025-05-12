#include "entity.hpp"
#include "world.hpp"
#if defined(XEN_USE_AUDIO)
#include "audio/sound.hpp"
#include "data/wav_format.hpp"
#endif
#include "math/angle.hpp"
#include "math/transform/transform.hpp"
#include "render/camera.hpp"
#include "render/light.hpp"
#include "render/render_system.hpp"
#include "render/window.hpp"

#include "demo_utils.hpp"

namespace DemoUtils {
void setup_camera_controls(xen::Entity& camera_entity, xen::Window& window)
{
    auto& camera = camera_entity.get_component<xen::Camera>();
    auto& camera_transform = camera_entity.get_component<xen::Transform>();

    /////////////////////
    // Camera movement //
    /////////////////////

    static float camera_speed = 1.f;

    // Camera speedup
    window.add_key_callback(
        xen::Keyboard::LEFT_SHIFT, [](float /* delta_time */) noexcept { camera_speed = 2.f; }, xen::Input::ONCE,
        []() noexcept { camera_speed = 1.f; }
    );

    window.add_key_callback(xen::Keyboard::SPACE, [&camera_transform](float delta_time) {
        camera_transform.move(xen::Vector3f(0.f, (10.f * delta_time) * camera_speed, 0.f));
    });

    window.add_key_callback(xen::Keyboard::V, [&camera_transform](float delta_time) {
        camera_transform.move(xen::Vector3f(0.f, (-10.f * delta_time) * camera_speed, 0.f));
    });

    window.add_key_callback(xen::Keyboard::W, [&camera_transform, &camera](float delta_time) {
        float const move_value = (-10.f * delta_time) * camera_speed;

        camera_transform.move(xen::Vector3f(0.f, 0.f, move_value));
        camera.set_orthographic_bound(camera.get_orthographic_bound() + move_value);
    });

    window.add_key_callback(xen::Keyboard::S, [&camera_transform, &camera](float delta_time) {
        float const move_value = (10.f * delta_time) * camera_speed;

        camera_transform.move(xen::Vector3f(0.f, 0.f, move_value));
        camera.set_orthographic_bound(camera.get_orthographic_bound() + move_value);
    });

    window.add_key_callback(xen::Keyboard::A, [&camera_transform](float delta_time) {
        camera_transform.move(xen::Vector3f((-10.f * delta_time) * camera_speed, 0.f, 0.f));
    });

    window.add_key_callback(xen::Keyboard::D, [&camera_transform](float delta_time) {
        camera_transform.move(xen::Vector3f((10.f * delta_time) * camera_speed, 0.f, 0.f));
    });

    window.set_mouse_scroll_callback([&camera](double /* xOffset */, double y_offset) {
        float const new_fov_degree =
            std::clamp(xen::Degreesf(camera.get_fov()).value + static_cast<float>(-y_offset) * 2.f, 15.f, 90.f);
        camera.set_fov(xen::Degreesf(new_fov_degree));
    });

    /////////////////////
    // Camera rotation //
    /////////////////////

    // The camera can be rotated while holding the mouse right click
    static bool is_right_clicking = false;

    window.add_mouse_button_callback(
        xen::Mouse::RIGHT_CLICK,
        [&window](float /* delta_time */) {
            is_right_clicking = true;
            window.disable_cursor();
        },
        xen::Input::ONCE,
        [&window]() {
            is_right_clicking = false;
            window.show_cursor();
        }
    );

    window.set_mouse_move_callback([&camera_transform, &window](double x_move, double y_move) {
        if (!is_right_clicking)
            return;

        using namespace xen::Literals;

        // Dividing movement by the window's size to scale between -1 and 1
        camera_transform.rotate(
            -90_deg * static_cast<float>(y_move) / window.get_height(),
            -90_deg * static_cast<float>(x_move) / window.get_width()
        );
    });
}

void setup_mesh_controls(xen::Entity& mesh_entity, xen::Window& window)
{
    auto& mesh_transform = mesh_entity.get_component<xen::Transform>();

    ///////////////////
    // Mesh movement //
    ///////////////////

    window.add_key_callback(xen::Keyboard::T, [&mesh_transform](float delta_time) {
        mesh_transform.move(xen::Vector3f(0.f, 0.f, 10.f * delta_time));
    });
    window.add_key_callback(xen::Keyboard::G, [&mesh_transform](float delta_time) {
        mesh_transform.move(xen::Vector3f(0.f, 0.f, -10.f * delta_time));
    });
    window.add_key_callback(xen::Keyboard::F, [&mesh_transform](float delta_time) {
        mesh_transform.move(xen::Vector3f(-10.f * delta_time, 0.f, 0.f));
    });
    window.add_key_callback(xen::Keyboard::H, [&mesh_transform](float delta_time) {
        mesh_transform.move(xen::Vector3f(10.f * delta_time, 0.f, 0.f));
    });

    //////////////////
    // Mesh scaling //
    //////////////////

    window.add_key_callback(
        xen::Keyboard::X, [&mesh_transform](float /* delta_time */) { mesh_transform.scale(xen::Vector3f(0.5f)); },
        xen::Input::ONCE
    );
    window.add_key_callback(
        xen::Keyboard::C, [&mesh_transform](float /* delta_time */) { mesh_transform.scale(xen::Vector3f(2.f)); },
        xen::Input::ONCE
    );

    ///////////////////
    // Mesh rotation //
    ///////////////////

    using namespace xen::Literals;

    window.add_key_callback(xen::Keyboard::UP, [&mesh_transform](float delta_time) {
        mesh_transform.rotate(xen::Quaternion(xen::Vector3f::Right, 90_deg * delta_time));
    });
    window.add_key_callback(xen::Keyboard::DOWN, [&mesh_transform](float delta_time) {
        mesh_transform.rotate(xen::Quaternion(xen::Vector3f::Right, -90_deg * delta_time));
    });
    window.add_key_callback(xen::Keyboard::LEFT, [&mesh_transform](float delta_time) {
        mesh_transform.rotate(xen::Quaternion(xen::Vector3f::Up, 90_deg * delta_time));
    });
    window.add_key_callback(xen::Keyboard::RIGHT, [&mesh_transform](float delta_time) {
        mesh_transform.rotate(xen::Quaternion(xen::Vector3f::Up, -90_deg * delta_time));
    });
}

void setup_light_controls(xen::Entity& light_entity, xen::RenderSystem const& render_system, xen::Window& window)
{
    auto& light = light_entity.get_component<xen::Light>();
    auto& light_transform = light_entity.get_component<xen::Transform>();

    ////////////////////
    // Light movement //
    ////////////////////

    window.add_key_callback(xen::Keyboard::I, [&light_transform, &render_system](float delta_time) {
        light_transform.translate(xen::Vector3f(0.f, 0.f, -10.f * delta_time));
        render_system.update_lights();
    });

    window.add_key_callback(xen::Keyboard::K, [&light_transform, &render_system](float delta_time) {
        light_transform.translate(xen::Vector3f(0.f, 0.f, 10.f * delta_time));
        render_system.update_lights();
    });

    window.add_key_callback(xen::Keyboard::J, [&light_transform, &render_system](float delta_time) {
        light_transform.translate(xen::Vector3f(-10.f * delta_time, 0.f, 0.f));
        render_system.update_lights();
    });

    window.add_key_callback(xen::Keyboard::L, [&light_transform, &render_system](float delta_time) {
        light_transform.translate(xen::Vector3f(10.f * delta_time, 0.f, 0.f));
        render_system.update_lights();
    });

    //////////////////
    // Light energy //
    //////////////////

    window.add_key_callback(xen::Keyboard::PAGEUP, [&light, &render_system](float delta_time) {
        light.set_energy(light.get_energy() + 1.f * delta_time);
        render_system.update_lights();
    });

    window.add_key_callback(xen::Keyboard::PAGEDOWN, [&light, &render_system](float delta_time) {
        light.set_energy(std::max(0.f, light.get_energy() - 1.f * delta_time));
        render_system.update_lights();
    });
}

void setup_add_light(xen::Transform const& transform, xen::World& world, xen::Window& window)
{
    window.add_mouse_button_callback(
        xen::Mouse::MIDDLE_CLICK,
        [&world, &transform](float /* delta_time */) {
            auto& new_light = world.add_entity_with_component<xen::Light>(
                xen::LightType::POINT, // Type (point light)
                10.f
            ); // Energy
            new_light.add_component<xen::Transform>(transform.get_position());
        },
        xen::Input::ONCE
    );
}

#if defined(XEN_USE_AUDIO)

void setup_sound_controls(const xen::Sound& sound, xen::Window& window)
{
    // Toggling play/pause
    window.add_key_callback(
        xen::Keyboard::NUM0,
        [&sound](float /* delta_time */) noexcept {
            if (sound.is_playing())
                sound.pause();
            else
                sound.play();
        },
        xen::Input::ONCE
    );

    // Stopping the sound
    window.add_key_callback(
        xen::Keyboard::DECIMAL, [&sound](float /* delta_time */) noexcept { sound.stop(); }, xen::Input::ONCE
    );
}

void setup_add_sound(
    xen::Transform const& transform, xen::FilePath const& sound_path, xen::World& world, xen::Window& window
)
{
    // Adding a new sound on the given position
    window.add_key_callback(
        xen::Keyboard::ADD,
        [&world, &sound_path, &transform](float /* delta_time */) {
            xen::Entity& new_sound = world.add_entity_with_component<xen::Sound>(xen::WavFormat::load(sound_path));
            new_sound.add_component<xen::Transform>(transform.get_position());
        },
        xen::Input::ONCE
    );
}

#endif // XEN_USE_AUDIO

void insert_overlay_camera_controls_help(xen::OverlayWindow& overlay_window)
{
    overlay_window.add_label("Press WASD to fly the camera around,");
    overlay_window.add_label("Space/V to go up/down,");
    overlay_window.add_label("& Shift to move faster.");
    overlay_window.add_label("Hold the right mouse button to rotate the camera.");
}

void insert_overlay_culling_option(xen::Window const& window, xen::OverlayWindow& overlay_window)
{
    overlay_window.add_checkbox(
        "Enable face culling", [&window]() { window.enable_face_culling(); },
        [&window]() { window.disable_face_culling(); }, true
    );
}

void insert_overlay_vertical_sync_option(xen::Window const& window, xen::OverlayWindow& overlay_window)
{
#if !defined(USE_OPENGL_ES)
    overlay_window.add_checkbox(
        "Enable vertical sync", [&window]() { window.enable_vertical_sync(); },
        [&window]() { window.disable_vertical_sync(); }, window.recover_vertical_sync_state()
    );
#endif
}

void insert_overlay_frame_speed(xen::OverlayWindow& overlay_window)
{
    overlay_window.add_frame_time("Frame time: %.3f ms/frame"); // Frame time's & FPS counter's texts must be formatted
    overlay_window.add_fps_counter("FPS: %.1f");
}

} // namespace DemoUtils
