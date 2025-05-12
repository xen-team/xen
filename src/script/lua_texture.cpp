#include <data/image.hpp>
#include <render/texture.hpp>
#include <script/lua_wrapper.hpp>
#include <utils/type_utils.hpp>

#define SOL_ALL_SAFETIES_ON 1
#include "sol/sol.hpp"

namespace xen {

using namespace TypeUtils;

void LuaWrapper::register_texture_types()
{
    sol::state& state = get_state();

    {
        sol::usertype<Texture> texture = state.new_usertype<Texture>("Texture", sol::no_constructor);
        texture["get_index"] = &Texture::get_index;
        texture["get_colorspace"] = &Texture::get_colorspace;
        texture["get_data_type"] = &Texture::get_data_type;
        texture["bind"] = &Texture::bind;
        texture["unbind"] = &Texture::unbind;
        texture["set_filter"] = sol::overload(
            PickOverload<TextureFilter>(&Texture::set_filter),
            PickOverload<TextureFilter, TextureFilter>(&Texture::set_filter),
            PickOverload<TextureFilter, TextureFilter, TextureFilter>(&Texture::set_filter)
        );
        texture["set_wrapping"] = &Texture::set_wrapping;
        texture["set_colorspace"] = sol::overload(
            PickOverload<TextureColorspace>(&Texture::set_colorspace),
            PickOverload<TextureColorspace, TextureDataType>(&Texture::set_colorspace)
        );
    }

#if !defined(USE_OPENGL_ES)
    {
        sol::usertype<Texture1D> texture1D = state.new_usertype<Texture1D>(
            "Texture1D",
            sol::constructors<
                Texture1D(TextureColorspace), Texture1D(TextureColorspace, TextureDataType),
                Texture1D(uint, TextureColorspace), Texture1D(uint, TextureColorspace, TextureDataType),
                Texture1D(Color const&), Texture1D(Color const&, uint)>(),
            sol::base_classes, sol::bases<Texture>()
        );
        texture1D["get_width"] = &Texture1D::get_width;
        texture1D
            ["create"] = sol::overload(&Texture1D::create<TextureColorspace>, &Texture1D::create<TextureColorspace, TextureDataType>, &Texture1D::create<uint, TextureColorspace>, &Texture1D::create<uint, TextureColorspace, TextureDataType>, &Texture1D::create<Color const&>, &Texture1D::create<Color const&, uint>);
        texture1D["resize"] = &Texture1D::resize;
        texture1D["fill"] = &Texture1D::fill;
    }
#endif

    {
        sol::usertype<Texture2D> texture2D = state.new_usertype<Texture2D>(
            "Texture2D",
            sol::constructors<
                Texture2D(TextureColorspace), Texture2D(TextureColorspace, TextureDataType),
                Texture2D(Vector2ui const&, TextureColorspace),
                Texture2D(Vector2ui const&, TextureColorspace, TextureDataType), Texture2D(Image const&),
                Texture2D(Image const&, bool), Texture2D(Image const&, bool, bool), Texture2D(Color const&),
                Texture2D(Color const&, Vector2ui)>(),
            sol::base_classes, sol::bases<Texture>()
        );
        texture2D["get_size"] = &Texture2D::get_size;
        texture2D["get_width"] = &Texture2D::get_width;
        texture2D["get_height"] = &Texture2D::get_height;
        texture2D["create"] =
            sol::overload(&Texture2D::create<TextureColorspace>, &Texture2D::create<TextureColorspace, TextureDataType>, &Texture2D::create<Vector2ui const&, TextureColorspace>, &Texture2D::create<Vector2ui const&, TextureColorspace, TextureDataType>, &Texture2D::create<Image const&>, &Texture2D::create<Image const&, bool>, &Texture2D::create<Image const&, bool, bool>, &Texture2D::create<Color const&>, &Texture2D::create<Color const&, Vector2ui const&>);
        texture2D["resize"] = &Texture2D::resize;
        texture2D["load"] = sol::overload(
            [](Texture2D& t, Image const& image) { t.load(image); },
            [](Texture2D& t, Image const& image, bool mips) { t.load(image, mips); },
            PickOverload<Image const&, bool, bool>(&Texture2D::load)
        );
        texture2D["fill"] = &Texture2D::fill;
        texture2D["recover_image"] = &Texture2D::recover_image;
    }

    {
        sol::usertype<Texture3D> texture3D = state.new_usertype<Texture3D>(
            "Texture3D",
            sol::constructors<
                Texture3D(TextureColorspace), Texture3D(TextureColorspace, TextureDataType),
                Texture3D(Vector3ui const&, TextureColorspace),
                Texture3D(Vector3ui const&, TextureColorspace, TextureDataType),
                // Constant references on vectors cannot be bound, and declaring
                //  constructors requires the exact prototype
                // Texture3D(const std::vector<Image>&),
                // Texture3D(const std::vector<Image>&, bool),
                // Texture3D(const std::vector<Image>&, bool, bool),
                Texture3D(Color const&, Vector3ui)>(),
            sol::base_classes, sol::bases<Texture>()
        );
        texture3D["get_size"] = &Texture3D::get_size;
        texture3D["get_width"] = &Texture3D::get_width;
        texture3D["get_height"] = &Texture3D::get_height;
        texture3D["get_depth"] = &Texture3D::get_depth;
        texture3D["create"]    = sol::overload(&Texture3D::create<TextureColorspace>,
                                           &Texture3D::create<TextureColorspace, TextureDataType>,
                                           &Texture3D::create<Vector3ui const&, TextureColorspace>,
                                           &Texture3D::create<Vector3ui const&, TextureColorspace, TextureDataType>,
                                           [] (std::vector<Image> images) { return Texture3D::create(images); },
                                           [] (std::vector<Image> images, bool mips) { return Texture3D::create(images, mips); },
                                           [] (std::vector<Image> images, bool mips, bool srgb) { return Texture3D::create(images, mips, srgb); },
                                           &Texture3D::create<Color const&>,
                                           &Texture3D::create<Color const&, Vector3ui const&>);
        texture3D["resize"] = &Texture3D::resize;
        texture3D["load"] = sol::overload(
            [](Texture3D& t, std::vector<Image> images) { t.load(images); },
            [](Texture3D& t, std::vector<Image> images, bool mips) { t.load(images, mips); },
            [](Texture3D& t, std::vector<Image> images, bool mips, bool srgb) { t.load(images, mips, srgb); }
        );
        texture3D["fill"] = &Texture3D::fill;
    }

    state.new_enum<TextureColorspace>(
        "TextureColorspace", {{"INVALID", TextureColorspace::INVALID},
                              {"GRAY", TextureColorspace::GRAY},
                              {"RG", TextureColorspace::RG},
                              {"RGB", TextureColorspace::RGB},
                              {"RGBA", TextureColorspace::RGBA},
                              {"SRGB", TextureColorspace::SRGB},
                              {"SRGBA", TextureColorspace::SRGBA},
                              {"DEPTH", TextureColorspace::DEPTH}}
    );

    state.new_enum<TextureDataType>(
        "TextureDataType",
        {{"BYTE", TextureDataType::BYTE}, {"FLOAT16", TextureDataType::FLOAT16}, {"FLOAT32", TextureDataType::FLOAT32}}
    );

    state.new_enum<TextureFilter>(
        "TextureFilter", {{"NEAREST", TextureFilter::NEAREST}, {"LINEAR", TextureFilter::LINEAR}}
    );

    state.new_enum<TextureWrapping>(
        "TextureWrapping", {{"REPEAT", TextureWrapping::REPEAT}, {"CLAMP", TextureWrapping::CLAMP}}
    );
}
}