#include "render_system.hpp"

#include <application.hpp>
#include <data/image.hpp>
#include <data/image_format.hpp>
#include <math/transform/transform.hpp>
#include <render/camera.hpp>
#include <render/light.hpp>
#include <render/mesh_renderer.hpp>
#include <render/renderer.hpp>
#if defined(XEN_USE_XR)
#include <xr/xr_system.hpp>
#endif

#include <tracy/Tracy.hpp>
#include <GL/glew.h> // Needed by TracyOpenGL.hpp
#include <tracy/TracyOpenGL.hpp>

namespace xen {
void RenderSystem::set_cubemap(Cubemap&& cubemap)
{
    this->cubemap = std::move(cubemap);
    camera_ubo.bind_uniform_block(this->cubemap->get_program(), "uboCameraInfo", 0);
}

#if defined(XEN_USE_XR)
void RenderSystem::enable_xr(XrSystem& xr_system)
{
    this->xr_system = &xr_system;

    xr_system.initialize_session();
    resize_viewport(xr_system.get_optimal_view_size());
}
#endif

void RenderSystem::resize_viewport(Vector2ui const& size)
{
    ZoneScopedN("RenderSystem::resize_viewport");

    this->size = size;

    Renderer::resize_viewport({0, 0}, this->size);

    if (camera_entity) {
        camera_entity->get_component<Camera>().resize_viewport(this->size);
    }

    render_graph.resize_viewport(this->size);
}

bool RenderSystem::update(FrameTimeInfo const& time_info)
{
    ZoneScopedN("RenderSystem::update");
    TracyGpuZone("RenderSystem::update");

    camera_ubo.bind_base(0);
    lights_ubo.bind_base(1);
    time_ubo.bind_base(2);
    model_ubo.bind_base(3);

    // TODO: this should be made only once at the passes' shader programs' initialization (as is done when updating
    // shaders), not every frame
    //   Forcing to update shaders when adding a new pass would not be ideal either, as it implies many operations. Find
    //   a better & user-friendly way
    for (size_t i = 0; i < render_graph.get_node_count(); ++i) {

        RenderShaderProgram const& pass_program = render_graph.get_node(i).get_program();
        camera_ubo.bind_uniform_block(pass_program, "uboCameraInfo", 0);
        lights_ubo.bind_uniform_block(pass_program, "uboLightsInfo", 1);
        time_ubo.bind_uniform_block(pass_program, "uboTimeInfo", 2);
    }

    time_ubo.bind();
    time_ubo.send_data(time_info.delta_time, 0);
    time_ubo.send_data(time_info.global_time, sizeof(float));

#if defined(XEN_USE_XR)
    if (xr_system) {
        render_xr_frame();
    }
    else
#endif
    {
        send_camera_info();
        render_graph.execute(*this);
    }

#if defined(XEN_CONFIG_DEBUG) && !defined(XEN_SKIP_RENDERER_ERRORS)
    Renderer::print_errors();
#endif

#if !defined(XEN_NO_WINDOW)
    if (window) {
        return window->run(time_info.delta_time);
    }
#endif

    return true;
}

void RenderSystem::update_lights() const
{
    ZoneScopedN("RenderSystem::update_lights");

    uint light_count = 0;

    lights_ubo.bind();

    for (Entity const* entity : entities) {
        if (!entity->is_enabled() || !entity->has_component<Light>()) {
            continue;
        }

        update_light(*entity, light_count);
        ++light_count;
    }

    lights_ubo.send_data(light_count, sizeof(Vector4f) * 4 * 100);
}

void RenderSystem::update_shaders() const
{
    ZoneScopedN("RenderSystem::update_shaders");

    render_graph.update_shaders();

    for (size_t i = 0; i < render_graph.get_node_count(); ++i) {
        RenderShaderProgram const& pass_program = render_graph.get_node(i).get_program();
        camera_ubo.bind_uniform_block(pass_program, "uboCameraInfo", 0);
        lights_ubo.bind_uniform_block(pass_program, "uboLightsInfo", 1);
        time_ubo.bind_uniform_block(pass_program, "uboTimeInfo", 2);
    }

    for (Entity* entity : entities) {
        if (!entity->has_component<MeshRenderer>()) {
            continue;
        }

        auto& mesh_renderer = entity->get_component<MeshRenderer>();

        for (Material& material : mesh_renderer.get_materials()) {
            material.get_program().update_shaders();
        }

        update_materials(mesh_renderer);
    }
}

void RenderSystem::update_materials(MeshRenderer const& mesh_renderer) const
{
    ZoneScopedN("RenderSystem::update_materials(MeshRenderer)");

    for (Material const& material : mesh_renderer.get_materials()) {
        RenderShaderProgram const& material_program = material.get_program();

        material_program.send_attributes();
        material_program.init_textures();
#if !defined(USE_WEBGL)
        material_program.init_image_textures();
#endif

        camera_ubo.bind_uniform_block(material_program, "uboCameraInfo", 0);
        lights_ubo.bind_uniform_block(material_program, "uboLightsInfo", 1);
        time_ubo.bind_uniform_block(material_program, "uboTimeInfo", 2);
        model_ubo.bind_uniform_block(material_program, "uboModelInfo", 3);
    }
}

void RenderSystem::update_materials() const
{
    ZoneScopedN("RenderSystem::update_materials");

    for (Entity const* entity : entities) {
        if (entity->has_component<MeshRenderer>()) {
            update_materials(entity->get_component<MeshRenderer>());
        }
    }
}

void RenderSystem::save_to_image(FilePath const& filepath, TextureFormat format, PixelDataType data_type) const
{
    ZoneScopedN("RenderSystem::save_to_image");
    ZoneTextF("Path: %s", filepath.to_utf8().c_str());

    ImageColorspace colorspace = ImageColorspace::RGB;

    switch (format) {
    case TextureFormat::DEPTH:
        colorspace = ImageColorspace::GRAY;
        data_type = PixelDataType::FLOAT;
        break;

    case TextureFormat::RGBA:
    case TextureFormat::BGRA:
        colorspace = ImageColorspace::RGBA;
        break;

    default:
        break;
    }

    Image image(size, colorspace, (data_type == PixelDataType::FLOAT ? ImageDataType::FLOAT : ImageDataType::BYTE));
    Renderer::recover_frame(size, format, data_type, image.data());

    ImageFormat::save(filepath, image, true);
}

void RenderSystem::destroy()
{
#if !defined(XEN_NO_WINDOW)
    if (window) {
        window->set_should_close();
    }
#endif
}

void RenderSystem::link_entity(EntityPtr const& entity)
{
    ZoneScopedN("RenderSystem::link_entity");

    System::link_entity(entity);

    if (entity->has_component<Camera>()) {
        camera_entity = entity.get();
    }

    if (entity->has_component<Light>()) {
        update_lights();
    }

    if (entity->has_component<MeshRenderer>()) {
        update_materials(entity->get_component<MeshRenderer>());
    }
}

void RenderSystem::init()
{
    ZoneScopedN("RenderSystem::initialize");

    register_components<Camera, Light, MeshRenderer>();

    // TODO: this Renderer initialization is technically useless; the RenderSystem needs to have it initialized before
    // construction
    //  (either manually or through the Window's initialization), since it constructs the RenderGraph's rendering
    //  objects As such, if reaching here, the Renderer is necessarily already functional. Ideally, this call below
    //  should be the only one in the whole program
    Renderer::init();
    Renderer::enable(Capability::CULL);
    Renderer::enable(Capability::BLEND);
    Renderer::enable(Capability::DEPTH_TEST);
    Renderer::enable(Capability::STENCIL_TEST);

#if !defined(USE_OPENGL_ES)
    Renderer::enable(Capability::CUBEMAP_SEAMLESS);
#endif

#if !defined(USE_OPENGL_ES)
    // Setting the depth to a [0; 1] range instead of a [-1; 1] one is always a good thing, since the [-1; 0] subrange
    // is never used anyway
    if (Renderer::check_version(4, 5) || Renderer::is_extension_supported("GL_ARB_clip_control")) {
        Renderer::set_clip_control(ClipOrigin::LOWER_LEFT, ClipDepth::ZERO_TO_ONE);
    }

    if (Renderer::check_version(4, 3)) {
        Renderer::set_label(RenderObjectType::BUFFER, camera_ubo.get_index(), "Camera uniform buffer");
        Renderer::set_label(RenderObjectType::BUFFER, lights_ubo.get_index(), "Lights uniform buffer");
        Renderer::set_label(RenderObjectType::BUFFER, time_ubo.get_index(), "Time uniform buffer");
        Renderer::set_label(RenderObjectType::BUFFER, model_ubo.get_index(), "Model uniform buffer");
    }
#endif
}

void RenderSystem::init(Vector2ui const& scene_size)
{
    init();
    resize_viewport(scene_size);
}

void RenderSystem::send_camera_info() const
{
    Log::rt_assert(camera_entity != nullptr, "Error: The render system needs a camera to send its info.");
    Log::rt_assert(
        camera_entity->has_component<Transform>(), "Error: The camera must have a transform component to send its info."
    );

    ZoneScopedN("RenderSystem::send_camera_info");

    auto& camera = camera_entity->get_component<Camera>();
    auto& cam_transform = camera_entity->get_component<Transform>();

    camera_ubo.bind();

    if (cam_transform.has_updated()) {
        if (camera.get_camera_type() == CameraType::LOOK_AT) {
            camera.compute_look_at(cam_transform.get_position());
        }
        else {
            camera.compute_view(cam_transform);
        }

        camera.compute_inverse_view();

        send_view(camera.get_view());
        send_inverse_view(camera.get_inverse_view());
        send_camera_position(cam_transform.get_position());

        cam_transform.set_updated(false);
    }

    send_projection(camera.get_projection());
    send_inverse_projection(camera.get_inverse_projection());
    send_view_projection(camera.get_projection() * camera.get_view());
}

void RenderSystem::update_light(Entity const& entity, uint light_index) const
{
    auto const& light = entity.get_component<Light>();
    size_t const data_stride = sizeof(Vector4f) * 4 * light_index;

    if (light.get_type() == LightType::DIRECTIONAL) {
        lights_ubo.send_data(Vector4f(0.f), static_cast<uint>(data_stride));
    }
    else {
        Log::rt_assert(
            entity.has_component<Transform>(), "Error: A non-directional light needs to have a Transform component."
        );
        lights_ubo.send_data(
            Vector4f(entity.get_component<Transform>().get_position(), 1.f), static_cast<uint>(data_stride)
        );
    }

    lights_ubo.send_data(light.get_direction(), static_cast<uint>(data_stride + sizeof(Vector4f)));
    lights_ubo.send_data(light.get_color(), static_cast<uint>(data_stride + sizeof(Vector4f) * 2));
    lights_ubo.send_data(light.get_energy(), static_cast<uint>(data_stride + sizeof(Vector4f) * 3));
    lights_ubo.send_data(
        light.get_angle().value, static_cast<uint>(data_stride + sizeof(Vector4f) * 3 + sizeof(float))
    );
}

#if defined(XEN_USE_XR)
void RenderSystem::render_xr_frame()
{
    ZoneScopedN("RenderSystem::render_xr_frame");
    TracyGpuZone("RenderSystem::render_xr_frame")

        const bool has_rendered =
            xr_system->render_frame([this](Vector3f position, Quaternion rotation, ViewFov view_fov) {
                if (camera_entity) {
                    auto const& cam_transform = camera_entity->get_component<Transform>();
                    position = cam_transform.get_rotation() * position + cam_transform.get_position();
                    rotation = cam_transform.get_rotation() * rotation;
                }

                Matrix4 inverse_view = rotation.to_rotation_matrix();
                inverse_view[3][0] = position.x;
                inverse_view[3][1] = position.y;
                inverse_view[3][2] = position.z;
                Matrix4 const view = inverse_view.inverse();

                float const tanAngleRight = std::tan(view_fov.angles[0].value);
                float const tanAngleLeft = std::tan(view_fov.angles[1].value);
                float const tanAngleUp = std::tan(view_fov.angles[2].value);
                float const tanAngleDown = std::tan(view_fov.angles[3].value);
                float const invAngleWidth = 1.f / (tanAngleRight - tanAngleLeft);
                float const invAngleHeight = 1.f / (tanAngleUp - tanAngleDown);
                float const angleWidthDiff = tanAngleRight + tanAngleLeft;
                float const angleHeightDiff = tanAngleUp + tanAngleDown;
                constexpr float nearZ = 0.1f;
                constexpr float farZ = 1000.f;
                constexpr float invDepthDiff = 1.f / (farZ - nearZ);
                Matrix4 const projection(
                    {2.f * invAngleWidth, 0.f, angleWidthDiff * invAngleWidth, 0.f, 0.f, 2.f * invAngleHeight,
                     angleHeightDiff * invAngleHeight, 0.f, 0.f, 0.f, -(farZ + nearZ) * invDepthDiff,
                     -(farZ * (nearZ + nearZ)) * invDepthDiff, 0.f, 0.f, -1.f, 0.f}
                );

                camera_ubo.bind();
                send_view(view);
                send_inverse_view(inverse_view);
                send_projection(projection);
                send_inverse_projection(projection.inverse());
                send_view_projection(projection * view);
                send_camera_position(position);

                render_graph.execute(*this);

                Log::rt_assert(render_graph.last_executed_pass, "Error: There is no valid last executed pass.");
                Framebuffer const& final_framebuffer = render_graph.last_executed_pass->get_framebuffer();
                Log::rt_assert(
                    final_framebuffer.get_color_buffer_count() >= 1,
                    "Error: The last executed pass must have at least one write color buffer."
                );
                Log::rt_assert(
                    final_framebuffer.has_depth_buffer() ||
                        render_graph.geometry_pass.get_framebuffer().has_depth_buffer(),
                    "Error: Either the last executed pass or the geometry pass must have a write depth buffer."
                );

                Texture2D const& depth_buffer =
                    (final_framebuffer.has_depth_buffer() ?
                         final_framebuffer.get_depth_buffer() :
                         render_graph.geometry_pass.get_framebuffer().get_depth_buffer());
                return std::make_pair(std::cref(final_framebuffer.get_color_buffer(0)), std::cref(depth_buffer));
            });

#if !defined(XEN_NO_WINDOW)
    if (!has_rendered) {
        return;
    }

    Framebuffer const& final_framebuffer = render_graph.last_executed_pass->get_framebuffer();
    Texture2D const& depth_buffer =
        (final_framebuffer.has_depth_buffer() ? final_framebuffer.get_depth_buffer() :
                                                render_graph.geometry_pass.get_framebuffer().get_depth_buffer());
    copy_to_window(final_framebuffer.get_color_buffer(0), depth_buffer, window->get_size());
#endif
}
#endif

void RenderSystem::copy_to_window(
    const Texture2D& color_buffer, const Texture2D& depth_buffer, Vector2ui const& window_size
) const
{
    Log::rt_assert(
        color_buffer.get_colorspace() != TextureColorspace::DEPTH &&
            color_buffer.get_colorspace() != TextureColorspace::INVALID,
        "Error: The given color buffer must have a valid & non-depth colorspace to be copied to the window."
    );
    Log::rt_assert(
        depth_buffer.get_colorspace() == TextureColorspace::DEPTH,
        "Error: The given depth buffer must have a depth colorspace to be copied to the window."
    );

    ZoneScopedN("RenderSystem::copy_to_window");
    TracyGpuZone("RenderSystem::copy_to_window")

        static RenderPass window_copy_pass = []() {
            RenderPass copy_pass(
                FragmentShader::load_from_source(R"(
      in vec2 fragTexcoords;

      uniform sampler2D uniFinalColorBuffer;
      uniform sampler2D uniFinalDepthBuffer;
      uniform vec2 uniSizeFactor;

      layout(location = 0) out vec4 fragColor;

      void main() {
        vec2 scaledUv = fragTexcoords * uniSizeFactor;
        fragColor     = texture(uniFinalColorBuffer, scaledUv).rgba;
        gl_FragDepth  = texture(uniFinalDepthBuffer, scaledUv).r;
      }
    )"),
                "Window copy pass"
            );

            RenderShaderProgram& copy_program = copy_pass.get_program();
            copy_program.set_attribute(0, "uniFinalColorBuffer");
            copy_program.set_attribute(1, "uniFinalDepthBuffer");

            return copy_pass;
        }();

    RenderShaderProgram& window_copy_program = window_copy_pass.get_program();

    Vector2f const size_factor(
        static_cast<float>(size.x) / static_cast<float>(window_size.x),
        static_cast<float>(size.y) / static_cast<float>(window_size.y)
    );
    window_copy_program.set_attribute(size_factor, "uniSizeFactor");
    window_copy_program.send_attributes();

    window_copy_program.use();
    Renderer::activate_texture(0);
    color_buffer.bind();
    Renderer::activate_texture(1);
    depth_buffer.bind();

    Renderer::bind_framebuffer(0);
    Renderer::clear(MaskType::COLOR | MaskType::DEPTH | MaskType::STENCIL);

    Renderer::set_depth_function(DepthStencilFunction::ALWAYS);
    window_copy_pass.execute();
    Renderer::set_depth_function(DepthStencilFunction::LESS);
}
}