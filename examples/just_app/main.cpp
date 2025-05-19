// #include "animation/animation_clip.hpp"
// #include "engine/engine.hpp"
// #include "input/axes/button_axis.hpp"
// #include "input/axes/combo_axis.hpp"
// #include "input/axes/mouse_axis.hpp"
// #include "input/axes/scroll_axis.hpp"
// #include "input/buttons/mouse_button.hpp"
// #include "physics/colliders/capsule_collider.hpp"
// #include "physics/kinematic_character.hpp"
// #include "physics/rigidbody.hpp"
// #include "render/mesh/common/cube.hpp"
// #include "scene/components/fps_player.hpp"
// #include "physics/kinematic_character.hpp"
// #include "scene/components/light.hpp"
// #include "scene/components/mesh.hpp"
// #include "scene/components/pose_animator.hpp"
// #include "scene/components/water.hpp"
// #include "system/windows.hpp"
// #include "scene/scenes.hpp"
// #include "input/inputs.hpp"
// #include "input/buttons/combo_button.hpp"
// #include "input/buttons/keyboard_button.hpp"
// #include <glm/glm.hpp>
// #include <resource/resources.hpp>
// #include <utils/shader_loader.hpp>

#include "audio/audio_data.hpp"
#include "script/lua_script.hpp"
#include "script/script_system.hpp"
using namespace xen;

namespace {

constexpr unsigned int baseWindowWidth = 375;
constexpr xen::Vector2f overlayAudioSize(baseWindowWidth, 100.f);
constexpr xen::Vector2f overlaySoundSize(baseWindowWidth, 220.f);
constexpr xen::Vector2f overlayMicSize(baseWindowWidth, 330.f);
constexpr auto baseWindowHeight = static_cast<unsigned int>(overlayAudioSize.y + overlaySoundSize.y + overlayMicSize.y);
#if !defined(RAZ_PLATFORM_EMSCRIPTEN)
constexpr xen::Vector2f overlayReverbSize(487.f, 655.f);
constexpr xen::Vector2f overlayChorusSize(320.f, 170.f);
constexpr xen::Vector2f overlayDistortionSize(350.f, 147.f);
constexpr xen::Vector2f overlayEchoSize(320.f, 147.f);
#endif

constexpr xen::AudioFormat recover_format(bool isStereo, uint8_t bitDepth)
{
    return (
        isStereo ? (bitDepth == 8 ? xen::AudioFormat::STEREO_U8 : xen::AudioFormat::STEREO_I16) :
                   (bitDepth == 8 ? xen::AudioFormat::MONO_U8 : xen::AudioFormat::MONO_I16)
    );
}

} // namespace

// #include "RaZ/RaZ.hpp"

#include "demo_utils.hpp"
#include "xen2.hpp"

using namespace std::literals;

