#include <data/mesh_format.hpp>

#include <data/fbx_format.hpp>
#include <data/gltf_format.hpp>
#include <data/mesh.hpp>
#include <data/obj_format.hpp>
#include <data/off_format.hpp>
#include <render/mesh_renderer.hpp>
#include <utils/filepath.hpp>
#include <utils/str_utils.hpp>

#include <tracy/Tracy.hpp>

namespace xen::MeshFormat {
std::pair<Mesh, MeshRenderer> load(FilePath const& filepath)
{
    ZoneScopedN("MeshFormat::load");
    ZoneTextF("Path: %s", filepath.to_utf8().c_str());

    std::string const file_extension = StrUtils::to_lower_copy(filepath.recover_extension().to_utf8());

    if (file_extension == "gltf" || file_extension == "glb") {
        return GltfFormat::load(filepath);
    }
    else if (file_extension == "obj") {
        return ObjFormat::load(filepath);
    }
    else if (file_extension == "off") {
        Mesh mesh = OffFormat::load(filepath);
        MeshRenderer mesh_renderer(mesh);
        return {std::move(mesh), std::move(mesh_renderer)};
    }
    else if (file_extension == "fbx") {
#if defined(XEN_USE_FBX)
        return FbxFormat::load(filepath);
#else
        throw std::invalid_argument("[MeshFormat] FBX format unsupported; check that you enabled its usage when "
                                    "building xen (if on a supported platform).");
#endif
    }

    throw std::invalid_argument("[MeshFormat] Unsupported mesh file extension '" + file_extension + "' for loading.");
}

void save(FilePath const& filepath, Mesh const& mesh, MeshRenderer const* mesh_renderer)
{
    ZoneScopedN("MeshFormat::save");
    ZoneTextF("Path: %s", filepath.to_utf8().c_str());

    std::string const file_extension = StrUtils::to_lower_copy(filepath.recover_extension().to_utf8());

    if (file_extension == "obj") {
        ObjFormat::save(filepath, mesh, mesh_renderer);
    }
    else {
        throw std::invalid_argument(
            "[MeshFormat] Unsupported mesh file extension '" + file_extension + "' for saving."
        );
    }
}
}