#include <animation/skeleton.hpp>
#include <audio/sound.hpp>
#include <data/bvh_format.hpp>
#if defined(XEN_USE_FBX)
#include <data/fbx_format.hpp>
#endif
#include <data/gltf_format.hpp>
#include <data/image.hpp>
#include <data/image_format.hpp>
#include <data/mesh.hpp>
#include <data/mesh_format.hpp>
#include <data/obj_format.hpp>
#include <data/off_format.hpp>
#include <data/tga_format.hpp>
#if defined(XEN_USE_AUDIO)
#include <data/wav_format.hpp>
#endif
#include <render/mesh_renderer.hpp>
#include <script/lua_wrapper.hpp>
#include <utils/filepath.hpp>
#include <utils/type_utils.hpp>

#define SOL_ALL_SAFETIES_ON 1
#include "sol/sol.hpp"

namespace xen {
using namespace TypeUtils;

void LuaWrapper::register_file_format_types()
{
    sol::state& state = get_state();

    {
        // sol::table bvhFormat = state["BvhFormat"].get_or_create<sol::table>();
        // bvhFormat["load"] = &BvhFormat::load;
    }

#if defined(XEN_USE_FBX)
    {
        sol::table fbxFormat = state["FbxFormat"].get_or_create<sol::table>();
        fbxFormat["load"] = &FbxFormat::load;
    }
#endif

    {
        sol::table gltfFormat = state["GltfFormat"].get_or_create<sol::table>();
        gltfFormat["load"] = &GltfFormat::load;
    }

    {
        sol::table imageFormat = state["ImageFormat"].get_or_create<sol::table>();
        imageFormat["load"] = sol::overload(
            [](FilePath const& p) { return ImageFormat::load(p); },
            PickOverload<FilePath const&, bool>(&ImageFormat::load)
        );
        imageFormat["load_from_data"] = sol::overload(
            [](std::vector<unsigned char> const& d) { return ImageFormat::load_from_data(d); },
            PickOverload<std::vector<unsigned char> const&, bool>(&ImageFormat::load_from_data),
            [](unsigned char const* d, size_t s) { return ImageFormat::load_from_data(d, s); },
            PickOverload<unsigned char const*, size_t, bool>(&ImageFormat::load_from_data)
        );
        imageFormat["save"] = sol::overload(
            [](FilePath const& p, Image const& i) { ImageFormat::save(p, i); },
            PickOverload<FilePath const&, Image const&, bool>(&ImageFormat::save)
        );
    }

    {
        sol::table meshFormat = state["MeshFormat"].get_or_create<sol::table>();
        meshFormat["load"] = &MeshFormat::load;
        meshFormat["save"] = sol::overload(
            [](FilePath const& p, Mesh const& m) { MeshFormat::save(p, m); },
            PickOverload<FilePath const&, Mesh const&, MeshRenderer const*>(&MeshFormat::save)
        );
    }

    {
        sol::table objFormat = state["ObjFormat"].get_or_create<sol::table>();
        objFormat["load"] = &ObjFormat::load;
        objFormat["save"] = sol::overload(
            [](FilePath const& p, Mesh const& m) { ObjFormat::save(p, m); },
            PickOverload<FilePath const&, Mesh const&, MeshRenderer const*>(&ObjFormat::save)
        );
    }

    {
        sol::table offFormat = state["OffFormat"].get_or_create<sol::table>();
        offFormat["load"] = &OffFormat::load;
    }

    {
        sol::table tgaFormat = state["TgaFormat"].get_or_create<sol::table>();
        tgaFormat["load"] = sol::overload(
            [](FilePath const& p) { return TgaFormat::load(p); }, PickOverload<FilePath const&, bool>(&TgaFormat::load)
        );
    }

#if defined(XEN_USE_AUDIO)
    {
        sol::table wavFormat = state["WavFormat"].get_or_create<sol::table>();
        wavFormat["load"] = &WavFormat::load;
        wavFormat["save"] = &WavFormat::save;
    }
#endif
}

}
