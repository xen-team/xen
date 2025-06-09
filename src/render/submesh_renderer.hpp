#pragma once

#include <data/submesh.hpp>
#include <render/graphic_objects.hpp>

namespace xen {
enum class RenderMode : uint32_t {
    POINT,    ///< Renders the submesh as points.
    LINE,     ///< Renders the submesh as lines.
    TRIANGLE, ///< Renders the submesh as triangles.
#if !defined(USE_OPENGL_ES)
    PATCH ///< Renders the submesh as patches, used for tessellation. Requires OpenGL 4.3+; unavailable with OpenGL ES.
#endif
};

class SubmeshRenderer {
public:
    SubmeshRenderer() = default;
    explicit SubmeshRenderer(Submesh const& submesh, RenderMode render_mode = RenderMode::TRIANGLE)
    {
        load(submesh, render_mode);
    }

    RenderMode get_render_mode() const { return render_mode; }

    size_t get_material_index() const { return material_index; }

    /// Sets a specific mode to render the submesh into.
    /// \param render_mode Render mode to apply.
    /// \param submesh Submesh to load the render mode's indices from.
    void set_render_mode(RenderMode render_mode, Submesh const& submesh);

    void set_material_index(size_t material_index) { this->material_index = material_index; }

    /// Clones the submesh renderer.
    /// \warning This doesn't load anything onto the graphics card; the load() function must be called afterward with a
    /// Submesh for this.
    /// \return Cloned submesh renderer.
    SubmeshRenderer clone() const;

    /// Loads the submesh's data (vertices & indices) onto the graphics card.
    /// \param submesh Submesh to load the data from.
    /// \param render_mode Primitive type to render the submesh with.
    void load(Submesh const& submesh, RenderMode render_mode = RenderMode::TRIANGLE);

    /// Draws the submesh in the scene.
    void draw() const;

private:
    VertexArray vao;
    VertexBuffer vbo;
    IndexBuffer ibo;

    RenderMode render_mode = RenderMode::TRIANGLE;
    std::function<void(VertexBuffer const&, IndexBuffer const&)> render_func{};

    size_t material_index = 0;

private:
    void load_vertices(Submesh const& submesh);

    void load_indices(Submesh const& submesh);
};
}