int main()
{
    try {
        ////////////////////
        // Initialization //
        ////////////////////

        xen::Application app;
        xen::World& world = app.add_world(10);

        ///////////////
        // Rendering //
        ///////////////

        auto& render_system =
            world.add_system<xen::RenderSystem>(Vector2ui(1920u, 1080u), "xen", xen::WindowSetting::DEFAULT, 2);

        render_system.set_cubemap(xen::Cubemap(
            xen::ImageFormat::load("assets/skyboxes/clouds_right.png"),
            xen::ImageFormat::load("assets/skyboxes/clouds_left.png"),
            xen::ImageFormat::load("assets/skyboxes/clouds_top.png"),
            xen::ImageFormat::load("assets/skyboxes/clouds_bottom.png"),
            xen::ImageFormat::load("assets/skyboxes/clouds_front.png"),
            xen::ImageFormat::load("assets/skyboxes/clouds_back.png")
        ));

        xen::Window& window = render_system.get_window();

        window.set_icon(xen::ImageFormat::load("assets/icons/xen_logo_128.png"));

        window.add_key_callback(xen::Keyboard::F5, [&render_system](float /* delta_time */) {
            render_system.update_shaders();
        });

        window.add_key_callback(
            xen::Keyboard::F11,
            [&window](float) noexcept {
                static bool isFullScreen = false;

                if (isFullScreen)
                    window.make_windowed();
                else
                    window.make_fullscreen();

                isFullScreen = !isFullScreen;
            },
            xen::Input::ONCE
        );

        // Allowing to quit the application by pressing the Escape key
        window.add_key_callback(xen::Keyboard::ESCAPE, [&app](float /* delta_time */) noexcept { app.quit(); });
        // Quitting the application when the close button is clicked
        window.set_close_callback([&app]() noexcept { app.quit(); });

        //////////////////
        // Post effects //
        //////////////////

        xen::RenderGraph& renderGraph = render_system.get_render_graph();
        xen::RenderPass& geometryPass = render_system.get_geometry_pass();

        // For demonstration purposes, a buffer is created here for each process' output. However, this is far from
        // ideal;
        //   always reuse buffers across passes whenever you can, as it may save a lot of memory
        // Note though that a buffer cannot be set as both read & write in the same pass or process
        auto const depthBuffer = xen::Texture2D::create(render_system.get_scene_size(), xen::TextureColorspace::DEPTH);
        auto const colorBuffer = xen::Texture2D::create(render_system.get_scene_size(), xen::TextureColorspace::RGB);
        auto const gradientBuffer = xen::Texture2D::create(render_system.get_scene_size(), xen::TextureColorspace::RGB);
        auto const gradDirBuffer = xen::Texture2D::create(render_system.get_scene_size(), xen::TextureColorspace::RGB);
        auto const edgeBuffer = xen::Texture2D::create(render_system.get_scene_size(), xen::TextureColorspace::GRAY);
        auto const edgeBlendBuffer =
            xen::Texture2D::create(render_system.get_scene_size(), xen::TextureColorspace::RGB);
        auto const chromAberrBuffer =
            xen::Texture2D::create(render_system.get_scene_size(), xen::TextureColorspace::RGB);
        auto const blurredBuffer = xen::Texture2D::create(render_system.get_scene_size(), xen::TextureColorspace::RGB);
        auto const vignetteBuffer = xen::Texture2D::create(render_system.get_scene_size(), xen::TextureColorspace::RGB);
        auto const filmGrainBuffer =
            xen::Texture2D::create(render_system.get_scene_size(), xen::TextureColorspace::RGB);

#if !defined(USE_OPENGL_ES)
        if (xen::Renderer::check_version(4, 3)) {
            xen::Renderer::set_label(xen::RenderObjectType::TEXTURE, depthBuffer->get_index(), "Depth buffer");
            xen::Renderer::set_label(xen::RenderObjectType::TEXTURE, colorBuffer->get_index(), "Color buffer");
            xen::Renderer::set_label(xen::RenderObjectType::TEXTURE, gradientBuffer->get_index(), "Gradient buffer");
            xen::Renderer::set_label(
                xen::RenderObjectType::TEXTURE, gradDirBuffer->get_index(), "Gradient direction buffer"
            );
            xen::Renderer::set_label(xen::RenderObjectType::TEXTURE, edgeBuffer->get_index(), "Edge buffer");
            xen::Renderer::set_label(xen::RenderObjectType::TEXTURE, edgeBlendBuffer->get_index(), "Edge blend buffer");
            xen::Renderer::set_label(
                xen::RenderObjectType::TEXTURE, chromAberrBuffer->get_index(), "Chrom. aberr. buffer"
            );
            xen::Renderer::set_label(xen::RenderObjectType::TEXTURE, blurredBuffer->get_index(), "Blurred buffer");
            xen::Renderer::set_label(xen::RenderObjectType::TEXTURE, vignetteBuffer->get_index(), "Vignette buffer");
            xen::Renderer::set_label(xen::RenderObjectType::TEXTURE, filmGrainBuffer->get_index(), "Film grain buffer");
        }
#endif

        geometryPass.set_write_depth_texture(depthBuffer); // A depth buffer is always needed
        geometryPass.add_write_color_texture(colorBuffer, 0);

        // Sobel

        auto& sobel = renderGraph.add_render_process<xen::SobelFilter>();
        sobel.set_input_buffer(colorBuffer);
        sobel.set_output_gradient_buffer(gradientBuffer);
        sobel.set_output_gradient_direction_buffer(gradDirBuffer);
        sobel.add_parent(geometryPass);

        // Canny

        // Note that in an ordinary Canny edge detection process, the input of the gradient information computing pass
        // (here Sobel)
        //  should be applied a gaussian blur to attempt removing high frequencies (small details). It's not done here
        //  to avoid computing another blur, as one already is further below

        auto& canny = renderGraph.add_render_process<xen::CannyFilter>();
        canny.set_input_gradient_buffer(gradientBuffer);
        canny.set_input_gradient_direction_buffer(gradDirBuffer);
        canny.set_output_buffer(edgeBuffer);
        canny.add_parent(sobel);

        // Edge blend

        auto& edgeBlend = renderGraph.add_node(xen::FragmentShader::load_from_source(R"(
      in vec2 fragTexcoords;

      uniform sampler2D uniColorBuffer;
      uniform sampler2D uniEdgeBuffer;
      uniform float uniBlendFactor;

      layout(location = 0) out vec4 fragColor;

      void main() {
        vec3 origColor = texture(uniColorBuffer, fragTexcoords).rgb;
        float edge     = texture(uniEdgeBuffer, fragTexcoords).r;

        vec3 blendedColor = origColor * (1.0 - edge);

        vec3 finalColor = mix(origColor, blendedColor, uniBlendFactor);
        fragColor       = vec4(finalColor, 1.0);
      }
    )"));
        edgeBlend.get_program().set_attribute(0.f, "uniBlendFactor");
        edgeBlend.get_program().send_attributes();
        edgeBlend.add_read_texture(colorBuffer, "uniColorBuffer");
        edgeBlend.add_read_texture(edgeBuffer, "uniEdgeBuffer");
        edgeBlend.add_write_color_texture(edgeBlendBuffer, 0);
        canny.add_child(edgeBlend);

        // Chromatic aberration

        auto& chromaticAberration = renderGraph.add_render_process<xen::ChromaticAberration>();
        chromaticAberration.set_input_buffer(edgeBlendBuffer);
        chromaticAberration.set_output_buffer(chromAberrBuffer);
        chromaticAberration.add_parent(edgeBlend);

        // Blur

        auto& boxBlur = renderGraph.add_render_process<xen::BoxBlur>();
        boxBlur.set_input_buffer(chromAberrBuffer);
        boxBlur.set_output_buffer(blurredBuffer);
        boxBlur.add_parent(chromaticAberration);

        // Vignette

        auto& vignette = renderGraph.add_render_process<xen::Vignette>();
        vignette.set_input_buffer(blurredBuffer);
        vignette.set_output_buffer(vignetteBuffer);
        vignette.add_parent(boxBlur);

        // Film grain

        auto& filmGrain = renderGraph.add_render_process<xen::FilmGrain>();
        filmGrain.set_input_buffer(vignetteBuffer);
        filmGrain.set_output_buffer(filmGrainBuffer);
        filmGrain.add_parent(vignette);

        // Pixelization

        auto& pixelization = renderGraph.add_render_process<xen::Pixelization>();
        pixelization.set_input_buffer(filmGrainBuffer);
        pixelization.add_parent(filmGrain);

        ////////////
        // Camera //
        ////////////

        auto& physics = world.add_system<xen::PhysicsSystem>();

        xen::Entity& camera = world.add_entity();
        auto& cameraComp = camera.add_component<xen::Camera>(render_system.get_scene_size());
        auto& cameraTrans = camera.add_component<xen::Transform>(xen::Vector3f(0.f, 5.f, 0.f));
        // camera.add_component<xen::RigidBody>(3.f, 0.75f);
        // The collider's position is always relative to the object
        // We will set a position to 0 in all the following colliders, since we want their shape centered on the object
        // camera.add_component<xen::Collider>(xen::Sphere(xen::Vector3f(0.f), 1.f));

        DemoUtils::setup_camera_controls(camera, window);

        //////////
        // Mesh //
        //////////

        xen::Entity& mesh = world.add_entity();

        auto [meshData, meshRenderData] = xen::MeshFormat::load("assets/meshes/shield.obj");

        xen::Entity& sponza = world.add_entity();
        auto& sponzaRenderComp =
            sponza.add_component<xen::MeshRenderer>(xen::ObjFormat::load("assets/meshes/crytek_sponza.obj").second);
        sponza.add_component<xen::Transform>(
            xen::Vector3f(0.f, -3.f, 0.f), xen::Quaternion(xen::Vector3f::Up, Degreesld(90)), xen::Vector3f(0.04f)
        );  

        // xen::Entity& floor = world.add_entity_with_component<xen::Transform>(xen::Vector3f(0.f, -4.f, 0.f));
        // auto& floorMesh = floor.add_component<xen::Mesh>(xen::Plane(0.f), 100.f, 10.f);
        // floor.add_component<xen::MeshRenderer>(floorMesh);
        // floor.add_component<xen::Collider>(xen::Plane(0.f));

        xen::Entity& shotgun = world.add_entity();
        shotgun.add_component<xen::MeshRenderer>(xen::ObjFormat::load("assets/meshes/PumpActionShotgunShort.obj").second
        );
        shotgun.add_component<xen::Transform>(xen::Vector3f(5.f, 0.f, 0.f));

        auto& meshComp = mesh.add_component<xen::Mesh>(std::move(meshData)); // Useful if you need to keep the geometry
        auto& meshRenderComp =
            mesh.add_component<xen::MeshRenderer>(std::move(meshRenderData)); // Useful if you need to render the mesh

        auto& meshTrans = mesh.add_component<xen::Transform>();
        meshTrans.scale(xen::Vector3f(0.2f));

        window.add_key_callback(
            xen::Keyboard::R, [&meshRenderComp](float /* delta_time */) noexcept { meshRenderComp.disable(); },
            xen::Input::ONCE, [&meshRenderComp]() noexcept { meshRenderComp.enable(); }
        );

        DemoUtils::setup_mesh_controls(mesh, window);

        ///////////
        // Light //
        ///////////

        xen::Entity& light = world.add_entity();
        /*auto& lightComp = light.add_component<xen::Light>(xen::LightType::POINT,    // Type
                                                         1.f,                      // Energy
                                                         xen::ColorPreset::White); // Color*/
        auto& lightComp = light.add_component<xen::Light>(
            xen::LightType::DIRECTIONAL, // Type
            -Vector3f::Front,            // Direction
            1.f,                         // Energy
            xen::Color::White
        ); // Color
        auto& lightTrans = light.add_component<xen::Transform>(Vector3f(0.f, 1.f, 0.f));

        DemoUtils::setup_light_controls(light, render_system, window);
        DemoUtils::setup_add_light(cameraTrans, world, window);

        ///////////////
        // Scripting //
        ///////////////

#if !defined(XEN_NO_LUA)
        constexpr std::string_view luaScriptSource = R"(
local rot_angle = Radiansf.new(Degreesf.new(20))

function setup()
    -- 'this' always represents the entity containing the script\
    this:get_transform().rotation = Quaternion.new(Vector3f.Up, -rot_angle)
end

function update(time_info)
    local angle = rot_angle * math.sin(time_info.global_time) * time_info.delta_time
    this:get_transform():rotate(Quaternion.new(Vector3f.Up, angle))
    this:get_transform():move(Vector3f.new(math.sin(time_info.global_time) * 0.01, 0, 0))
end)";

        // local rot_angle = Degreesf.new(20)
        // this:get_transform().rotation = Quaternion.new(-rot_angle, Vector3f.Up)
        // local angle = rot_angle * math.sin(time_info.global_time) * time_info.delta_time
        // this:get_transform():rotate(Quaternion.new(angle, Vector3f.Up))

        world.add_system<xen::ScriptSystem>();

        auto& luaScript = mesh.add_component<xen::LuaScript>(luaScriptSource.data());
#endif

        ///////////
        // Audio //
        ///////////

        // #if defined(XEN_USE_AUDIO)
        //         auto& audio = world.add_system<xen::AudioSystem>();

        //         auto& meshSound =
        //             mesh.add_component<xen::Sound>(xen::WavFormat::load(XEN_ROOT "assets/sounds/wave_seagulls.wav"));
        //         meshSound.set_repeat(true);
        //         meshSound.set_gain(0.f);
        //         meshSound.play();

        //         // Adding a Listener component, so that the sound's volume decays over the camera's distance to the
        //         source
        //         // Passing its initial position and/or orientation is optional, since they will be sent every time
        //         the camera
        //         // moves,
        //         //  but recommended so that the sound instantly takes that information into account
        //         camera.add_component<xen::Listener>(
        //             cameraTrans.get_position(), xen::Matrix3(cameraTrans.get_rotation().to_rotation_matrix())
        //         );

        //         DemoUtils::setup_sound_controls(meshSound, window);
        //         DemoUtils::setup_add_sound(cameraTrans, XEN_ROOT "assets/sounds/wave_seagulls.wav", world, window);
        //         camera.add_component<xen::Listener>(
        //             cameraTrans.get_position(), xen::Matrix3(cameraTrans.get_rotation().to_rotation_matrix())
        //         );

        // #endif // XEN_USE_AUDIO

        /////////////
        // Overlay //
        /////////////

#if !defined(XEN_NO_OVERLAY)
        xen::OverlayWindow& overlay = window.get_overlay().add_window("xen - Full demo", xen::Vector2f(-1.f));

        DemoUtils::insert_overlay_camera_controls_help(overlay);
        overlay.add_label("Press F11 to toggle fullscreen.");

        overlay.add_separator();

        DemoUtils::insert_overlay_culling_option(window, overlay);
        DemoUtils::insert_overlay_vertical_sync_option(window, overlay);

#if !defined(XEN_NO_LUA)
        constexpr float overlayScriptWidth = 600.f;
        constexpr float overlayScriptSourceHeight = 225.f;
        constexpr float overlayScriptHeight = overlayScriptSourceHeight + 60.f;

        xen::OverlayWindow& overlayScript = window.get_overlay().add_window(
            "Edit script", xen::Vector2f(overlayScriptWidth, overlayScriptHeight),
            xen::Vector2f(static_cast<float>(window.get_width()) - overlayScriptWidth, 0.f)
        );
        overlayScript.disable();

        xen::OverlayTextArea& scriptTextArea = overlayScript.add_text_area(
            "Lua script", [](std::string const&) noexcept {}, luaScriptSource.data() + 1, overlayScriptSourceHeight
        );

        overlayScript.add_button("Apply", [&]() {
            try {
                // Running a dummy script, checking that it's syntactically correct
                xen::LuaScript testScript(scriptTextArea.get_label());
                testScript.register_entity(mesh, "this");
                testScript.update({});

                luaScript.load_code(scriptTextArea.get_label());
            }
            catch (std::exception const& ex) {
                xen::Log::error("Failed to reload the Lua script:\n" + std::string(ex.what()));
            }
        });

        overlay.add_button("Edit script", [&overlayScript]() noexcept {
            overlayScript.enable(!overlayScript.is_enabled());
        });
#endif

        overlay.add_separator();

        // #if defined(XEN_USE_AUDIO)
        // overlay.add_slider(
        // "Sound volume", [&meshSound](float value) noexcept { meshSound.set_gain(value); }, Vector2f{0.f, 1.f}, 0.f
        // );
        // #endif // XEN_USE_AUDIO

        overlay.add_slider(
            "Edge enhanc. strength",
            [&edgeBlend](float value) {
                edgeBlend.get_program().set_attribute(value, "uniBlendFactor");
                edgeBlend.get_program().send_attributes();
            },
            Vector2f{0.f, 1.f}, 0.f
        );

        overlay.add_slider(
            "Canny lower bound", [&canny](float value) { canny.set_lower_bound(value); }, Vector2f{0.f, 1.f}, 0.1f
        );

        overlay.add_slider(
            "Canny upper bound", [&canny](float value) { canny.set_upper_bound(value); }, Vector2f{0.f, 1.f}, 0.3f
        );

        overlay.add_slider(
            "Chrom. aberr. strength", [&chromaticAberration](float value) { chromaticAberration.set_strength(value); },
            Vector2f{-15.f, 15.f}, 0.f
        );

        overlay.add_slider(
            "Blur strength", [&boxBlur](float value) { boxBlur.set_strength(static_cast<unsigned int>(value)); },
            Vector2f{1.f, 16.f}, 1.f
        );

        overlay.add_slider(
            "Vignette strength", [&vignette](float value) { vignette.set_strength(value); }, Vector2f{0.f, 1.f}, 0.25f
        );

        overlay.add_slider(
            "Vignette opacity", [&vignette](float value) { vignette.set_opacity(value); }, Vector2f{0.f, 1.f}, 1.f
        );

        overlay.add_color_picker(
            "Vignette color", [&vignette](xen::Color const& color) { vignette.set_color(color); }, xen::Color::Black
        );

        overlay.add_slider(
            "Film grain strength", [&filmGrain](float value) { filmGrain.set_strength(value); }, Vector2f{0.f, 1.f},
            0.05f
        );

        overlay.add_slider(
            "Pixelization strength", [&pixelization](float value) { pixelization.set_strength(value); },
            Vector2f{0.f, 1.f}, 0.f
        );

#if !defined(USE_OPENGL_ES) // GPU timing capabilities are not available with OpenGL ES
        overlay.add_separator();

        xen::OverlayPlot& plot = overlay.add_plot("Profiler", 100, {}, "Time (ms)", Vector2f{0.f, 100.f}, false, 200.f);
        xen::OverlayPlotEntry& geomPlot = plot.add_entry("Geometry");
        xen::OverlayPlotEntry& sobelPlot = plot.add_entry("Sobel");
        xen::OverlayPlotEntry& cannyPlot = plot.add_entry("Canny");
        xen::OverlayPlotEntry& chromAberrPlot = plot.add_entry("Chrom. aberr.");
        xen::OverlayPlotEntry& blurPlot = plot.add_entry("Blur");
        xen::OverlayPlotEntry& vignettePlot = plot.add_entry("Vignette");
        xen::OverlayPlotEntry& filmGrainPlot = plot.add_entry("Film grain");
        xen::OverlayPlotEntry& pixelizationPlot = plot.add_entry("Pixelization");
#endif // USE_OPENGL_ES

        overlay.add_separator();

        DemoUtils::insert_overlay_frame_speed(overlay);
#endif // XEN_NO_OVERLAY

        ///////////
        // Audio //
        ///////////

        static xen::AudioData const knockAudio = xen::WavFormat::load("assets/sounds/knock.wav");
        static xen::AudioData const waveSeagullsAudio = xen::WavFormat::load("assets/sounds/plankton-augh.wav");

        auto& audio = world.add_system<xen::AudioSystem>();

        // The Listener entity requires a Transform component
        // auto& listener = world.add_entity_with_component<xen::Transform>().addComponent<xen::Listener>();

        //         DemoUtils::setup_add_sound(cameraTrans, XEN_ROOT "assets/sounds/wave_seagulls.wav", world, window);
        auto& listener = camera.add_component<xen::Listener>(
            cameraTrans.get_position(), xen::Matrix3(cameraTrans.get_rotation().to_rotation_matrix())
        );

        xen::Entity& sound = world.add_entity();
        auto& soundTrans = meshTrans;
        auto& soundComp = sound.add_component<xen::Sound>(knockAudio);

        xen::Microphone microphone(xen::AudioFormat::MONO_U8, 16000, 1.f);

        // Sound effects are not (yet?) available with Emscripten's own OpenAL implementation
#if !defined(RAZ_PLATFORM_EMSCRIPTEN)
        xen::SoundEffect reverb;
        xen::ReverberationParams reverbParams{};
        reverb.load(reverbParams);

        xen::SoundEffect chorus;
        xen::ChorusParams chorusParams{};
        chorus.load(chorusParams);

        xen::SoundEffect distortion;
        xen::DistortionParams distortionParams{};
        distortion.load(distortionParams);

        xen::SoundEffect echo;
        xen::EchoParams echoParams{};
        echo.load(echoParams);

        xen::SoundEffect* enabledEffect = nullptr;

        xen::SoundEffectSlot effectSlot;
#endif

        /////////////
        // Overlay //
        /////////////

        bool isRepeating = false;

        std::string captureDevice;
        bool isCapturing = false;
        bool isCaptureStereo = false;
        uint8_t captureBitDepth = 8;

        float const audio_overlays_xpos = window.get_size().x - overlayAudioSize.x;

        xen::OverlayWindow& overlayAudio =
            window.get_overlay().add_window("Audio Demo", overlayAudioSize, Vector2f(audio_overlays_xpos, 0.f));

#if !defined(RAZ_PLATFORM_EMSCRIPTEN)
        overlayAudio.add_dropdown(
            "Output device", xen::AudioSystem::recover_devices(),
            [&](const std::string& name, std::size_t) {
                float const listenerGain = listener.recover_gain();
                float const soundGain = soundComp.recover_gain();
                float const soundPitch = soundComp.recover_pitch();

                audio.open_device(name);

                listener.set_gain(listenerGain);

                soundComp.init();
                soundComp.set_repeat(isRepeating);
                soundComp.set_gain(soundGain);
                soundComp.set_pitch(soundPitch);

                reverb.init();
                reverb.load(reverbParams);

                chorus.init();
                chorus.load(chorusParams);

                distortion.init();
                distortion.load(distortionParams);

                echo.init();
                echo.load(echoParams);

                effectSlot.init();

                if (enabledEffect) {
                    effectSlot.load_effect(*enabledEffect);
                    soundComp.link_slot(effectSlot);
                }
            }
        );

        overlayAudio.add_dropdown(
            "Input device", xen::Microphone::recover_devices(),
            [&captureDevice, &microphone, &isCaptureStereo, &captureBitDepth](std::string const& name, std::size_t) {
                captureDevice = name;
                microphone.open_device(recover_format(isCaptureStereo, captureBitDepth), 16000, 1.f, captureDevice);
            }
        );
#else
        overlayAudio.addLabel("Output & input devices cannot be changed with Emscripten\n"
                              "Reload the page if you changed them externally");
#endif

        overlayAudio.add_slider(
            "Listener gain", [&listener](float val) noexcept { listener.set_gain(val); }, Vector2f(0.f, 1.f), 1.f
        );

        ///////////
        // Sound //
        ///////////

        xen::AudioData captureData;
        xen::AudioData captureData2;

        auto& captureSound = mesh.add_component<xen::Sound>();

        xen::OverlayWindow& overlaySound = window.get_overlay().add_window(
            "Sound params", overlaySoundSize, xen::Vector2f(audio_overlays_xpos, overlayAudioSize.y)
        );

        overlaySound.add_dropdown("Sound", {"Knock", "Plankton"}, [&](std::string const&, std::size_t i) {
            float const gain = soundComp.recover_gain();
            float const pitch = soundComp.recover_pitch();

            switch (i) {
            case 0:
            default:
                soundComp.load(knockAudio);
                break;

            case 1:
                soundComp.load(waveSeagullsAudio);
                break;
            }

            soundComp.set_repeat(isRepeating);
            soundComp.set_gain(gain);
            soundComp.set_pitch(pitch);

#if !defined(RAZ_PLATFORM_EMSCRIPTEN)
            if (enabledEffect)
                soundComp.link_slot(effectSlot);
#endif
        });

        overlaySound.add_button("Play sound", [&soundComp]() noexcept { soundComp.play(); });

        overlaySound.add_button("Pause sound", [&soundComp]() noexcept { soundComp.pause(); });

        overlaySound.add_checkbox(
            "Repeat",
            [&isRepeating, &soundComp]() noexcept {
                isRepeating = true;
                soundComp.set_repeat(isRepeating);
            },
            [&isRepeating, &soundComp]() noexcept {
                isRepeating = false;
                soundComp.set_repeat(isRepeating);
            },
            isRepeating
        );

        overlaySound.add_slider(
            "Sound gain", [&soundComp](float val) noexcept { soundComp.set_gain(val); }, Vector2f(0.f, 1.f), 1.f
        );

        overlaySound.add_slider(
            "Sound pitch", [&soundComp](float val) noexcept { soundComp.set_pitch(val); }, Vector2f(0.f, 1.f), 1.f
        );

        ////////////////
        // Microphone //
        ////////////////

        xen::OverlayWindow& overlayMic = window.get_overlay().add_window(
            "Microphone params", overlayMicSize,
            xen::Vector2f(audio_overlays_xpos, overlayAudioSize.y + overlaySoundSize.y)
        );

        overlayMic.add_button("Start capture", [&isCapturing, &microphone]() noexcept {
            isCapturing = true;
            microphone.start();
        });

        overlayMic.add_button("Stop capture", [&isCapturing, &microphone]() noexcept {
            isCapturing = false;
            microphone.stop();
        });

        overlayMic.add_button("Save capture as sound and play it", [&]() noexcept {
            isCapturing = false;
            microphone.stop();
            soundComp.load(captureData2);
            soundComp.play();
        });

        xen::OverlayPlot& stereoPlot =
            overlayMic.add_plot("Stereo capture", 16000.f, {}, {}, Vector2f(-1.f, 1.f), true, 200.f);
        xen::OverlayPlotEntry& leftCapturePlot = stereoPlot.add_entry("Left");
        xen::OverlayPlotEntry& rightCapturePlot = stereoPlot.add_entry("Right");
        stereoPlot.disable();

        xen::OverlayPlot& monoPlot =
            overlayMic.add_plot("Mono capture", 16000.f, {}, {}, Vector2f(-1.f, 1.f), true, 200.f);
        xen::OverlayPlotEntry& monoCapturePlot = monoPlot.add_entry({});

        overlayMic.add_dropdown(
            "Channels", {"Mono", "Stereo"},
            [&isCaptureStereo, &captureBitDepth, &microphone, &captureDevice, &isCapturing, &stereoPlot,
             &monoPlot](std::string const&, std::size_t i) {
                isCaptureStereo = (i == 1);
                microphone.open_device(recover_format(isCaptureStereo, captureBitDepth), 16000, 1.f, captureDevice);
                isCapturing = false;

                stereoPlot.enable(isCaptureStereo);
                monoPlot.enable(!isCaptureStereo);
            },
            0
        );

        overlayMic.add_dropdown(
            "Bit depth", {"8", "16"},
            [&captureBitDepth, &isCaptureStereo, &microphone, &captureDevice,
             &isCapturing](std::string const&, std::size_t i) {
                captureBitDepth = (i == 0 ? 8 : 16);
                microphone.open_device(recover_format(isCaptureStereo, captureBitDepth), 16000, 1.f, captureDevice);
                isCapturing = false;
            },
            0
        );

        ///////////////////
        // Sound effects //
        ///////////////////

#if !defined(RAZ_PLATFORM_EMSCRIPTEN)
        xen::OverlayWindow& overlayReverb = window.get_overlay().add_window(
            "Reverb params", overlayReverbSize, xen::Vector2f(baseWindowWidth - 20, 0.f)
        );
        overlayReverb.disable();
        xen::OverlayWindow& overlayChorus = window.get_overlay().add_window(
            "Chorus params", overlayChorusSize, xen::Vector2f(baseWindowWidth - 20, 0.f)
        );
        overlayChorus.disable();
        xen::OverlayWindow& overlayDistortion = window.get_overlay().add_window(
            "Distortion params", overlayDistortionSize, xen::Vector2f(baseWindowWidth - 20, 0.f)
        );
        overlayDistortion.disable();
        xen::OverlayWindow& overlayEcho =
            window.get_overlay().add_window("Echo params", overlayEchoSize, xen::Vector2f(baseWindowWidth - 20, 0.f));
        overlayEcho.disable();

        overlaySound.add_dropdown(
            "Sound effect", {"None", "Reverberation", "Chorus", "Distortion", "Echo"},
            [&](std::string const&, std::size_t i) noexcept {
                overlayReverb.enable(i == 1);
                overlayChorus.enable(i == 2);
                overlayDistortion.enable(i == 3);
                overlayEcho.enable(i == 4);

                if (i == 0) {
                    enabledEffect = nullptr;
                    soundComp.unlink_slot();
                    window.resize(Vector2ui(baseWindowWidth, baseWindowHeight));
                    return;
                }

                unsigned int newWindowWidth = baseWindowWidth;
                unsigned int overlayHeight{};

                switch (i) {
                case 1:
                    enabledEffect = &reverb;
                    newWindowWidth += static_cast<unsigned int>(overlayReverbSize.x);
                    overlayHeight = static_cast<unsigned int>(overlayReverbSize.y);
                    break;

                case 2:
                    enabledEffect = &chorus;
                    newWindowWidth += static_cast<unsigned int>(overlayChorusSize.x);
                    overlayHeight = static_cast<unsigned int>(overlayChorusSize.y);
                    break;

                case 3:
                    enabledEffect = &distortion;
                    newWindowWidth += static_cast<unsigned int>(overlayDistortionSize.x);
                    overlayHeight = static_cast<unsigned int>(overlayDistortionSize.y);
                    break;

                case 4:
                    enabledEffect = &echo;
                    newWindowWidth += static_cast<unsigned int>(overlayEchoSize.x);
                    overlayHeight = static_cast<unsigned int>(overlayEchoSize.y);
                    break;

                default:
                    break;
                }

                window.resize(Vector2ui(newWindowWidth - 40, std::max(baseWindowHeight, overlayHeight)));

                effectSlot.load_effect(*enabledEffect);
                soundComp.link_slot(effectSlot);
            },
            0
        );

        /////////////////////////
        // Reveberation params //
        /////////////////////////

        overlayReverb.add_slider(
            "Density",
            [&reverbParams, &reverb, &effectSlot](float val) {
                reverbParams.density = val;
                reverb.load(reverbParams);
                effectSlot.load_effect(reverb);
            },
            Vector2f(0.f, 1.f), reverbParams.density
        );

        overlayReverb.add_slider(
            "Diffusion",
            [&reverbParams, &reverb, &effectSlot](float val) {
                reverbParams.diffusion = val;
                reverb.load(reverbParams);
                effectSlot.load_effect(reverb);
            },
            Vector2f(0.f, 1.f), reverbParams.diffusion
        );

        overlayReverb.add_slider(
            "Gain",
            [&reverbParams, &reverb, &effectSlot](float val) {
                reverbParams.gain = val;
                reverb.load(reverbParams);
                effectSlot.load_effect(reverb);
            },
            Vector2f(0.f, 1.f), reverbParams.gain
        );

        overlayReverb.add_slider(
            "Gain high frequency",
            [&reverbParams, &reverb, &effectSlot](float val) {
                reverbParams.gain_high_frequency = val;
                reverb.load(reverbParams);
                effectSlot.load_effect(reverb);
            },
            Vector2f(0.f, 1.f), reverbParams.gain_high_frequency
        );

        overlayReverb.add_slider(
            "Gain low frequency",
            [&reverbParams, &reverb, &effectSlot](float val) {
                reverbParams.gain_low_frequency = val;
                reverb.load(reverbParams);
                effectSlot.load_effect(reverb);
            },
            Vector2f(0.f, 1.f), reverbParams.gain_low_frequency
        );

        overlayReverb.add_slider(
            "Decay time",
            [&reverbParams, &reverb, &effectSlot](float val) {
                reverbParams.decay_time = val;
                reverb.load(reverbParams);
                effectSlot.load_effect(reverb);
            },
            Vector2f(0.1f, 20.f), reverbParams.decay_time
        );

        overlayReverb.add_slider(
            "Decay high frequency ratio",
            [&reverbParams, &reverb, &effectSlot](float val) {
                reverbParams.decay_high_frequency_ratio = val;
                reverb.load(reverbParams);
                effectSlot.load_effect(reverb);
            },
            Vector2f(0.1f, 2.f), reverbParams.decay_high_frequency_ratio
        );

        overlayReverb.add_slider(
            "Decay low frequency ratio",
            [&reverbParams, &reverb, &effectSlot](float val) {
                reverbParams.decay_low_frequency_ratio = val;
                reverb.load(reverbParams);
                effectSlot.load_effect(reverb);
            },
            Vector2f(0.1f, 2.f), reverbParams.decay_low_frequency_ratio
        );

        overlayReverb.add_slider(
            "Reflections gain",
            [&reverbParams, &reverb, &effectSlot](float val) {
                reverbParams.reflections_gain = val;
                reverb.load(reverbParams);
                effectSlot.load_effect(reverb);
            },
            Vector2f(0.f, 3.16f), reverbParams.reflections_gain
        );

        overlayReverb.add_slider(
            "Reflections delay",
            [&reverbParams, &reverb, &effectSlot](float val) {
                reverbParams.reflections_delay = val;
                reverb.load(reverbParams);
                effectSlot.load_effect(reverb);
            },
            Vector2f(0.f, 0.3f), reverbParams.reflections_delay
        );

        overlayReverb.add_slider(
            "Reflections pan X",
            [&reverbParams, &reverb, &effectSlot](float val) {
                reverbParams.reflections_pan[0] = val;
                reverb.load(reverbParams);
                effectSlot.load_effect(reverb);
            },
            Vector2f(-100.f, 100.f), reverbParams.reflections_pan[0]
        );

        overlayReverb.add_slider(
            "Reflections pan Y",
            [&reverbParams, &reverb, &effectSlot](float val) {
                reverbParams.reflections_pan[1] = val;
                reverb.load(reverbParams);
                effectSlot.load_effect(reverb);
            },
            Vector2f(-100.f, 100.f), reverbParams.reflections_pan[1]
        );

        overlayReverb.add_slider(
            "Reflections pan Z",
            [&reverbParams, &reverb, &effectSlot](float val) {
                reverbParams.reflections_pan[2] = val;
                reverb.load(reverbParams);
                effectSlot.load_effect(reverb);
            },
            Vector2f(-100.f, 100.f), reverbParams.reflections_pan[2]
        );

        overlayReverb.add_slider(
            "Late reverb gain",
            [&reverbParams, &reverb, &effectSlot](float val) {
                reverbParams.late_reverb_gain = val;
                reverb.load(reverbParams);
                effectSlot.load_effect(reverb);
            },
            Vector2f(0.f, 10.f), reverbParams.late_reverb_gain
        );

        overlayReverb.add_slider(
            "Late reverb delay",
            [&reverbParams, &reverb, &effectSlot](float val) {
                reverbParams.late_reverb_delay = val;
                reverb.load(reverbParams);
                effectSlot.load_effect(reverb);
            },
            Vector2f(0.f, 0.01f), reverbParams.late_reverb_delay
        );

        overlayReverb.add_slider(
            "Late reverb pan X",
            [&reverbParams, &reverb, &effectSlot](float val) {
                reverbParams.late_reverb_pan[0] = val;
                reverb.load(reverbParams);
                effectSlot.load_effect(reverb);
            },
            Vector2f(-100.f, 100.f), reverbParams.late_reverb_pan[0]
        );

        overlayReverb.add_slider(
            "Late reverb pan Y",
            [&reverbParams, &reverb, &effectSlot](float val) {
                reverbParams.late_reverb_pan[1] = val;
                reverb.load(reverbParams);
                effectSlot.load_effect(reverb);
            },
            Vector2f(-100.f, 100.f), reverbParams.late_reverb_pan[1]
        );

        overlayReverb.add_slider(
            "Late reverb pan Z",
            [&reverbParams, &reverb, &effectSlot](float val) {
                reverbParams.late_reverb_pan[2] = val;
                reverb.load(reverbParams);
                effectSlot.load_effect(reverb);
            },
            Vector2f(-100.f, 100.f), reverbParams.late_reverb_pan[2]
        );

        overlayReverb.add_slider(
            "Echo time",
            [&reverbParams, &reverb, &effectSlot](float val) {
                reverbParams.echo_time = val;
                reverb.load(reverbParams);
                effectSlot.load_effect(reverb);
            },
            Vector2f(0.075f, 0.25f), reverbParams.echo_time
        );

        overlayReverb.add_slider(
            "Echo depth",
            [&reverbParams, &reverb, &effectSlot](float val) {
                reverbParams.echo_depth = val;
                reverb.load(reverbParams);
                effectSlot.load_effect(reverb);
            },
            Vector2f(0.f, 1.f), reverbParams.echo_depth
        );

        overlayReverb.add_slider(
            "Modulation time",
            [&reverbParams, &reverb, &effectSlot](float val) {
                reverbParams.modulation_time = val;
                reverb.load(reverbParams);
                effectSlot.load_effect(reverb);
            },
            Vector2f(0.04f, 4.f), reverbParams.modulation_time
        );

        overlayReverb.add_slider(
            "Modulation depth",
            [&reverbParams, &reverb, &effectSlot](float val) {
                reverbParams.modulation_depth = val;
                reverb.load(reverbParams);
                effectSlot.load_effect(reverb);
            },
            Vector2f(0.f, 1.f), reverbParams.modulation_depth
        );

        overlayReverb.add_slider(
            "Air absorption gain high frequency",
            [&reverbParams, &reverb, &effectSlot](float val) {
                reverbParams.air_absorption_gain_high_frequency = val;
                reverb.load(reverbParams);
                effectSlot.load_effect(reverb);
            },
            Vector2f(0.892f, 1.f), reverbParams.air_absorption_gain_high_frequency
        );

        overlayReverb.add_slider(
            "High frequency reference",
            [&reverbParams, &reverb, &effectSlot](float val) {
                reverbParams.high_frequency_reference = val;
                reverb.load(reverbParams);
                effectSlot.load_effect(reverb);
            },
            Vector2f(1000.f, 20000.f), reverbParams.high_frequency_reference
        );

        overlayReverb.add_slider(
            "Low frequency reference",
            [&reverbParams, &reverb, &effectSlot](float val) {
                reverbParams.low_frequency_reference = val;
                reverb.load(reverbParams);
                effectSlot.load_effect(reverb);
            },
            Vector2f(20.f, 1000.f), reverbParams.low_frequency_reference
        );

        overlayReverb.add_slider(
            "Room rolloff factor",
            [&reverbParams, &reverb, &effectSlot](float val) {
                reverbParams.room_rolloff_factor = val;
                reverb.load(reverbParams);
                effectSlot.load_effect(reverb);
            },
            Vector2f(0.f, 10.f), reverbParams.room_rolloff_factor
        );

        overlayReverb.add_checkbox(
            "Decay high frequency limit",
            [&reverbParams, &reverb, &effectSlot]() {
                reverbParams.decay_high_frequency_limit = true;
                reverb.load(reverbParams);
                effectSlot.load_effect(reverb);
            },
            [&reverbParams, &reverb, &effectSlot]() {
                reverbParams.decay_high_frequency_limit = false;
                reverb.load(reverbParams);
                effectSlot.load_effect(reverb);
            },
            reverbParams.decay_high_frequency_limit
        );

        ///////////////////
        // Chorus params //
        ///////////////////

        overlayChorus.add_dropdown(
            "Waveform", {"Sinusoid", "Triangle"},
            [&chorusParams, &chorus, &effectSlot](std::string const&, std::size_t i) {
                chorusParams.waveform = static_cast<xen::SoundWaveform>(i);
                chorus.load(chorusParams);
                effectSlot.load_effect(chorus);
            },
            static_cast<std::size_t>(chorusParams.waveform)
        );

        overlayChorus.add_slider(
            "Phase",
            [&chorusParams, &chorus, &effectSlot](float val) {
                chorusParams.phase = static_cast<int>(val);
                chorus.load(chorusParams);
                effectSlot.load_effect(chorus);
            },
            Vector2f(-180.f, 180.f), static_cast<float>(chorusParams.phase)
        );

        overlayChorus.add_slider(
            "Rate",
            [&chorusParams, &chorus, &effectSlot](float val) {
                chorusParams.rate = val;
                chorus.load(chorusParams);
                effectSlot.load_effect(chorus);
            },
            Vector2f(0.f, 10.f), chorusParams.rate
        );

        overlayChorus.add_slider(
            "Depth",
            [&chorusParams, &chorus, &effectSlot](float val) {
                chorusParams.depth = val;
                chorus.load(chorusParams);
                effectSlot.load_effect(chorus);
            },
            Vector2f(0.f, 1.f), chorusParams.depth
        );

        overlayChorus.add_slider(
            "Feedback",
            [&chorusParams, &chorus, &effectSlot](float val) {
                chorusParams.feedback = val;
                chorus.load(chorusParams);
                effectSlot.load_effect(chorus);
            },
            Vector2f(-1.f, 1.f), chorusParams.feedback
        );

        overlayChorus.add_slider(
            "Delay",
            [&chorusParams, &chorus, &effectSlot](float val) {
                chorusParams.delay = val;
                chorus.load(chorusParams);
                effectSlot.load_effect(chorus);
            },
            Vector2f(0.f, 0.016f), chorusParams.delay
        );

        ///////////////////////
        // Distortion params //
        ///////////////////////

        overlayDistortion.add_slider(
            "Edge",
            [&distortionParams, &distortion, &effectSlot](float val) {
                distortionParams.edge = val;
                distortion.load(distortionParams);
                effectSlot.load_effect(distortion);
            },
            Vector2f(0.f, 1.f), distortionParams.edge
        );

        overlayDistortion.add_slider(
            "Gain",
            [&distortionParams, &distortion, &effectSlot](float val) {
                distortionParams.gain = val;
                distortion.load(distortionParams);
                effectSlot.load_effect(distortion);
            },
            Vector2f(0.01f, 1.f), distortionParams.gain
        );

        overlayDistortion.add_slider(
            "Lowpass cutoff",
            [&distortionParams, &distortion, &effectSlot](float val) {
                distortionParams.lowpass_cutoff = val;
                distortion.load(distortionParams);
                effectSlot.load_effect(distortion);
            },
            Vector2f(80.f, 24000.f), distortionParams.lowpass_cutoff
        );

        overlayDistortion.add_slider(
            "Eq. center",
            [&distortionParams, &distortion, &effectSlot](float val) {
                distortionParams.eq_center = val;
                distortion.load(distortionParams);
                effectSlot.load_effect(distortion);
            },
            Vector2f(80.f, 24000.f), distortionParams.eq_center
        );

        overlayDistortion.add_slider(
            "Eq. bandwidth",
            [&distortionParams, &distortion, &effectSlot](float val) {
                distortionParams.eq_bandwidth = val;
                distortion.load(distortionParams);
                effectSlot.load_effect(distortion);
            },
            Vector2f(80.f, 24000.f), distortionParams.eq_bandwidth
        );

        /////////////////
        // Echo params //
        /////////////////

        overlayEcho.add_slider(
            "Delay",
            [&echoParams, &echo, &effectSlot](float val) {
                echoParams.delay = val;
                echo.load(echoParams);
                effectSlot.load_effect(echo);
            },
            Vector2f(0.f, 0.207f), echoParams.delay
        );

        overlayEcho.add_slider(
            "Left/right delay",
            [&echoParams, &echo, &effectSlot](float val) {
                echoParams.left_right_delay = val;
                echo.load(echoParams);
                effectSlot.load_effect(echo);
            },
            Vector2f(0.f, 0.404f), echoParams.left_right_delay
        );

        overlayEcho.add_slider(
            "Damping",
            [&echoParams, &echo, &effectSlot](float val) {
                echoParams.damping = val;
                echo.load(echoParams);
                effectSlot.load_effect(echo);
            },
            Vector2f(0.f, 0.99f), echoParams.damping
        );

        overlayEcho.add_slider(
            "Feedback",
            [&echoParams, &echo, &effectSlot](float val) {
                echoParams.feedback = val;
                echo.load(echoParams);
                effectSlot.load_effect(echo);
            },
            Vector2f(0.f, 1.f), echoParams.feedback
        );

        overlayEcho.add_slider(
            "Spread",
            [&echoParams, &echo, &effectSlot](float val) {
                echoParams.spread = val;
                echo.load(echoParams);
                effectSlot.load_effect(echo);
            },
            Vector2f(-1.f, 1.f), echoParams.spread
        );
#endif

        //////////////////////////
        // Starting application //
        //////////////////////////

#if !defined(USE_OPENGL_ES)
        app.run([&](const xen::FrameTimeInfo&) {
            geomPlot.push(geometryPass.recover_elapsed_time());
            sobelPlot.push(sobel.recover_elapsed_time());
            cannyPlot.push(canny.recover_elapsed_time());
            chromAberrPlot.push(chromaticAberration.recover_elapsed_time());
            blurPlot.push(boxBlur.recover_elapsed_time());
            vignettePlot.push(vignette.recover_elapsed_time());
            filmGrainPlot.push(filmGrain.recover_elapsed_time());
            pixelizationPlot.push(pixelization.recover_elapsed_time());

            if (isCapturing) {
                microphone.recover_data(captureData);

                if (captureData2.buffer.empty()) {
                    captureData2 = captureData;
                }

                captureData2.buffer.insert(
                    captureData2.buffer.end(), captureData.buffer.begin(), captureData.buffer.end()
                );

                // Stereo data is interleaved: the left capture is first, then the right (L, R, L, R, ...)

                // 8-bit depth values are unsigned and have a range between [0; 255]. They are remapped to [-1; 1]

                // 16-bit depth values are assumed to be in little endian, and are converted to big endian (each
                // 2-bytes pack [AB] is converted to [BA]) They are signed and have a range between [-32768; 32767].
                // They are remapped to [-1; 1]

                constexpr float factorU8 = 1.f / 128;
                constexpr float factorI16 = 1.f / 32767;

                if (captureBitDepth == 8) {
                    for (std::size_t i = 0; i < captureData.buffer.size(); ++i) {
                        if (isCaptureStereo) { // Stereo 8
                            leftCapturePlot.push(static_cast<float>(captureData.buffer[i]) * factorU8 - 1.f);
                            rightCapturePlot.push(static_cast<float>(captureData.buffer[i + 1]) * factorU8 - 1.f);
                            ++i;
                        }
                        else { // Mono 8
                            monoCapturePlot.push(static_cast<float>(captureData.buffer[i]) * factorU8 - 1.f);
                        }
                    }
                }
                else {
                    for (std::size_t i = 0; i < captureData.buffer.size(); i += 2) {
                        if (isCaptureStereo) { // Stereo 16
                            leftCapturePlot.push(
                                static_cast<float>(
                                    static_cast<int16_t>(captureData.buffer[i] | (captureData.buffer[i + 1] << 8u))
                                ) *
                                factorI16
                            );
                            rightCapturePlot.push(
                                static_cast<float>(
                                    static_cast<int16_t>(captureData.buffer[i + 2] | (captureData.buffer[i + 3] << 8u))
                                ) *
                                factorI16
                            );
                            i += 2;
                        }
                        else { // Mono 16
                            monoCapturePlot.push(
                                static_cast<float>(
                                    static_cast<int16_t>(captureData.buffer[i] | (captureData.buffer[i + 1] << 8u))
                                ) *
                                factorI16
                            );
                        }
                    }
                }
            }
        });
#else
        app.run();
#endif
    }
    catch (std::exception const& exception) {
        xen::Log::error("Exception occurred: "s + exception.what());
    }

    return EXIT_SUCCESS;
}

