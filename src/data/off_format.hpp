#pragma once

namespace xen {
class FilePath;
class Mesh;

namespace OffFormat {
/// Loads a mesh from an OFF file.
/// \param filepath File from which to load the mesh.
/// \return Loaded mesh's data (vertices & indices).
Mesh load(FilePath const& filepath);
}
}
