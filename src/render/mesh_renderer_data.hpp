#pragma once

#include "data/mesh.hpp"
#include <render/material.hpp>
#include <render/submesh_renderer.hpp>

namespace xen {
class MeshRendererData {
public:
    MeshRendererData() = default;
    explicit MeshRendererData(
        Mesh const& mesh, RenderMode render_mode = RenderMode::TRIANGLE, bool skip_depth = false
    ) : skip_depth(skip_depth)
    {
        load(mesh, render_mode);
    }

    MeshRendererData(MeshRendererData const&) = delete;
    MeshRendererData& operator=(MeshRendererData const&) = delete;
    MeshRendererData(MeshRendererData&&) noexcept = default;
    MeshRendererData& operator=(MeshRendererData&&) noexcept = default;

    [[nodiscard]] std::vector<SubmeshRenderer> const& get_submesh_renderers() const { return submesh_renderers; }
    [[nodiscard]] std::vector<SubmeshRenderer>& get_submesh_renderers() { return submesh_renderers; }

    [[nodiscard]] std::vector<Material> const& get_materials() const { return materials; }
    [[nodiscard]] std::vector<Material>& get_materials() { return materials; }

    template <typename... Args>
    SubmeshRenderer& add_submesh_renderer(Args&&... args)
    {
        return submesh_renderers.emplace_back(std::forward<Args>(args)...);
    }

    Material& set_material(Material&& material);
    Material& add_material(Material&& material = Material()) { return materials.emplace_back(std::move(material)); }
    void remove_material(size_t material_index);

    void load(Mesh const& mesh, RenderMode render_mode = RenderMode::TRIANGLE);
    void load_materials() const;
    void draw() const;

    bool skip_depth = false;

private:
    std::vector<SubmeshRenderer> submesh_renderers;
    std::vector<Material> materials;
};
}