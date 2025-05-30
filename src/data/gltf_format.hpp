#pragma once

#include "world.hpp"
namespace xen {
class Mesh;
class MeshRendererData;
class FilePath;
class Rigidbody;
class Entity;

namespace GltfFormat {
/// Loads a mesh from a glTF or GLB file.
/// \param filepath File from which to load the mesh.
/// \return Pair containing respectively the mesh's data (vertices & indices) and rendering information (materials,
/// textures, ...).
std::pair<Mesh, MeshRendererData> load(FilePath const& filepath);

Rigidbody& create_map_rigidbody_from_mesh(Entity& entity, std::shared_ptr<Mesh> map_mesh);
}
}