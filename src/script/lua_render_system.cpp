#include <render/render_system.hpp>
#include <script/lua_wrapper.hpp>
#include <utils/type_utils.hpp>

#define SOL_ALL_SAFETIES_ON 1
#include "sol/sol.hpp"

namespace xen {

using namespace TypeUtils;

void LuaWrapper::register_render_system_types()
{
    sol::state& state = get_state();

    {
        sol::usertype<RenderSystem> render_system = state.new_usertype<RenderSystem>(
            "RenderSystem",
            sol::constructors<
                RenderSystem(), RenderSystem(Vector2ui const&)
#if !defined(XEN_NO_WINDOW)
                                    ,
                RenderSystem(Vector2ui const&, const std::string&),
                RenderSystem(Vector2ui const&, const std::string&, WindowSetting),
                RenderSystem(Vector2ui const&, const std::string&, WindowSetting, uint8_t)
#endif
                >(),
            sol::base_classes, sol::bases<System>()
        );
        render_system["get_scene_width"] = &RenderSystem::get_scene_width;
        render_system["get_scene_height"] = &RenderSystem::get_scene_height;
#if !defined(XEN_NO_WINDOW)
        render_system["has_window"] = &RenderSystem::has_window;
        render_system["get_window"] = PickNonConstOverload<>(&RenderSystem::get_window);
#endif
        render_system["get_geometry_pass"] = PickNonConstOverload<>(&RenderSystem::get_geometry_pass);
        render_system["get_render_graph"] = PickNonConstOverload<>(&RenderSystem::get_render_graph);
        render_system["has_cubemap"] = &RenderSystem::has_cubemap;
        render_system["get_cubemap"] = [](RenderSystem const& r) { return &r.get_cubemap(); };
        render_system["set_cubemap"] = [](RenderSystem& r, Cubemap& c) { r.set_cubemap(std::move(c)); };
#if defined(XEN_USE_XR)
        render_system["enable_xr"] = [](RenderSystem& r, XrSystem* x) { r.enable_xr(*x); };
#endif
#if !defined(XEN_NO_WINDOW)
        render_system["create_window"] = sol::overload(
            [](RenderSystem& r, Vector2ui const& size) { r.create_window(size); },
            [](RenderSystem& r, Vector2ui const& size, const std::string& t) { r.create_window(size, t); },
            [](RenderSystem& r, Vector2ui const& size, const std::string& t, WindowSetting s) {
                r.create_window(size, t, s);
            },
            PickOverload<Vector2ui const&, const std::string&, WindowSetting, uint8_t>(&RenderSystem::create_window)
        );
#endif
        render_system["resize_viewport"] = &RenderSystem::resize_viewport;
        render_system["update_lights"] = &RenderSystem::update_lights;
        render_system["update_shaders"] = &RenderSystem::update_shaders;
        render_system["update_materials"] = sol::overload(
            [](RenderSystem const& r) { r.update_materials(); },
            [](RenderSystem const& r, MeshRenderer const* m) { r.update_materials(*m); }
        );
        render_system["save_to_image"] = sol::overload(
            [](RenderSystem const& r, FilePath const& p) { r.save_to_image(p); },
            [](RenderSystem const& r, FilePath const& p, TextureFormat f) { r.save_to_image(p, f); },
            PickOverload<FilePath const&, TextureFormat, PixelDataType>(&RenderSystem::save_to_image)
        );
        render_system["remove_cubemap"] = &RenderSystem::remove_cubemap;

        state.new_enum<TextureFormat>(
            "TextureFormat", {{"RED", TextureFormat::RED},
                              {"GREEN", TextureFormat::GREEN},
                              {"BLUE", TextureFormat::BLUE},
                              {"RG", TextureFormat::RG},
                              {"RGB", TextureFormat::RGB},
                              {"BGR", TextureFormat::BGR},
                              {"RGBA", TextureFormat::RGBA},
                              {"BGRA", TextureFormat::BGRA},
                              {"SRGB", TextureFormat::SRGB},
                              {"DEPTH", TextureFormat::DEPTH},
                              {"STENCIL", TextureFormat::STENCIL},
                              {"DEPTH_STENCIL", TextureFormat::DEPTH_STENCIL}}
        );

        state.new_enum<PixelDataType>(
            "PixelDataType", {{"UBYTE", PixelDataType::UBYTE}, {"FLOAT", PixelDataType::FLOAT}}
        );
    }
}

}
