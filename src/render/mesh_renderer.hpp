#pragma once

#include <component.hpp>
#include <render/material.hpp>
#include <render/submesh_renderer.hpp>

namespace xen {

class Mesh;

class MeshRenderer final : public Component {
public:
    MeshRenderer() = default;
    explicit MeshRenderer(Mesh const& mesh, RenderMode render_mode = RenderMode::TRIANGLE) { load(mesh, render_mode); }
    MeshRenderer(MeshRenderer const&) = delete;
    MeshRenderer(MeshRenderer&&) noexcept = default;

    MeshRenderer& operator=(MeshRenderer const&) = delete;
    MeshRenderer& operator=(MeshRenderer&&) noexcept = default;

    [[nodiscard]] bool is_enabled() const { return enabled; }

    [[nodiscard]] std::vector<SubmeshRenderer> const& get_submesh_renderers() const { return submesh_renderers; }

    [[nodiscard]] std::vector<SubmeshRenderer>& get_submesh_renderers() { return submesh_renderers; }

    [[nodiscard]] std::vector<Material> const& get_materials() const { return materials; }

    [[nodiscard]] std::vector<Material>& get_materials() { return materials; }

    /// Changes the mesh renderer's state.
    /// \note Only the rendering will be affected, not the entity itself.
    /// \param enabled True if the mesh should be rendered, false otherwise.
    /// \see Entity::enable()
    void enable(bool enabled = true) { enabled = enabled; }

    /// Disables the rendering of the mesh.
    /// \note Only the rendering will be affected, not the entity itself.
    /// \see Entity::disable()
    void disable() { enable(false); }

    /// Sets a specific mode to render the mesh into.
    /// \param render_mode Render mode to apply.
    /// \param mesh Mesh to load the render mode's indices from.
    void set_render_mode(RenderMode render_mode, Mesh const& mesh);

    /// Sets one unique material for the whole mesh.
    /// \warning This clears all previously existing materials.
    /// \param material Material to be set.
    /// \return Reference to the set material.
    Material& set_material(Material&& material);

    /// Adds a given material into the mesh renderer.
    /// \note This doesn't apply the material to any submesh; to do so, manually set the corresponding material index to
    /// any submesh renderer.
    /// \param material Material to be added.
    /// \return Reference to the newly added material.
    Material& add_material(Material&& material = Material()) { return materials.emplace_back(std::move(material)); }

    /// Removes an existing material.
    /// \param material_index Index of the material to remove.
    void remove_material(size_t material_index);

    /// Adds a new submesh renderer to render a submesh with.
    /// \tparam Args Types of the arguments to be forwarded to the submesh renderer.
    /// \param args Arguments to be forwarded to the submesh renderer.
    /// \return Reference to the newly added submesh renderer.
    template <typename... Args>
    SubmeshRenderer& add_submesh_renderer(Args&&... args)
    {
        return submesh_renderers.emplace_back(std::forward<Args>(args)...);
    }

    /// Clones the mesh renderer.
    /// \warning This doesn't load anything onto the GPU; to do so, call the load() function taking a Mesh afterward.
    /// \return Cloned mesh renderer.
    [[nodiscard]] MeshRenderer clone() const;

    /// Loads a mesh onto the GPU.
    /// \param mesh Mesh to be loaded.
    /// \param render_mode Render mode to apply.
    void load(Mesh const& mesh, RenderMode render_mode = RenderMode::TRIANGLE);

    /// Loads the materials.
    void load_materials() const;

    /// Renders the mesh.
    void draw() const;

private:
    bool enabled = true;

    std::vector<SubmeshRenderer> submesh_renderers{};
    std::vector<Material> materials{};
};
}