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
struct MeshCache {
    std::shared_ptr<Mesh> mesh;
    std::shared_ptr<MeshRendererData> mesh_renderer_data;

    [[nodiscard]] auto get_shared() const { return std::make_pair(mesh, MeshRenderer(mesh_renderer_data)); }
};

static auto& get_mesh_cache()
{
    static std::map<std::string, MeshCache> mesh_cache;
    return mesh_cache;
}
std::pair<std::shared_ptr<Mesh>, MeshRenderer> load(FilePath const& filepath)
{
    ZoneScopedN("MeshFormat::load");
    ZoneTextF("Path: %s", filepath.to_utf8().c_str());

    auto& mesh_cache = get_mesh_cache();
    if (auto it = mesh_cache.find(filepath.to_utf8()); it != mesh_cache.cend()) {
        return it->second.get_shared();
    }

    std::string const file_extension = StrUtils::to_lower_copy(filepath.recover_extension().to_utf8());

    Mesh temp_mesh;
    MeshRendererData temp_mesh_renderer_data;

    if (file_extension == "gltf" || file_extension == "glb") {
        auto loaded_data = GltfFormat::load(filepath);
        temp_mesh = std::move(loaded_data.first);
        temp_mesh_renderer_data = std::move(loaded_data.second);
    }
    else if (file_extension == "obj") {
        auto loaded_data = ObjFormat::load(filepath);
        temp_mesh = std::move(loaded_data.first);
        temp_mesh_renderer_data = std::move(loaded_data.second);
    }
    else if (file_extension == "off") {
        //     auto loaded_mesh = OffFormat::load(filepath);
        //     temp_mesh_renderer = MeshRenderer(loaded_mesh);
        //     temp_mesh = std::move(loaded_mesh);
    }
    else if (file_extension == "fbx") {
#if defined(XEN_USE_FBX)
        // auto loaded_data = FbxFormat::load(filepath);
        // temp_mesh = std::move(loaded_data.first);
        // temp_mesh_renderer = std::move(loaded_data.second);
#else
        throw std::invalid_argument("[MeshFormat] FBX format unsupported; check that you enabled its usage when "
                                    "building xen (if on a supported platform).");
#endif
    }

    auto shared_mesh = std::make_shared<Mesh>(std::move(temp_mesh));

    auto cached_entry = mesh_cache.emplace(
        filepath.to_utf8(),
        MeshCache{
            .mesh = shared_mesh,
            .mesh_renderer_data = std::make_shared<MeshRendererData>(std::move(temp_mesh_renderer_data))
        }
    );

    return cached_entry.first->second.get_shared();
}

void save(FilePath const& filepath, Mesh const& mesh, MeshRenderer const* mesh_renderer)
{
    ZoneScopedN("MeshFormat::save");
    ZoneTextF("Path: %s", filepath.to_utf8().c_str());

    std::string const file_extension = StrUtils::to_lower_copy(filepath.recover_extension().to_utf8());

    if (file_extension == "obj") {
        // ObjFormat::save(filepath, mesh, mesh_renderer);
    }
    else {
        throw std::invalid_argument(
            "[MeshFormat] Unsupported mesh file extension '" + file_extension + "' for saving."
        );
    }
}
}