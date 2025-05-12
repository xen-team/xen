#include <data/mesh.hpp>
#include <script/lua_wrapper.hpp>
#include <utils/type_utils.hpp>

#define SOL_ALL_SAFETIES_ON 1
#include "sol/sol.hpp"

namespace xen {

using namespace TypeUtils;

void LuaWrapper::register_mesh_types()
{
    sol::state& state = get_state();

    {
        sol::usertype<Mesh> mesh = state.new_usertype<Mesh>(
            "Mesh",
            sol::constructors<
                Mesh(), Mesh(Plane const&, float, float), Mesh(Sphere const&, uint32_t, SphereMeshType),
                Mesh(Triangle const&, Vector2f const&, Vector2f const&, Vector2f const&), Mesh(Quad const&),
                Mesh(AABB const&)>(),
            sol::base_classes, sol::bases<Component>()
        );
        mesh["get_submeshes"] = PickNonConstOverload<>(&Mesh::get_submeshes);
        mesh["get_bounding_box"] = &Mesh::get_bounding_box;
        mesh["recover_vertex_count"] = &Mesh::recover_vertex_count;
        mesh["recover_triangle_count"] = &Mesh::recover_triangle_count;
        mesh["add_submesh"] = &Mesh::add_submesh<>;
        mesh["conpute_bounding_box"] = &Mesh::conpute_bounding_box;
        mesh["compute_tangents"] = &Mesh::compute_tangents;

        state.new_enum<SphereMeshType>("SphereMeshType", {{"UV", SphereMeshType::UV}, {"ICO", SphereMeshType::ICO}});
    }

    {
        sol::usertype<Submesh> submesh = state.new_usertype<Submesh>("Submesh", sol::constructors<Submesh()>());
        submesh["get_vertices"] = PickConstOverload<>(&Submesh::get_vertices);
        submesh["get_vertex_count"] = &Submesh::get_vertex_count;
        submesh["get_triangle_indices"] = PickConstOverload<>(&Submesh::get_triangle_indices);
        submesh["get_triangle_index_count"] = &Submesh::get_triangle_index_count;
        submesh["get_bounding_box"] = &Submesh::get_bounding_box;
        submesh["conpute_bounding_box"] = &Submesh::conpute_bounding_box;

        sol::usertype<Vertex> vertex = state.new_usertype<Vertex>("Vertex", sol::constructors<Vertex()>());
        vertex["position"] = &Vertex::position;
        vertex["texcoords"] = &Vertex::texcoords;
        vertex["normal"] = &Vertex::normal;
        vertex["tangent"] = &Vertex::tangent;
    }
}
}