#include <data/image.hpp>

#include <script/lua_wrapper.hpp>
#include <utils/type_utils.hpp>

#define SOL_ALL_SAFETIES_ON 1
#include "sol/sol.hpp"

namespace xen {

using namespace TypeUtils;

void LuaWrapper::register_image_types()
{
    sol::state& state = get_state();

    {
        sol::usertype<Image> image = state.new_usertype<Image>(
            "Image",
            sol::constructors<
                Image(), Image(ImageColorspace), Image(ImageColorspace, ImageDataType),
                Image(Vector2ui const&, ImageColorspace), Image(Vector2ui const&, ImageColorspace, ImageDataType)>()
        );
        image["get_size"] = &Image::get_size;
        image["get_width"] = &Image::get_width;
        image["get_height"] = &Image::get_height;
        image["get_colorspace"] = &Image::get_colorspace;
        image["get_data_type"] = &Image::get_data_type;
        image["get_channel_count"] = &Image::get_channel_count;
        image["empty"] = &Image::empty;
        image["recover_byte_value"] = &Image::recover_byte_value;
        image["recover_float_value"] = &Image::recover_float_value;
        image["recover_byte_pixel"] = PickOverload<size_t, size_t>(&Image::recover_pixel<uint8_t>);
        image["recover_float_pixel"] = PickOverload<size_t, size_t>(&Image::recover_pixel<float>);
        // image["recoverVector2ubPixel"] = PickOverload<size_t, size_t>(&Image::recover_pixel<uint8_t, 2>);
        // image["recoverVector3ubPixel"] = PickOverload<size_t, size_t>(&Image::recover_pixel<uint8_t, 3>);
        // image["recoverVector4ubPixel"] = PickOverload<size_t, size_t>(&Image::recover_pixel<uint8_t, 4>);
        // image["recoverVector2fPixel"] = PickOverload<size_t, size_t>(&Image::recover_pixel<float, 2>);
        // image["recoverVector3fPixel"] = PickOverload<size_t, size_t>(&Image::recover_pixel<float, 3>);
        // image["recoverVector4fPixel"] = PickOverload<size_t, size_t>(&Image::recover_pixel<float, 4>);
        image["set_byte_value"] = &Image::set_byte_value;
        image["set_float_value"] = &Image::set_float_value;
        image["set_byte_pixel"] = PickOverload<size_t, size_t, uint8_t>(&Image::set_pixel<uint8_t>);
        image["set_float_pixel"] = PickOverload<size_t, size_t, float>(&Image::set_pixel<float>);
        // image["setVector2ubPixel"] = PickOverload<size_t, size_t, Vector2ub const&>(&Image::set_pixel<uint8_t, 2>);
        // image["setVector3ubPixel"] = PickOverload<size_t, size_t, Vector3ub const&>(&Image::set_pixel<uint8_t, 3>);
        // image["setVector4ubPixel"] = PickOverload<size_t, size_t, Vector4ub const&>(&Image::set_pixel<uint8_t, 4>);
        // image["setVector2fPixel"] = PickOverload<size_t, size_t, Vector2f const&>(&Image::set_pixel<float, 2>);
        // image["setVector3fPixel"] = PickOverload<size_t, size_t, Vector3f const&>(&Image::set_pixel<float, 3>);
        // image["setVector4fPixel"] = PickOverload<size_t, size_t, Vector4f const&>(&Image::set_pixel<float, 4>);

        state.new_enum<ImageColorspace>(
            "ImageColorspace", {{"GRAY", ImageColorspace::GRAY},
                                {"GRAY_ALPHA", ImageColorspace::GRAY_ALPHA},
                                {"RGB", ImageColorspace::RGB},
                                {"RGBA", ImageColorspace::RGBA},
                                {"SRGB", ImageColorspace::SRGB},
                                {"SRGBA", ImageColorspace::SRGBA}}
        );

        state.new_enum<ImageDataType>(
            "ImageDataType", {{"BYTE", ImageDataType::BYTE}, {"FLOAT", ImageDataType::FLOAT}}
        );
    }
}
}