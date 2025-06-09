#include <data/image.hpp>
#include <math/transform/transform.hpp>
#include <render/camera.hpp>
#include <render/graphic_objects.hpp>
#include <render/light.hpp>
#include <render/mesh_renderer.hpp>
#include <render/render_system.hpp>
#include <script/lua_wrapper.hpp>
#include <utils/type_utils.hpp>

#define SOL_ALL_SAFETIES_ON 1
#include "sol/sol.hpp"

namespace xen {

using namespace TypeUtils;

void LuaWrapper::register_render_types()
{
    sol::state& state = get_state();

    {
        sol::usertype<Camera> camera = state.new_usertype<Camera>(
            "Camera",
            sol::constructors<
                Camera(Vector2ui const&), Camera(Vector2ui const&, Vector3f),
                Camera(Vector2ui const&, Vector3f, Radiansf), Camera(Vector2ui const&, Vector3f, Radiansf, float),
                Camera(Vector2ui const&, Vector3f, Radiansf, float, float),
                Camera(Vector2ui const&, Vector3f, Radiansf, float, float, ProjectionType)>(),
            sol::base_classes, sol::bases<Component>()
        );
        camera["fov"] = sol::property(&Camera::get_fov, &Camera::set_fov);
        camera["orthographic_bound"] = sol::property(&Camera::get_orthographic_bound, &Camera::set_orthographic_bound);
        camera["camera_type"] = sol::property(&Camera::get_camera_type, &Camera::set_camera_type);
        camera["get_view"] = &Camera::get_view;
        camera["get_inverse_view"] = &Camera::get_inverse_view;
        camera["get_projection"] = &Camera::get_projection;
        camera["get_inverse_projection"] = &Camera::get_inverse_projection;
        camera["set_projection_type"] = &Camera::set_projection_type;
        camera["set_target"] = &Camera::set_target;
        camera["compute_view"] = &Camera::compute_view;
        camera["compute_look_at"] = &Camera::compute_look_at;
        camera["compute_inverse_view"] = &Camera::compute_inverse_view;
        camera["compute_perspective"] = &Camera::compute_perspective;
        camera["compute_orthographic"] = sol::overload(
            PickOverload<float, float, float, float, float, float>(&Camera::compute_orthographic),
            PickOverload<>(&Camera::compute_orthographic)
        );
        camera["compute_projection"] = &Camera::compute_projection;
        camera["compute_inverse_projection"] = &Camera::compute_inverse_projection;
        camera["resize_viewport"] = &Camera::resize_viewport;
        camera["unproject"] = sol::overload(
            PickOverload<Vector4f const&>(&Camera::unproject), PickOverload<Vector3f const&>(&Camera::unproject),
            PickOverload<Vector2f const&>(&Camera::unproject)
        );

        state.new_enum<CameraType>(
            "CameraType", {{"FREE_FLY", CameraType::FREE_FLY}, {"LOOK_AT", CameraType::LOOK_AT}}
        );

        state.new_enum<ProjectionType>(
            "ProjectionType",
            {{"PERSPECTIVE", ProjectionType::PERSPECTIVE}, {"ORTHOGRAPHIC", ProjectionType::ORTHOGRAPHIC}}
        );
    }

    {
        sol::usertype<Cubemap> cubemap = state.new_usertype<Cubemap>(
            "Cubemap",
            sol::constructors<
                Cubemap(), Cubemap(Image const&, Image const&, Image const&, Image const&, Image const&, Image const&)>(
            )
        );
        cubemap["get_index"] = &Cubemap::get_index;
        cubemap["get_program"] = [](Cubemap const& c) { return &c.get_program(); };
        cubemap["load"] = &Cubemap::load;
        cubemap["bind"] = &Cubemap::bind;
        cubemap["unbind"] = &Cubemap::unbind;
        cubemap["draw"] = &Cubemap::draw;
    }

    {
        sol::usertype<Framebuffer> framebuffer =
            state.new_usertype<Framebuffer>("Framebuffer", sol::constructors<Framebuffer()>());
        framebuffer["get_index"] = &Framebuffer::get_index;
        framebuffer["empty"] = &Framebuffer::empty;
        framebuffer["has_depth_buffer"] = &Framebuffer::has_depth_buffer;
        framebuffer["get_depth_buffer"] = [](Framebuffer const& f) { return &f.get_depth_buffer(); };
        framebuffer["get_color_buffer_count"] = &Framebuffer::get_color_buffer_count;
        framebuffer["get_color_buffer"] = [](Framebuffer const& f, size_t i) { return &f.get_color_buffer(i); };
        framebuffer["recover_vertex_shader"] = &Framebuffer::recover_vertex_shader;
        framebuffer["set_depth_buffer"] = &Framebuffer::set_depth_buffer;
        framebuffer["add_color_buffer"] = &Framebuffer::add_color_buffer;
        framebuffer["remove_texture_buffer"] = &Framebuffer::remove_texture_buffer;
        framebuffer["clear_depth_buffer"] = &Framebuffer::clear_depth_buffer;
        framebuffer["clear_color_buffers"] = &Framebuffer::clear_color_buffers;
        framebuffer["clear_texture_buffers"] = &Framebuffer::clear_texture_buffers;
        framebuffer["resize_buffers"] = &Framebuffer::resize_buffers;
        framebuffer["map_buffers"] = &Framebuffer::map_buffers;
        framebuffer["bind"] = &Framebuffer::bind;
        framebuffer["unbind"] = &Framebuffer::unbind;
        framebuffer["display"] = &Framebuffer::display;
    }

    // GraphicObjects
    {
        {
            sol::usertype<IndexBuffer> index_buffer =
                state.new_usertype<IndexBuffer>("IndexBuffer", sol::constructors<IndexBuffer()>());
            index_buffer["get_index"] = &IndexBuffer::get_index;
            index_buffer["bind"] = &IndexBuffer::bind;
            index_buffer["unbind"] = &IndexBuffer::unbind;
            index_buffer["line_index_count"] = &IndexBuffer::line_index_count;
            index_buffer["triangle_index_count"] = &IndexBuffer::triangle_index_count;
        }

        {
            sol::usertype<VertexArray> vertexArray =
                state.new_usertype<VertexArray>("VertexArray", sol::constructors<VertexArray()>());
            vertexArray["get_index"] = &VertexArray::get_index;
            vertexArray["bind"] = &VertexArray::bind;
            vertexArray["unbind"] = &VertexArray::unbind;
        }

        {
            sol::usertype<VertexBuffer> vertex_buffer =
                state.new_usertype<VertexBuffer>("VertexBuffer", sol::constructors<VertexBuffer()>());
            vertex_buffer["get_index"] = &VertexBuffer::get_index;
            vertex_buffer["bind"] = &VertexBuffer::bind;
            vertex_buffer["unbind"] = &VertexBuffer::unbind;
            vertex_buffer["vertex_count"] = &VertexBuffer::vertex_count;
        }
    }

    {
        sol::usertype<Light> light = state.new_usertype<Light>(
            "Light",
            sol::constructors<
                Light(LightType, float), Light(LightType, float, Color const&),
                Light(LightType, Vector3f const&, float), Light(LightType, Vector3f const&, float, Color const&),
                Light(LightType, Vector3f const&, float, Radiansf),
                Light(LightType, Vector3f const&, float, Radiansf, Color const&)>(),
            sol::base_classes, sol::bases<Component>()
        );
        light["type"] = sol::property(&Light::get_type, &Light::set_type);
        light["direction"] = sol::property(&Light::get_direction, &Light::set_direction);
        light["energy"] = sol::property(&Light::get_energy, &Light::set_energy);
        light["color"] = sol::property(&Light::get_color, &Light::set_color);
        light["angle"] = sol::property(&Light::get_angle, &Light::set_angle);

        state.new_enum<LightType>(
            "LightType",
            {{"POINT", LightType::POINT}, {"DIRECTIONAL", LightType::DIRECTIONAL}, {"SPOT", LightType::SPOT}}
        );
    }

    {
        sol::usertype<Material> material =
            state.new_usertype<Material>("Material", sol::constructors<Material(), Material(MaterialType)>());
        material["get_program"] = PickNonConstOverload<>(&Material::get_program);
        material["empty"] = &Material::empty;
        material["clone"] = &Material::clone;
        material["load_type"] = &Material::load_type;

        state.new_enum<std::string>(
            "MaterialAttribute", {{"BaseColor", MaterialAttribute::BaseColor},
                                  {"Emissive", MaterialAttribute::Emissive},
                                  {"Metallic", MaterialAttribute::Metallic},
                                  {"Roughness", MaterialAttribute::Roughness},
                                  {"Sheen", MaterialAttribute::Sheen},
                                  {"Ambient", MaterialAttribute::Ambient},
                                  {"Specular", MaterialAttribute::Specular},
                                  {"Opacity", MaterialAttribute::Opacity}}
        );

        state.new_enum<std::string>(
            "MaterialTexture", {{"BaseColor", MaterialTexture::BaseColor},
                                {"Emissive", MaterialTexture::Emissive},
                                {"Ambient", MaterialTexture::Ambient},
                                {"Normal", MaterialTexture::Normal},
                                {"Metallic", MaterialTexture::Metallic},
                                {"Roughness", MaterialTexture::Roughness},
                                {"Sheen", MaterialTexture::Sheen},
                                {"Specular", MaterialTexture::Specular},
                                {"Opacity", MaterialTexture::Opacity},
                                {"Bump", MaterialTexture::Bump}}
        );

        state.new_enum<MaterialType>(
            "MaterialType", {{"COOK_TORRANCE", MaterialType::COOK_TORRANCE},
                             {"BLINN_PHONG", MaterialType::BLINN_PHONG},
                             {"SINGLE_TEXTURE_2D", MaterialType::SINGLE_TEXTURE_2D},
                             {"SINGLE_TEXTURE_3D", MaterialType::SINGLE_TEXTURE_3D}}
        );
    }

    {
        sol::usertype<UniformBuffer> uniform_buffer = state.new_usertype<UniformBuffer>(
            "UniformBuffer", sol::constructors<UniformBuffer(uint32_t), UniformBuffer(uint32_t, UniformBufferUsage)>()
        );
        uniform_buffer["get_index"] = &UniformBuffer::get_index;
        uniform_buffer["bind_uniform_block"] = sol::overload(
            PickOverload<ShaderProgram const&, uint32_t, uint32_t>(&UniformBuffer::bind_uniform_block),
            PickOverload<ShaderProgram const&, std::string const&, uint32_t>(&UniformBuffer::bind_uniform_block)
        );
        uniform_buffer["bind_base"] = &UniformBuffer::bind_base;
        uniform_buffer["bind_range"] = &UniformBuffer::bind_range;
        uniform_buffer["bind"] = &UniformBuffer::bind;
        uniform_buffer["unbind"] = &UniformBuffer::unbind;
        // uniform_buffer["send_int_data"] = &UniformBuffer::send_data<int>;
        // uniform_buffer["send_uint_data"] = &UniformBuffer::send_data<uint32_t>;
        // uniform_buffer["send_float_data"] = &UniformBuffer::send_data<float>;
        // uniform_buffer
        // ["send_data"] = sol::overload(&UniformBuffer::send_data<Vector2i const&>, &UniformBuffer::send_data<Vector3i
        // const&>, &UniformBuffer::send_data<Vector4i const&>, &UniformBuffer::send_data<Vector2ui const&>,
        // &UniformBuffer::send_data<Vector3ui const&>, &UniformBuffer::send_data<Vector4ui const&>,
        // &UniformBuffer::send_data<Vector2f const&>, &UniformBuffer::send_data<Vector3f const&>,
        // &UniformBuffer::send_data<Vector4f const&>, &UniformBuffer::send_data<Matrix2 const&>,
        // &UniformBuffer::send_data<Matrix3 const&>, &UniformBuffer::send_data<Matrix4 const&>);

        state.new_enum<UniformBufferUsage>(
            "UniformBufferUsage", {{"STATIC", UniformBufferUsage::STATIC},
                                   {"DYNAMIC", UniformBufferUsage::DYNAMIC},
                                   {"STREAM", UniformBufferUsage::STREAM}}
        );
    }
}

}
