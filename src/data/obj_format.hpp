#pragma once

namespace xen {
class FilePath;
class Mesh;
class MeshRenderer;

namespace ObjFormat {

/// Loads a mesh from an OBJ file.
/// \param filepath File from which to load the mesh.
/// \return Pair containing respectively the mesh's data (vertices & indices) and rendering information (materials,
/// textures, ...).
std::pair<Mesh, MeshRenderer> load(FilePath const& filepath);

/// Saves a mesh to an OBJ file.
/// \param filepath File to which to save the mesh.
/// \param mesh Mesh to export data from.
/// \param mesh_renderer Optional mesh renderer to export materials & textures from.
void save(FilePath const& filepath, Mesh const& mesh, MeshRenderer const* mesh_renderer = nullptr);
}
}