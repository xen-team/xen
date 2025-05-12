#pragma once

namespace xen {
class Mesh;
class MeshRenderer;
class FilePath;

namespace GltfFormat {
/// Loads a mesh from a glTF or GLB file.
/// \param filepath File from which to load the mesh.
/// \return Pair containing respectively the mesh's data (vertices & indices) and rendering information (materials,
/// textures, ...).
std::pair<Mesh, MeshRenderer> load(FilePath const& filepath);
}
}