/*
void load_test_scene(Scene& scene)
{
    Resources& resources = *Resources::get();

    // Optional post fx resources
    //
Arcane::Application::GetInstance().GetMasterRenderPass()->GetPostProcessPass()->SetBloomDirtTexture(assetManager.Load2DTextureAsync(std::string("res/textures/bloom-dirt.png")));
    //
Arcane::Application::GetInstance().GetMasterRenderPass()->GetPostProcessPass()->SetVignetteTexture(assetManager.Load2DTextureAsync(std::string("res/textures/vignette_mask.jpg")));
    // Render::get()->get_post_process_pass()->set_bloom_dirt_texture(
    //     *resources.load_2d_texture_async("res/textures/bloom_dirt.png")
    // );

    // Render::get()->get_post_process_pass()->set_vignette_texture(
    //     *resources.load_2d_texture_async("res/textures/vignette_mask.jpg")
    // );

    // Load some assets for the scene at startup
    auto cube = std::make_unique<Cube>();
    auto quad = std::make_unique<Quad>();

    // Initialize some entities and components at startup
    // {
    //     std::shared_ptr<Model> gunModel = std::make_shared<Model>(*resources.load_model_async(
    //         std::string("res/3d_models/Cerberus_Gun/Cerberus_LP.FBX"),
    //         [](Model* loadedModel) {
    //             Resources& resources = *Resources::get();
    //             auto& material = loadedModel->get_meshes()[0].get_material();

    //             material.set_normal_map(
    //                 *resources.load_2d_texture_async("res/3d_models/Cerberus_Gun/Textures/Cerberus_N.tga")
    //             );
    //             material.set_metallic_map(
    //                 *resources.load_2d_texture_async("res/3d_models/Cerberus_Gun/Textures/Cerberus_M.tga")
    //             );
    //             material.set_roughness_map(
    //                 *resources.load_2d_texture_async("res/3d_models/Cerberus_Gun/Textures/Cerberus_R.tga")
    //             );
    //             material.set_ambient_occlusion_map(
    //                 *resources.load_2d_texture_async("res/3d_models/Cerberus_Gun/Textures/Cerberus_AO.tga")
    //             );
    //         }
    //     ));

    //     auto gun = scene.create_entity("Cerberus Gun");
    //     auto& transformComponent = gun.get_component<TransformComponent>();
    //     transformComponent.position = {-32.60f, -9.28f, 48.48f};
    //     transformComponent.scale = {0.05f, 0.05f, 0.05f};
    //     auto& meshComponent = gun.add_component<MeshComponent>(gunModel);
    //     meshComponent.is_static = true;
    //     meshComponent.is_transparent = false;
    // }

    // {
    //     auto shieldModel = std::make_shared<Model>(*resources.load_model_async(
    //         std::string("res/3d_models/Hyrule_Shield/HShield.obj"),
    //         [](Model* loadedModel) {
    //             Resources& resources = *Resources::get();
    //             auto& material = loadedModel->get_meshes()[0].get_material();

    //             TextureSettings srgbTextureSettings;
    //             srgbTextureSettings.is_srgb = true;

    //             material.set_albedo_map(*resources.load_2d_texture_async(
    //                 "res/3d_models/Hyrule_Shield/HShield_[Albedo].tga", &srgbTextureSettings
    //             ));
    //             material.set_normal_map(
    //                 *resources.load_2d_texture_async("res/3d_models/Hyrule_Shield/HShield_[Normal].tga")
    //             );
    //             material.set_metallic_map(
    //                 *resources.load_2d_texture_async("res/3d_models/Hyrule_Shield/HShield_[Metallic].tga")
    //             );
    //             material.set_roughness_map(
    //                 *resources.load_2d_texture_async("res/3d_models/Hyrule_Shield/HShield_[Roughness].tga")
    //             );
    //             material.set_ambient_occlusion_map(
    //                 *resources.load_2d_texture_async("res/3d_models/Hyrule_Shield/HShield_[Occlusion].tga")
    //             );
    //         }
    //     ));

    //     auto shield = scene.create_entity("Hyrule Shield");
    //     auto& transformComponent = shield.get_component<TransformComponent>();
    //     transformComponent.position = {-7.4f, -7.6f, -31.4f};
    //     auto& meshComponent = shield.add_component<MeshComponent>(shieldModel);
    //     meshComponent.is_static = true;
    //     meshComponent.is_transparent = false;
    // }

    {
        // auto cubeModel = std::make_shared<Model>(*cube);

        Model cube_model = Model(Cube());

        auto cube = scene.create_entity("Cube");
        auto& transformComponent = cube.get_component<TransformComponent>();
        transformComponent.scale = {5.0f, 5.0f, 5.0f};

        auto& meshComponent = cube.add_component<MeshComponent>(std::move(cube_model));
        meshComponent.is_static = true;
        meshComponent.is_transparent = false;
    }

    {
        // auto quadModel = std::make_shared<Model>(*quad);

        Model quad_model = Model(Quad());

        TextureSettings srgbTextureSettings;
        srgbTextureSettings.is_srgb = true;

        quad_model.get_meshes()[0].get_material().set_albedo_map(
            *resources.load_2d_texture_async("res/textures/window.png", &srgbTextureSettings)
        );

        auto window = scene.create_entity("Window");
        auto& transformComponent = window.get_component<TransformComponent>();
        transformComponent.position = {-32.60f, 10.0f, 48.48f};
        transformComponent.rotation = {0.0f, glm::radians(180.0f), 0.0f};
        transformComponent.scale = {10.0f, 10.0f, 10.0f};

        auto& meshComponent = window.add_component<MeshComponent>(std::move(quad_model));
        meshComponent.is_static = true;
        meshComponent.is_transparent = true;
        meshComponent.should_backface_cull = false;
    }

    // {
    //     Model cube_model = Model(Cube());

    //     auto playa = scene.create_entity("player");
    //     auto& rigid = playa.add_component<Rigidbody>(std::make_unique<CapsuleCollider>());
    //     auto& kinematic = playa.add_component<KinematicCharacter>(std::make_unique<CapsuleCollider>());
    //     auto& fps = playa.add_component<FpsPlayer>();
    //     auto& mesh = playa.add_component<MeshComponent>(std::move(cube_model));
    // }

    {
        auto directionalLight = scene.create_entity("Directional Light");
        auto& transformComponent = directionalLight.get_component<TransformComponent>();
        transformComponent.rotation.x = glm::radians(-120.0f);
        auto& lightComponent = directionalLight.add_component<LightComponent>();
        lightComponent.type = LightType::Directional;
        lightComponent.intensity = 2.0f;
        lightComponent.color = Color(1.0f, 1.0f, 1.0f);
        lightComponent.is_static = true;
        lightComponent.cast_shadows = true;
        lightComponent.shadow_resolution = ShadowQuality::Ultra;
    }

    {
        auto pointLight = scene.create_entity("Point Light1");
        auto& transformComponent = pointLight.get_component<TransformComponent>();
        transformComponent.position = Vector3f(24.1f, 2.2f, 47.5f);
        auto& lightComponent = pointLight.add_component<LightComponent>();
        lightComponent.type = LightType::Point;
        lightComponent.intensity = 10.0f;
        lightComponent.color = Color(0.0f, 1.0f, 0.0f);
        lightComponent.attenuation_range = 30.0f;
        lightComponent.is_static = false;
    }

    {
        auto pointLight = scene.create_entity("Point Light2");
        auto& transformComponent = pointLight.get_component<TransformComponent>();
        transformComponent.position = Vector3f(-27.2f, -9.0f, 52.0f);
        auto& lightComponent = pointLight.add_component<LightComponent>();
        lightComponent.type = LightType::Point;
        lightComponent.intensity = 30.0f;
        lightComponent.color = Color(1.0f, 0.0f, 1.0f);
        lightComponent.attenuation_range = 30.0f;
        lightComponent.is_static = true;
    }

    {
        auto spotLight = scene.create_entity("Spot Light1");
        auto& transformComponent = spotLight.get_component<TransformComponent>();
        transformComponent.position = Vector3f(-86.9f, -5.0f, -28.2f);
        transformComponent.rotation.x = glm::radians(-50.0f);
        auto& lightComponent = spotLight.add_component<LightComponent>();
        lightComponent.type = LightType::Spot;
        lightComponent.intensity = 150.0f;
        lightComponent.attenuation_range = 50.0f;
        lightComponent.color = Color(1.0f, 1.0f, 1.0f);
        lightComponent.is_static = true;
    }
    // {
    //     auto vampire = scene.create_entity("Animated Vampire");
    //     auto& transformComponent = vampire.get_component<TransformComponent>();
    //     transformComponent.position = {-70.88f, -9.22f, -39.02f};
    //     transformComponent.rotation.y = glm::radians(90.0f);
    //     transformComponent.scale = {0.05f, 0.05f, 0.05f};
    //     auto& poseAnimatorComponent = vampire.add_component<PoseAnimatorComponent>();

    //     auto animatedVampire = std::make_shared<Model>(*resources.load_model_async(
    //         std::string("res/3d_models/Vampire/Dancing_Vampire.dae"),
    //         [&poseAnimatorComponent](Model* loadedModel) {
    //             int animIndex = 0;
    //             auto clip = std::make_shared<AnimationClip>(
    //                 "res/3d_models/Vampire/Dancing_Vampire.dae", animIndex, *loadedModel
    //             );

    //             poseAnimatorComponent.pose_animator.set_animation_clip(clip);

    //             Resources& resources = *Resources::get();
    //             Material& meshMaterial = loadedModel->get_meshes()[0].get_material();
    //             meshMaterial.set_roughness_value(1.0f);
    //             meshMaterial.set_normal_map(
    //                 *resources.load_2d_texture_async("res/3d_models/Vampire/textures/Vampire_normal.png")
    //             );
    //             meshMaterial.set_metallic_map(
    //                 *resources.load_2d_texture_async("res/3d_models/Vampire/textures/Vampire_specular.png")
    //             );
    //         }
    //     ));

    //     auto& meshComponent = vampire.add_component<MeshComponent>(animatedVampire);
    //     meshComponent.is_static = false;
    //     meshComponent.is_transparent = false;
    //     meshComponent.should_backface_cull = false;
    // }

    {
        auto water = scene.create_entity("Water");
        auto& transformComponent = water.get_component<TransformComponent>();
        transformComponent.position = {25.0f, -14.0f, -50.0f};
        transformComponent.rotation = {glm::radians(-90.0f), 0.0f, 0.0f};
        transformComponent.scale = {150.0f, 150.0f, 150.0f};
        auto& waterComponent = water.add_component<WaterComponent>();
        waterComponent.water_distortion_texture =
            std::make_shared<Texture>(*resources.load_2d_texture_async("res/water/dudv.png"));
        waterComponent.water_normal_map =
            std::make_shared<Texture>(*resources.load_2d_texture_async("res/water/normals.png"));
    }

    // {
    //     auto brickModel = std::make_shared<Model>(*quad);

    //     auto bricks = scene.create_entity("Displaced Bricks");
    //     auto& transformComponent = bricks.get_component<TransformComponent>();
    //     transformComponent.position = {47.70f, -6.5f, 6.0f};
    //     transformComponent.rotation = {0.0f, glm::radians(210.0f), 0.0f};
    //     transformComponent.scale = {5.0f, 5.0f, 5.0f};
    //     auto& meshComponent = bricks.add_component<MeshComponent>(brickModel);
    //     meshComponent.is_static = true;

    //     TextureSettings srgbTextureSettings;
    //     srgbTextureSettings.is_srgb = true;

    //     Material& meshMaterial = meshComponent.model.get_meshes()[0].get_material();
    //     meshMaterial.set_albedo_map(*resources.load_2d_texture_async("res/textures/bricks2.jpg",
    //     &srgbTextureSettings));
    //     meshMaterial.set_normal_map(*resources.load_2d_texture_async("res/textures/bricks2_normal.jpg"));
    //     meshMaterial.set_displacement_map(*resources.load_2d_texture_async("res/textures/bricks2_disp.jpg"));
    //     meshMaterial.set_roughness_value(1.0f);
    // }

    // {
    //     auto flatModel = std::make_shared<Model>(*quad);

    //     auto emissionWall = scene.create_entity("Emission Lava");
    //     auto& transformComponent = emissionWall.get_component<TransformComponent>();
    //     transformComponent.position = {47.70f, 6.5f, 6.0f};
    //     transformComponent.rotation = {0.0f, glm::radians(210.0f), 0.0f};
    //     transformComponent.scale = {5.0f, 5.0f, 5.0f};
    //     auto& meshComponent = emissionWall.add_component<MeshComponent>(flatModel);
    //     meshComponent.is_static = true;

    //     TextureSettings srgbTextureSettings;
    //     srgbTextureSettings.is_srgb = true;

    //     Material& meshMaterial = meshComponent.model.get_meshes()[0].get_material();
    //     meshMaterial.set_albedo_map(*xen::Resources::get_black_srgb_texture());
    //     meshMaterial.set_emission_map(
    //         *resources.load_2d_texture_async("res/textures/circuitry-emission.png", &srgbTextureSettings)
    //     );
    //     meshMaterial.set_emission_intensity(45.0f);
    //     meshMaterial.set_metallic_value(1.0f);
    //     meshMaterial.set_roughness_value(1.0f);
    // }

    // {
    //     auto cubeModel = std::make_shared<Model>(*cube);

    //     auto cube = scene.create_entity("Emmissive Cube");
    //     auto& transformComponent = cube.get_component<TransformComponent>();
    //     transformComponent.position = {0.0f, 15.0f, 0.0f};
    //     transformComponent.scale = {5.0f, 5.0f, 5.0f};
    //     auto& meshComponent = cube.add_component<MeshComponent>(cubeModel);
    //     meshComponent.is_static = true;
    //     meshComponent.is_transparent = false;

    //     Material& meshMaterial = meshComponent.model.get_meshes()[0].get_material();
    //     meshMaterial.set_emission_color(Color(1.0f, 0.0f, 0.0f));
    //     meshMaterial.set_emission_intensity(15.0f);
    // }

    // {
    //     auto brickModel = std::make_shared<Model>(*quad);

    //     auto bricks = scene.create_entity("Emmissive Bricks");
    //     auto& transformComponent = bricks.get_component<TransformComponent>();
    //     transformComponent.position = {47.70f, 19.5f, 6.0f};
    //     transformComponent.rotation = {0.0f, glm::radians(210.0f), 0.0f};
    //     transformComponent.scale = {5.0f, 5.0f, 5.0f};
    //     auto& meshComponent = bricks.add_component<MeshComponent>(brickModel);
    //     meshComponent.is_static = true;

    //     TextureSettings srgbTextureSettings;
    //     srgbTextureSettings.is_srgb = true;

    //     Material& meshMaterial = meshComponent.model.get_meshes()[0].get_material();
    //     meshMaterial.set_albedo_map(*resources.load_2d_texture_async("res/textures/bricks2.jpg",
    //     &srgbTextureSettings));
    //     meshMaterial.set_normal_map(*resources.load_2d_texture_async("res/textures/bricks2_normal.jpg"));
    //     meshMaterial.set_displacement_map(*resources.load_2d_texture_async("res/textures/bricks2_disp.jpg"));
    //     meshMaterial.set_emission_map(
    //         *resources.load_2d_texture_async("res/textures/bricks2_emiss.png", &srgbTextureSettings)
    //     );
    //     meshMaterial.set_emission_intensity(5.0f);
    //     meshMaterial.set_roughness_value(1.0f);
    // }
}
int main()
{
    {
        ShaderLoader::set_shader_filepath("../resources/shaders/");

        Engine engine;

        auto* window0 = Windows::get()->add_window();
        window0->set_title("Game");
        window0->set_cursor_hidden(true);
        window0->on_close.connect([]() { Engine::get()->request_close(); });

        Log::info("qqq1");

        Render::get()->init_renderer();

        auto* input_scheme =
            Inputs::get()->add_scheme("Test", std::make_unique<InputScheme>("./test_scheme2.json"), true);
        input_scheme->read();

        auto scroll = std::make_unique<ScrollInputAxis>(Vector3f::Up);

        input_scheme->add_axis("scroll", std::move(scroll));

        input_scheme->get_button("exit")->on_button.connect([](InputAction action, InputMods) {
            // Do some useful workaround with input actions
            if (action != InputAction::Press) {
                return;
            }
            Engine::get()->request_close();
            std::exit(1);
        });

        input_scheme->get_button("capture_mouse")->on_button.connect([&window0](InputAction action, InputMods) {
            if (action != InputAction::Press) {
                return;
            }
            window0->set_cursor_hidden(!window0->is_cursor_hidden());
        });

        input_scheme->get_button("fullscreen")->on_button.connect([&window0](InputAction action, InputMods) {
            if (action != InputAction::Press) {
                return;
            }
            window0->set_fullscreen(!window0->is_fullscreen());
        });

        auto* scene = Scenes::get()->set_scene(std::make_unique<Scene>());

        Resources::get()->first_load();

        input_scheme->get_button("save")->on_button.connect([&scene](InputAction action, InputMods) {
            if (action != InputAction::Press) {
                return;
            }
            scene->serialize("pivo.org");
        });

        input_scheme->get_button("load")->on_button.connect([&scene](InputAction action, InputMods) {
            if (action != InputAction::Press) {
                return;
            }
            scene->deserialize("pivo.org");
        });

        input_scheme->get_axis("scroll")->on_axis.connect([&scene](float amount) {
            auto& current_camera = scene->get_camera();
            current_camera.set_fov(current_camera.get_fov() - amount);
        });

        // scene->serialize("lol.scene");
        // scene->deserialize("lol.scene");

        // for (auto ent : scene->get_root().recursive_hierarchy()) {
        // std::cerr << ent.get_name() << '\n';
        // }

        Render::get()->init_scene(*scene);

        Resources& resources = *Resources::get();

        // Optional post fx resources
        //
Arcane::Application::GetInstance().GetMasterRenderPass()->GetPostProcessPass()->SetBloomDirtTexture(assetManager.Load2DTextureAsync(std::string("res/textures/bloom-dirt.png")));
        //
Arcane::Application::GetInstance().GetMasterRenderPass()->GetPostProcessPass()->SetVignetteTexture(assetManager.Load2DTextureAsync(std::string("res/textures/vignette_mask.jpg")));
        // Render::get()->get_post_process_pass()->set_bloom_dirt_texture(
        //     *resources.load_2d_texture_async("res/textures/bloom_dirt.png")
        // );

        // Render::get()->get_post_process_pass()->set_vignette_texture(
        //     *resources.load_2d_texture_async("res/textures/vignette_mask.jpg")
        // );

        // load_test_scene(*scene);

        // Something when destoyed blockes program exit scenario, find the culprit
        auto exit_code = engine.run();
    }
    return 1;
}
    */