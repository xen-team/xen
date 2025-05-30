#pragma once

#include "render/mesh_renderer_data.hpp"
#include <component.hpp>
#include <render/material.hpp>
#include <render/submesh_renderer.hpp>

namespace xen {

class Mesh;

class MeshRenderer final : public Component {
public:
    MeshRenderer() = default;
    explicit MeshRenderer(std::shared_ptr<MeshRendererData> data_ptr) : m_data(std::move(data_ptr)) {}

    explicit MeshRenderer(Mesh const& mesh, RenderMode render_mode = RenderMode::TRIANGLE, bool skip_depth = false) :
        m_data(std::make_shared<MeshRendererData>(mesh, render_mode, skip_depth))
    {
    }

    MeshRenderer(MeshRenderer const&) = default;
    MeshRenderer(MeshRenderer&&) noexcept = default;

    MeshRenderer& operator=(MeshRenderer const&) = default;
    MeshRenderer& operator=(MeshRenderer&&) noexcept = default;

    [[nodiscard]] bool is_enabled() const { return enabled; }

    [[nodiscard]] std::shared_ptr<MeshRendererData> get_data() const { return m_data; }
    [[nodiscard]] MeshRendererData& get_mutable_data() { return *m_data; }

    [[nodiscard]] std::vector<SubmeshRenderer> const& get_submesh_renderers() const
    {
        return m_data->get_submesh_renderers();
    }
    [[nodiscard]] std::vector<SubmeshRenderer>& get_submesh_renderers() { return m_data->get_submesh_renderers(); }

    [[nodiscard]] std::vector<Material> const& get_materials() const { return m_data->get_materials(); }
    [[nodiscard]] std::vector<Material>& get_materials() { return m_data->get_materials(); }

    void enable(bool enabled = true) { this->enabled = enabled; }
    void disable() { enable(false); }

    void set_render_mode(RenderMode render_mode, Mesh const& mesh) { m_data->load(mesh, render_mode); }
    Material& set_material(Material&& material) { return m_data->set_material(std::move(material)); }
    Material& add_material(Material&& material = Material()) { return m_data->add_material(std::move(material)); }
    void remove_material(size_t material_index) { m_data->remove_material(material_index); }

    template <typename... Args>
    SubmeshRenderer& add_submesh_renderer(Args&&... args)
    {
        return m_data->add_submesh_renderer(std::forward<Args>(args)...);
    }

    [[nodiscard]] bool is_skip_depth() const { return m_data->skip_depth; }
    void set_skip_depth(bool value) { m_data->skip_depth = value; }

    void load_materials() const { m_data->load_materials(); }
    void draw() const
    {
        if (enabled && m_data) {
            m_data->draw();
        }
    }

private:
    std::shared_ptr<MeshRendererData> m_data;
    bool enabled = true;
};
}