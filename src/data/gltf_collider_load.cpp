#include "entity.hpp"
#include "gltf_collider_load.hpp"

#include "utils/shape.hpp"
#include "world.hpp"

#include "fastgltf/core.hpp"
#include "fastgltf/math.hpp"
#include <fastgltf/util.hpp>
#include "fastgltf/types.hpp"
#include "fastgltf/tools.hpp"
#include <functional>

#include <tracy/Tracy.hpp>

namespace xen {
namespace GltfColliderLoader {

xen::Transform GetNodeLocalTransform(fastgltf::Node const& node)
{
    if (auto const* trs = std::get_if<fastgltf::TRS>(&node.transform)) {
        return xen::Transform(
            xen::Vector3f(trs->translation.x(), trs->translation.y(), trs->translation.z()),
            xen::Quaternion(trs->rotation.x(), trs->rotation.y(), trs->rotation.z(), trs->rotation.w()),
            xen::Vector3f(trs->scale.x(), trs->scale.y(), trs->scale.z())
        );
    }
    else if (auto const* matrix = std::get_if<fastgltf::math::fmat4x4>(&node.transform)) {
        Log::warning("[GltfColliderLoad] Node transform is a matrix, expected TRS. Attempting decomposition.");
        fastgltf::math::fvec3 scale_v, translation_v;
        fastgltf::math::fquat rotation_q;
        fastgltf::math::decomposeTransformMatrix(*matrix, translation_v, rotation_q, scale_v);
        return {
            xen::Vector3f(translation_v.x(), translation_v.y(), translation_v.z()),
            xen::Quaternion(rotation_q.x(), rotation_q.y(), rotation_q.z(), rotation_q.w()),
            xen::Vector3f(scale_v.x(), scale_v.y(), scale_v.z())
        };
    }
    return xen::Transform();
}

void ComputeGlobalTransforms(fastgltf::Asset const& asset, std::vector<xen::Transform>& out_global_transforms)
{
    return;
    ZoneScopedN("GltfColliderLoader::ComputeGlobalTransforms");

    out_global_transforms.assign(asset.nodes.size(), xen::Transform());
    std::vector<bool> computed(asset.nodes.size(), false);

    std::function<void(size_t, xen::Transform const&)> compute_recursive =
        [&](size_t node_idx, xen::Transform const& parent_global_transform) {
            if (computed[node_idx]) {}

            auto const& node = asset.nodes[node_idx];
            xen::Transform local_transform = GetNodeLocalTransform(node);
            xen::Transform current_global_transform = parent_global_transform * local_transform;

            out_global_transforms[node_idx] = current_global_transform;
            computed[node_idx] = true;

            for (size_t child_node_idx : node.children) {
                compute_recursive(child_node_idx, current_global_transform);
            }
        };

    for (auto const& scene : asset.scenes) {
        xen::Transform scene_root_parent_transform;
        for (size_t root_node_idx : scene.nodeIndices) {
            if (!computed[root_node_idx]) {
                compute_recursive(root_node_idx, scene_root_parent_transform);
            }
        }
    }
    for (size_t i = 0; i < asset.nodes.size(); ++i) {
        if (!computed[i]) {
            Log::warning(
                "[GltfColliderLoad::ComputeGlobalTransforms] Node %zu was not computed via scene graph traversal.", i
            );
        }
    }
}

xen::AABB ComputeAABBFromLocalVertices(std::vector<xen::Vector3f> const& vertices)
{
    ZoneScopedN("GltfColliderLoader::ComputeAABBFromLocalVertices");
    if (vertices.empty()) {
        return xen::AABB(xen::Vector3f::Zero, xen::Vector3f::Zero);
    }
    xen::Vector3f min_pos = vertices[0];
    xen::Vector3f max_pos = vertices[0];
    for (size_t i = 1; i < vertices.size(); ++i) {
        min_pos.x = std::min(min_pos.x, vertices[i].x);
        min_pos.y = std::min(min_pos.y, vertices[i].y);
        min_pos.z = std::min(min_pos.z, vertices[i].z);
        max_pos.x = std::max(max_pos.x, vertices[i].x);
        max_pos.y = std::max(max_pos.y, vertices[i].y);
        max_pos.z = std::max(max_pos.z, vertices[i].z);
        // min_pos = xen::Vector3f::min(min_pos, vertices[i]);
        // max_pos = xen::Vector3f::max(max_pos, vertices[i]);
    }
    return xen::AABB(min_pos, max_pos);
}

bool is_identity_rotation(xen::Quaternion const& q, float epsilon)
{
    return (
        std::abs(q.x) < epsilon && std::abs(q.y) < epsilon && std::abs(q.z) < epsilon &&
        std::abs(std::abs(q.w) - 1.0f) < epsilon
    );
}

void LoadCollidersFromGltf(
    xen::World& world, fastgltf::Asset const& asset, std::vector<xen::Transform> const& global_node_transforms,
    std::string const& proxy_prefix
)
{
    return;
    ZoneScopedN("GltfColliderLoader::LoadCollidersFromGltf");
    Log::info("[GltfColliderLoad] Starting to load colliders with prefix ", proxy_prefix.c_str());

    for (size_t node_idx = 0; node_idx < asset.nodes.size(); ++node_idx) {
        auto const& node = asset.nodes[node_idx];

        // far todo prefix checking
        if (true) {
            Log::info("[GltfColliderLoad] Found proxy node: ", node.name.c_str(), " index: ", node_idx);

            if (!node.meshIndex.has_value()) {
                Log::warning("[GltfColliderLoad] Proxy node no mesh< skip by: ", node.name.c_str());
                continue;
            }

            xen::Transform const& node_global_transform = global_node_transforms[node_idx];

            xen::Vector3f effective_node_position = node_global_transform.get_position();
            xen::Quaternion effective_node_rotation = node_global_transform.get_rotation();
            xen::Vector3f effective_node_scale = node_global_transform.get_scale();

            fastgltf::Mesh const& gltf_mesh = asset.meshes[*node.meshIndex];
            for (fastgltf::Primitive const& primitive : gltf_mesh.primitives) {
                ZoneScopedN("GltfColliderLoader::ProcessPrimitive");
                auto pos_it = primitive.findAttribute("POSITION");
                if (pos_it == primitive.attributes.end()) {
                    Log::warning(
                        "[GltfColliderLoad] Proxy mesh in node '%s' has no POSITION attribute. Skipping primitive.",
                        node.name.c_str()
                    );
                    continue;
                }

                fastgltf::Accessor const& pos_accessor = asset.accessors[pos_it->accessorIndex];
                std::vector<xen::Vector3f> local_vertices_of_mesh;
                local_vertices_of_mesh.reserve(pos_accessor.count);

                fastgltf::iterateAccessor<fastgltf::math::fvec3>(
                    asset, pos_accessor,
                    [&local_vertices_of_mesh](fastgltf::math::fvec3 const& pos) {
                        local_vertices_of_mesh.emplace_back(pos.x(), pos.y(), pos.z());
                    }
                );

                if (local_vertices_of_mesh.empty()) {
                    Log::warning(
                        "[GltfColliderLoad] Proxy mesh primitive in node '%s' resulted in 0 vertices. Skipping.",
                        node.name.c_str()
                    );
                    continue;
                }

                xen::AABB local_mesh_aabb = ComputeAABBFromLocalVertices(local_vertices_of_mesh);
                xen::Vector3f local_mesh_center = local_mesh_aabb.compute_centroid();
                xen::Vector3f local_mesh_half_extents = local_mesh_aabb.compute_half_extents();

                xen::Vector3f shape_scaled_half_extents = local_mesh_half_extents * effective_node_scale;

                xen::AABB shape_definition_aabb(-shape_scaled_half_extents, shape_scaled_half_extents);

                xen::Entity& entity = world.add_entity();
                auto& entity_transform = entity.add_component<xen::Transform>();

                entity_transform.set_position(
                    effective_node_position + effective_node_rotation * (local_mesh_center * effective_node_scale)
                );
                entity_transform.set_scale(xen::Vector3f::One);

                float const rotation_epsilon = 1e-5f;
                bool has_rotation = is_identity_rotation(effective_node_rotation, rotation_epsilon);

                if (has_rotation) {
                    Log::info("[GltfColliderLoad] Using OBB for node '", node.name.c_str(), "' due to rotation.");
                    entity_transform.set_rotation(xen::Quaternion::Identity);
                    xen::OBB obb_shape(shape_definition_aabb, effective_node_rotation);
                    //////// entity.add_component<xen::Collider>(std::move(obb_shape));
                }
                else {
                    Log::info(
                        "[GltfColliderLoad] Using AABB for node '", node.name.c_str(), "' (no significant rotation)."
                    );
                    entity_transform.set_rotation(xen::Quaternion::Identity);
                    xen::AABB aabb_shape(
                        shape_definition_aabb.get_min_position(), shape_definition_aabb.get_max_position()
                    );
                    //////// entity.add_component<xen::Collider>(std::move(aabb_shape));
                }

                Log::vinfo(
                    "[GltfColliderLoad] Created collider for {}. EntityPos: ({}, {}, {}), ShapeCenterLocal : (0, 0, "
                    "0), "
                    "ShapeHalfExtents : ({}, {}, {}) ",
                    node.name.c_str(), entity_transform.get_position().x, entity_transform.get_position().y,
                    entity_transform.get_position().z, shape_scaled_half_extents.x, shape_scaled_half_extents.y,
                    shape_scaled_half_extents.z
                );
            }
        }
    }
    Log::info("[GltfColliderLoad] Finished loading colliders.");
}
}
}