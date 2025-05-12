#include "mesh_renderer.hpp"

#include <data/mesh.hpp>

#include <tracy/Tracy.hpp>
#include <GL/glew.h> // Needed by TracyOpenGL.hpp
#include <tracy/TracyOpenGL.hpp>

namespace xen {
void MeshRenderer::set_render_mode(RenderMode render_mode, Mesh const& mesh)
{
    for (size_t i = 0; i < submesh_renderers.size(); ++i) {
        submesh_renderers[i].set_render_mode(render_mode, mesh.get_submeshes()[i]);
    }
}

Material& MeshRenderer::set_material(Material&& material)
{
    ZoneScopedN("MeshRenderer::set_material");

    materials.clear();

    Material& new_material = materials.emplace_back(std::move(material));
    new_material.get_program().send_attributes();
    new_material.get_program().init_textures();
#if !defined(USE_WEBGL)
    new_material.get_program().init_image_textures();
#endif

    for (SubmeshRenderer& submesh_renderer : submesh_renderers) {
        submesh_renderer.set_material_index(0);
    }

    return new_material;
}

void MeshRenderer::remove_material(size_t material_index)
{
    Log::rt_assert(material_index < materials.size(), "Error: Cannot remove a material that does not exist.");

    materials.erase(materials.begin() + static_cast<std::ptrdiff_t>(material_index));

    for (SubmeshRenderer& submesh_renderer : submesh_renderers) {
        size_t const submesh_material_index = submesh_renderer.get_material_index();

        if (submesh_material_index == std::numeric_limits<size_t>::max()) {
            continue;
        }

        if (submesh_material_index == material_index) {
            submesh_renderer.set_material_index(std::numeric_limits<size_t>::max());
        }
        else if (submesh_material_index > material_index) {
            submesh_renderer.set_material_index(submesh_material_index - 1);
        }
    }
}

MeshRenderer MeshRenderer::clone() const
{
    MeshRenderer mesh_renderer;

    mesh_renderer.submesh_renderers.reserve(submesh_renderers.size());
    for (SubmeshRenderer const& submesh_renderer : submesh_renderers) {
        mesh_renderer.submesh_renderers.emplace_back(submesh_renderer.clone());
    }

    mesh_renderer.materials.reserve(materials.size());
    for (Material const& material : materials) {
        mesh_renderer.materials.emplace_back(material.clone());
    }

    return mesh_renderer;
}

void MeshRenderer::load(Mesh const& mesh, RenderMode render_mode)
{
    ZoneScopedN("MeshRenderer::load");

    if (mesh.get_submeshes().empty()) {
        Log::error("[MeshRenderer] Cannot load an empty mesh.");
        return;
    }

    Log::debug("[MeshRenderer] Loading mesh data...");

    submesh_renderers.resize(mesh.get_submeshes().size());

    for (size_t submesh_index = 0; submesh_index < mesh.get_submeshes().size(); ++submesh_index) {
        submesh_renderers[submesh_index].load(mesh.get_submeshes()[submesh_index], render_mode);
    }

    // If no material exists, create a default one
    if (materials.empty()) {
        set_material(Material(MaterialType::COOK_TORRANCE));
    }

    Log::debug("[MeshRenderer] Loaded mesh data");
}

void MeshRenderer::load_materials() const
{
    ZoneScopedN("MeshRenderer::load_materials");

    for (Material const& material : materials) {
        material.get_program().send_attributes();
        material.get_program().init_textures();
#if !defined(USE_WEBGL)
        material.get_program().init_image_textures();
#endif
    }
}

void MeshRenderer::draw() const
{
    ZoneScopedN("MeshRenderer::draw");
    TracyGpuZone("MeshRenderer::draw");

    for (SubmeshRenderer const& submesh_renderer : submesh_renderers) {
        if (submesh_renderer.get_material_index() != std::numeric_limits<size_t>::max()) {
            Log::rt_assert(
                submesh_renderer.get_material_index() < materials.size(),
                "Error: The material index does not reference any existing material."
            );
            materials[submesh_renderer.get_material_index()].get_program().bind_textures();
        }

        submesh_renderer.draw();
    }
}
}