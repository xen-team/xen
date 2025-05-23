#pragma once

namespace xen {
class Mesh;
class MeshRenderer;
class FilePath;
class Rigidbody;
class Entity;

namespace GltfFormat {
/// Loads a mesh from a glTF or GLB file.
/// \param filepath File from which to load the mesh.
/// \return Pair containing respectively the mesh's data (vertices & indices) and rendering information (materials,
/// textures, ...).
std::pair<Mesh, MeshRenderer> load(FilePath const& filepath, FilePath const& proxy_filepath);

Rigidbody& create_map_rigidbody_from_mesh(Entity& entity, Mesh map_mesh, float mass = 0.0f, float friction = 0.5f);
}
}