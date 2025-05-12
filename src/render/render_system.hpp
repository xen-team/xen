#pragma once

#include <system.hpp>
#include <render/cubemap.hpp>
#include <render/renderer.hpp>
#include <render/render_graph.hpp>
#include <render/platform/uniform_buffer.hpp>
#include <render/window.hpp>

#if !defined(__APPLE__) && !defined(__EMSCRIPTEN__) && !defined(XEN_NO_WINDOW)
// XR currently isn't available with macOS or Emscripten and requires windowing capabilities
// #define XEN_USE_XR
#endif

namespace xen {
class Entity;
class MeshRenderer;
#if defined(XEN_USE_XR)
class XrSystem;
#endif

/// RenderSystem class, handling the rendering part.
class RenderSystem final : public System {
    friend RenderGraph;

public:
    /// Creates a render system, initializing its inner data.
    RenderSystem() { init(); }

    /// Creates a render system with a given scene size.
    /// \param sceneWidth Width of the scene.
    /// \param sceneHeight Height of the scene.
    RenderSystem(Vector2ui const& scene_size) : RenderSystem() { resize_viewport(scene_size); }

#if !defined(XEN_NO_WINDOW)
    /// Creates a render system along with a window.
    /// \param windowWidth Width of the window.
    /// \param windowHeight Height of the window.
    /// \param window_title Title of the window.
    /// \param settings Settings to create the window with.
    /// \param anti_aliasing_sample_count Number of anti-aliasing samples.
    /// \note The window's width & height are to be considered just hints; the window manager remains responsible for
    /// the actual dimensions, which may be lower.
    ///   This can notably happen when the requested window size exceeds what the screens can display. The actual
    ///   window's size can be queried afterward.
    /// \see get_window(), Window::get_width(), Window::get_height()
    RenderSystem(
        Vector2ui const& window_size, std::string const& window_title,
        WindowSetting window_settings = WindowSetting::DEFAULT, uint8_t anti_aliasing_sample_count = 1
    ) : window{Window::create(*this, window_size, window_title, window_settings, anti_aliasing_sample_count)}
    {
        init(window->get_size());
    }
#endif

    Vector2ui get_scene_size() const { return size; }

    uint get_scene_width() const { return size.x; }

    uint get_scene_height() const { return size.y; }

#if !defined(XEN_NO_WINDOW)
    bool has_window() const { return (window != nullptr); }

    Window const& get_window() const
    {
        Log::rt_assert(has_window(), "Error: The window must be set before being accessed.");
        return *window;
    }

    Window& get_window() { return const_cast<Window&>(static_cast<RenderSystem const*>(this)->get_window()); }
#endif

    const RenderPass& get_geometry_pass() const { return render_graph.get_geometry_pass(); }

    RenderPass& get_geometry_pass() { return render_graph.get_geometry_pass(); }

    RenderGraph const& get_render_graph() const { return render_graph; }

    RenderGraph& get_render_graph() { return render_graph; }

    bool has_cubemap() const { return cubemap.has_value(); }

    Cubemap const& get_cubemap() const
    {
        Log::rt_assert("Error: The cubemap must be set before being accessed." && has_cubemap());
        return *cubemap;
    }

    void set_cubemap(Cubemap&& cubemap);

#if defined(XEN_USE_XR)
    void enable_xr(XrSystem& xr_system);
#endif

#if !defined(XEN_NO_WINDOW)
    void create_window(
        Vector2ui const& size, const std::string& title = {}, WindowSetting settings = WindowSetting::DEFAULT,
        uint8_t anti_aliasing_sample_count = 1
    )
    {
        window = Window::create(*this, size, title, settings, anti_aliasing_sample_count);
    }
#endif

    void resize_viewport(Vector2ui const& size);

    bool update(FrameTimeInfo const& time_info) override;

    /// Updates all lights referenced by the RenderSystem, sending their data to the GPU.
    void update_lights() const;

    void update_shaders() const;

    void update_materials(MeshRenderer const& mesh_renderer) const;

    void update_materials() const;

    /// Retrieves & saves the back buffer's data from the GPU.
    /// \warning The pixel storage pack & unpack alignments should be set to 1 in order to recover actual pixels.
    /// \see Renderer::set_pixel_storage()
    /// \warning Retrieving an image from the GPU is slow; use this function with caution.
    void save_to_image(
        FilePath const& filepath, TextureFormat format = TextureFormat::RGB,
        PixelDataType data_type = PixelDataType::UBYTE
    ) const;

    void remove_cubemap() { cubemap.reset(); }

    void destroy() override;

protected:
    void link_entity(EntityPtr const& entity) override;

private:
    Vector2ui size;

#if !defined(XEN_NO_WINDOW)
    WindowPtr window{};
#endif

    Entity* camera_entity{};
    RenderGraph render_graph;
    UniformBuffer camera_ubo = UniformBuffer(sizeof(Matrix4) * 5 + sizeof(Vector4f), UniformBufferUsage::DYNAMIC);
    UniformBuffer lights_ubo =
        UniformBuffer(sizeof(Vector4f) * 4 * 100 + sizeof(Vector4ui), UniformBufferUsage::DYNAMIC);
    UniformBuffer time_ubo = UniformBuffer(sizeof(float) * 2, UniformBufferUsage::STREAM);
    UniformBuffer model_ubo = UniformBuffer(sizeof(Matrix4), UniformBufferUsage::STREAM);

    std::optional<Cubemap> cubemap{};

#if defined(XEN_USE_XR)
    const XrSystem* xr_system{};
#endif

private:
    void init();

    void init(Vector2ui const& scene_size);

    void send_camera_info() const;

    void send_view(Matrix4 const& view) const { camera_ubo.send_data(view, 0); }

    void send_inverse_view(Matrix4 const& inverse_view) const { camera_ubo.send_data(inverse_view, sizeof(Matrix4)); }

    void send_projection(Matrix4 const& projection) const { camera_ubo.send_data(projection, sizeof(Matrix4) * 2); }

    void send_inverse_projection(Matrix4 const& inverse_projection) const
    {
        camera_ubo.send_data(inverse_projection, sizeof(Matrix4) * 3);
    }

    void send_view_projection(Matrix4 const& view_projection) const
    {
        camera_ubo.send_data(view_projection, sizeof(Matrix4) * 4);
    }

    void send_camera_position(Vector3f const& camera_pos) const
    {
        camera_ubo.send_data(camera_pos, sizeof(Matrix4) * 5);
    }

    /// Updates a single light, sending its data to the GPU.
    /// \warning The lights UBO needs to be bound before calling this function.
    /// \note If resetting a removed light or updating one not yet known by the application, call update_lights()
    /// instead to fully take that change into account.
    /// \param entity Light entity to be updated; if not a directional light, needs to have a Transform component.
    /// \param light_index Index of the light to be updated.
    void update_light(Entity const& entity, uint light_index) const;

#if defined(XEN_USE_XR)
    void render_xr_frame();
#endif

    void
    copy_to_window(const Texture2D& color_buffer, const Texture2D& depth_buffer, Vector2ui const& window_size) const;
};
}