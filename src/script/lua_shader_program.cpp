#include <render/shader/shader_program.hpp>
#include <script/lua_wrapper.hpp>
#include <utils/type_utils.hpp>

#define SOL_ALL_SAFETIES_ON 1
#include "sol/sol.hpp"

namespace xen {

using namespace TypeUtils;

void LuaWrapper::register_shader_program_types()
{
    sol::state& state = get_state();

#if !defined(USE_WEBGL)
    {
        sol::usertype<ComputeShaderProgram> compute_shader_program = state.new_usertype<ComputeShaderProgram>(
            "ComputeShaderProgram", sol::constructors<ComputeShaderProgram()>(), sol::base_classes,
            sol::bases<ShaderProgram>()
        );
        compute_shader_program["get_shader"] = [](ComputeShaderProgram const& p) { return &p.get_shader(); };
        compute_shader_program["set_shader"] = [](ComputeShaderProgram& p, ComputeShader& s) {
            p.set_shader(std::move(s));
        };
        compute_shader_program["clone"] = &ComputeShaderProgram::clone;
        compute_shader_program["execute"] = &ComputeShaderProgram::execute;
        compute_shader_program["destroy_shader"] = &ComputeShaderProgram::destroy_shader;
    }
#endif

    {
        sol::usertype<RenderShaderProgram> render_shader_program = state.new_usertype<RenderShaderProgram>(
            "RenderShaderProgram", sol::constructors<RenderShaderProgram()>(), sol::base_classes,
            sol::bases<ShaderProgram>()
        );
        render_shader_program["get_vertex_shader"] = [](RenderShaderProgram const& p) {
            return &p.get_vertex_shader();
        };
#if !defined(USE_OPENGL_ES)
        render_shader_program["has_tessellation_control_shader"] =
            &RenderShaderProgram::has_tessellation_control_shader;
        render_shader_program["get_tessellation_control_shader"] = [](RenderShaderProgram const& p) {
            return &p.get_tessellation_control_shader();
        };
        render_shader_program["has_tessellation_evaluation_shader"] =
            &RenderShaderProgram::has_tessellation_evaluation_shader;
        render_shader_program["get_tessellation_evaluation_shader"] = [](RenderShaderProgram const& p) {
            return &p.get_tessellation_evaluation_shader();
        };
        render_shader_program["has_geometry_shader"] = &RenderShaderProgram::has_geometry_shader;
        render_shader_program["get_geometry_shader"] = [](RenderShaderProgram const& p) {
            return &p.get_geometry_shader();
        };
#endif
        render_shader_program["get_fragment_shader"] = [](const RenderShaderProgram& p) {
            return &p.get_fragment_shader();
        };
        render_shader_program["set_vertex_shader"] = [](RenderShaderProgram& p, VertexShader& s) {
            p.set_vertex_shader(std::move(s));
        };
#if !defined(USE_OPENGL_ES)
        render_shader_program["set_tessellation_control_shader"] = [](RenderShaderProgram& p,
                                                                      TessellationControlShader& s) {
            p.set_tessellation_control_shader(std::move(s));
        };
        render_shader_program["set_tessellation_evaluation_shader"] = [](RenderShaderProgram& p,
                                                                         TessellationEvaluationShader& s) {
            p.set_tessellation_evaluation_shader(std::move(s));
        };
        render_shader_program["set_geometry_shader"] = [](RenderShaderProgram& p, GeometryShader& s) {
            p.set_geometry_shader(std::move(s));
        };
#endif
        render_shader_program["set_fragment_shader"] = [](RenderShaderProgram& p, FragmentShader& s) {
            p.set_fragment_shader(std::move(s));
        };
        render_shader_program["set_shaders"] = sol::overload(
            [](RenderShaderProgram& p, VertexShader& v, FragmentShader& f) {
                p.set_shaders(std::move(v), std::move(f));
            }
#if !defined(USE_OPENGL_ES)
            ,
            [](RenderShaderProgram& p, VertexShader& v, GeometryShader& g, FragmentShader& f) {
                p.set_shaders(std::move(v), std::move(g), std::move(f));
            },
            [](RenderShaderProgram& p, VertexShader& v, TessellationEvaluationShader& te, FragmentShader& f) {
                p.set_shaders(std::move(v), std::move(te), std::move(f));
            },
            [](RenderShaderProgram& p, VertexShader& v, TessellationControlShader& tc, TessellationEvaluationShader& te,
               FragmentShader& f) { p.set_shaders(std::move(v), std::move(tc), std::move(te), std::move(f)); }
#endif
        );
        render_shader_program["clone"] = &RenderShaderProgram::clone;
        render_shader_program["destroy_vertex_shader"] = &RenderShaderProgram::destroy_vertex_shader;
#if !defined(USE_OPENGL_ES)
        render_shader_program["destroy_tessellation_control_shader"] =
            &RenderShaderProgram::destroy_tessellation_control_shader;
        render_shader_program["destroy_tessellation_evaluation_shader"] =
            &RenderShaderProgram::destroy_tessellation_evaluation_shader;
        render_shader_program["destroy_geometry_shader"] = &RenderShaderProgram::destroy_geometry_shader;
#endif
        render_shader_program["destroy_fragment_shader"] = &RenderShaderProgram::destroy_fragment_shader;
    }

    {
        sol::table image_texture_usage = state["ImageTextureUsage"].get_or_create<sol::table>();
        image_texture_usage["READ"] = ImageTextureUsage::READ;
        image_texture_usage["WRITE"] = ImageTextureUsage::WRITE;
        image_texture_usage["READ_WRITE"] = ImageTextureUsage::READ_WRITE;

        sol::usertype<ShaderProgram> shader_program =
            state.new_usertype<ShaderProgram>("ShaderProgram", sol::no_constructor);
        shader_program["has_attribute"] = [](ShaderProgram const& p, std::string const& n) {
            return p.has_attribute(n);
        };
        shader_program["get_attribute_count"] = &ShaderProgram::get_attribute_count;
        shader_program["has_texture"] = sol::overload(
            PickOverload<Texture const&>(&ShaderProgram::has_texture),
            PickOverload<std::string const&>(&ShaderProgram::has_texture)
        );
        shader_program["get_texture_count"] = &ShaderProgram::get_texture_count;
        shader_program["get_texture"] = sol::overload(
            [](ShaderProgram const& p, size_t i) { return &p.get_texture(i); },
            [](ShaderProgram const& p, std::string const& n) { return &p.get_texture(n); }
        );
#if !defined(USE_WEBGL)
        shader_program["has_image_texture"] = sol::overload(
            PickOverload<const Texture&>(&ShaderProgram::has_image_texture),
            PickOverload<const std::string&>(&ShaderProgram::has_image_texture)
        );
        shader_program["get_image_texture_count"] = &ShaderProgram::get_image_texture_count;
        shader_program["get_image_texture"] = sol::overload(
            [](ShaderProgram const& p, size_t i) { return &p.get_image_texture(i); },
            [](ShaderProgram const& p, std::string const& n) { return &p.get_image_texture(n); }
        );
#endif
        shader_program["set_int_attribute"] = &ShaderProgram::set_attribute<int>;
        shader_program["set_uint_attribute"] = &ShaderProgram::set_attribute<uint32_t>;
        shader_program["set_float_attribute"] = &ShaderProgram::set_attribute<float>;
        shader_program
            ["set_attribute"] =
                sol::
                    overload(&ShaderProgram::set_attribute<Vector2i const&>, &ShaderProgram::set_attribute<Vector3i const&>, &ShaderProgram::set_attribute<Vector4i const&>, &ShaderProgram::set_attribute<Vector2ui const&>, &ShaderProgram::set_attribute<Vector3ui const&>, &ShaderProgram::set_attribute<Vector4ui const&>, &ShaderProgram::set_attribute<Vector2f const&>, &ShaderProgram::set_attribute<Vector3f const&>, &ShaderProgram::set_attribute<Vector4f const&>, &ShaderProgram::set_attribute<Matrix2 const&>, &ShaderProgram::set_attribute<Matrix3 const&>, &ShaderProgram::set_attribute<Matrix4 const&>);
        // Sol does not seem to be able to bind shared pointers from derived classes to a shared pointer of the base
        // class
        //   (e.g., Texture2DPtr cannot be given directly to set_texture(), which takes a TexturePtr)
        shader_program["set_texture"] = sol::overload(
#if !defined(USE_OPENGL_ES)
            [](ShaderProgram& p, Texture1DPtr t, const std::string& n) { p.set_texture(std::move(t), n); },
#endif
            [](ShaderProgram& p, Texture2DPtr t, const std::string& n) { p.set_texture(std::move(t), n); },
            [](ShaderProgram& p, Texture3DPtr t, const std::string& n) { p.set_texture(std::move(t), n); }
        );
#if !defined(USE_WEBGL)
        shader_program["set_image_texture"] = sol::overload(
#if !defined(USE_OPENGL_ES)
            [](ShaderProgram& p, Texture1DPtr t, const std::string& n) {
                p.set_image_texture(std::move(t), n, ImageTextureUsage::READ_WRITE);
            },
            [](ShaderProgram& p, Texture1DPtr t, const std::string& n, ImageTextureUsage u) {
                p.set_image_texture(std::move(t), n, u);
            },
#endif
            [](ShaderProgram& p, Texture2DPtr t, const std::string& n) {
                p.set_image_texture(std::move(t), n, ImageTextureUsage::READ_WRITE);
            },
            [](ShaderProgram& p, Texture2DPtr t, const std::string& n, ImageTextureUsage u) {
                p.set_image_texture(std::move(t), n, u);
            },
            [](ShaderProgram& p, Texture3DPtr t, const std::string& n) {
                p.set_image_texture(std::move(t), n, ImageTextureUsage::READ_WRITE);
            },
            [](ShaderProgram& p, Texture3DPtr t, const std::string& n, ImageTextureUsage u) {
                p.set_image_texture(std::move(t), n, u);
            }
        );
#endif
        shader_program["load_shaders"] = &ShaderProgram::load_shaders;
        shader_program["compile_shaders"] = &ShaderProgram::compile_shaders;
        shader_program["link"] = &ShaderProgram::link;
        shader_program["is_linked"] = &ShaderProgram::is_linked;
        shader_program["update_shaders"] = &ShaderProgram::update_shaders;
        shader_program["use"] = &ShaderProgram::use;
        shader_program["is_used"] = &ShaderProgram::is_used;
        shader_program["send_attributes"] = &ShaderProgram::send_attributes;
        shader_program["remove_attribute"] = &ShaderProgram::remove_attribute;
        shader_program["clear_attributes"] = &ShaderProgram::clear_attributes;
        shader_program["init_textures"] = &ShaderProgram::init_textures;
        shader_program["bind_textures"] = &ShaderProgram::bind_textures;
        shader_program["remove_texture"] = sol::overload(
            PickOverload<Texture const&>(&ShaderProgram::remove_texture),
            PickOverload<std::string const&>(&ShaderProgram::remove_texture)
        );
        shader_program["clear_textures"] = &ShaderProgram::clear_textures;
#if !defined(USE_WEBGL)
        shader_program["init_image_textures"] = &ShaderProgram::init_image_textures;
        shader_program["bind_image_textures"] = &ShaderProgram::bind_image_textures;
        shader_program["remove_image_texture"] = sol::overload(
            PickOverload<Texture const&>(&ShaderProgram::remove_image_texture),
            PickOverload<std::string const&>(&ShaderProgram::remove_image_texture)
        );
        shader_program["clear_image_textures"] = &ShaderProgram::clear_image_textures;
#endif
        shader_program["recover_uniform_location"] = &ShaderProgram::recover_uniform_location;
    }
}

}
