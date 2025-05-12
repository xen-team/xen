#include <data/mesh.hpp>
#include <render/mesh_renderer.hpp>
#include <script/lua_wrapper.hpp>
#include <utils/type_utils.hpp>

#define SOL_ALL_SAFETIES_ON 1
#include "sol/sol.hpp"

namespace xen {
using namespace TypeUtils;

void LuaWrapper::register_mesh_renderer_types()
{
    sol::state& state = get_state();

    {
        sol::usertype<MeshRenderer> mesh_renderer = state.new_usertype<MeshRenderer>(
            "MeshRenderer",
            sol::constructors<MeshRenderer(), MeshRenderer(Mesh const&), MeshRenderer(Mesh const&, RenderMode)>(),
            sol::base_classes, sol::bases<Component>()
        );
        mesh_renderer["is_enabled"] = &MeshRenderer::is_enabled;
        mesh_renderer["get_submesh_renderers"] = PickNonConstOverload<>(&MeshRenderer::get_submesh_renderers);
        mesh_renderer["get_materials"] = PickNonConstOverload<>(&MeshRenderer::get_materials);
        mesh_renderer["enable"] =
            sol::overload([](MeshRenderer& r) { r.enable(); }, PickOverload<bool>(&MeshRenderer::enable));
        mesh_renderer["disable"] = &MeshRenderer::disable;
        mesh_renderer["set_render_mode"] = &MeshRenderer::set_render_mode;
        mesh_renderer["set_material"] = [](MeshRenderer& r, Material& mat) { return &r.set_material(std::move(mat)); };
        mesh_renderer["add_material"] = sol::overload(
            [](MeshRenderer& r) { return &r.add_material(); },
            [](MeshRenderer& r, Material& mat) { return &r.add_material(std::move(mat)); }
        );
        mesh_renderer["remove_material"] = &MeshRenderer::remove_material;
        mesh_renderer["add_submesh_renderer"] = sol::
            overload(&MeshRenderer::add_submesh_renderer<>, &MeshRenderer::add_submesh_renderer<Submesh const&>, &MeshRenderer::add_submesh_renderer<Submesh const&, RenderMode>);
        mesh_renderer["clone"] = &MeshRenderer::clone;
        mesh_renderer["load"] = sol::overload(
            [](MeshRenderer& r, Mesh const& m) { r.load(m); },
            PickOverload<Mesh const&, RenderMode>(&MeshRenderer::load)
        );
        mesh_renderer["load_materials"] = &MeshRenderer::load_materials;
        mesh_renderer["draw"] = &MeshRenderer::draw;
    }

    {
        sol::usertype<SubmeshRenderer> submesh_renderer = state.new_usertype<SubmeshRenderer>(
            "SubmeshRenderer",
            sol::constructors<
                SubmeshRenderer(), SubmeshRenderer(Submesh const&), SubmeshRenderer(Submesh const&, RenderMode)>()
        );
        submesh_renderer["get_render_mode"] = &SubmeshRenderer::get_render_mode;
        submesh_renderer["set_render_mode"] = &SubmeshRenderer::set_render_mode;
        submesh_renderer["material_index"] =
            sol::property(&SubmeshRenderer::get_material_index, &SubmeshRenderer::set_material_index);
        submesh_renderer["clone"] = &SubmeshRenderer::clone;
        submesh_renderer["load"] = sol::overload(
            [](SubmeshRenderer& r, Submesh const& s) { r.load(s); },
            PickOverload<Submesh const&, RenderMode>(&SubmeshRenderer::load)
        );
        submesh_renderer["draw"] = &SubmeshRenderer::draw;

        state.new_enum<RenderMode>(
            "RenderMode", {{"POINT", RenderMode::POINT},
                           {"TRIANGLE", RenderMode::TRIANGLE},
#if !defined(USE_OPENGL_ES)
                           {"PATCH", RenderMode::PATCH}
#endif
                          }
        );
    }
}
}