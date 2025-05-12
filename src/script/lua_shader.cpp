#include <render/shader/shader.hpp>
#include <script/lua_wrapper.hpp>
#include <utils/type_utils.hpp>

#define SOL_ALL_SAFETIES_ON 1
#include "sol/sol.hpp"

namespace xen {
using namespace TypeUtils;

void LuaWrapper::register_shader_types()
{
    sol::state& state = get_state();

#if !defined(USE_WEBGL)
    {
        sol::usertype<ComputeShader> compute_shader = state.new_usertype<ComputeShader>(
            "ComputeShader", sol::constructors<ComputeShader(), ComputeShader(FilePath)>(), sol::base_classes,
            sol::bases<Shader>()
        );
        compute_shader["load_from_source"] = sol::overload(
            PickOverload<std::string const&>(&ComputeShader::load_from_source),
            PickOverload<char const*>(&ComputeShader::load_from_source)
        );
        compute_shader["clone"] = &ComputeShader::clone;
    }
#endif

    {
        sol::usertype<FragmentShader> fragment_shader = state.new_usertype<FragmentShader>(
            "FragmentShader", sol::constructors<FragmentShader(), FragmentShader(FilePath)>(), sol::base_classes,
            sol::bases<Shader>()
        );
        fragment_shader["load_from_source"] = sol::overload(
            PickOverload<std::string const&>(&FragmentShader::load_from_source),
            PickOverload<char const*>(&FragmentShader::load_from_source)
        );
        fragment_shader["clone"] = &FragmentShader::clone;
    }

#if !defined(USE_OPENGL_ES)
    {
        sol::usertype<GeometryShader> geometry_shader = state.new_usertype<GeometryShader>(
            "GeometryShader", sol::constructors<GeometryShader(), GeometryShader(FilePath)>(), sol::base_classes,
            sol::bases<Shader>()
        );
        geometry_shader["load_from_source"] = sol::overload(
            PickOverload<std::string const&>(&GeometryShader::load_from_source),
            PickOverload<char const*>(&GeometryShader::load_from_source)
        );
        geometry_shader["clone"] = &GeometryShader::clone;
    }
#endif

    {
        sol::usertype<Shader> shader = state.new_usertype<Shader>("Shader", sol::no_constructor);
        shader["get_index"] = &Shader::get_index;
        shader["get_path"] = &Shader::get_path;
        shader["is_valid"] = &Shader::is_valid;
        shader["import"] = &Shader::import;
        shader["load"] = &Shader::load;
        shader["compile"] = &Shader::compile;
        shader["is_compiled"] = &Shader::is_compiled;
        shader["destroy"] = &Shader::destroy;
    }

#if !defined(USE_OPENGL_ES)
    {
        sol::usertype<TessellationControlShader> tess_ctrl_shader = state.new_usertype<TessellationControlShader>(
            "TessellationControlShader",
            sol::constructors<TessellationControlShader(), TessellationControlShader(FilePath)>(), sol::base_classes,
            sol::bases<Shader>()
        );
        tess_ctrl_shader["load_from_source"] = sol::overload(
            PickOverload<std::string const&>(&TessellationControlShader::load_from_source),
            PickOverload<char const*>(&TessellationControlShader::load_from_source)
        );
        tess_ctrl_shader["clone"] = &TessellationControlShader::clone;
    }

    {
        sol::usertype<TessellationEvaluationShader> tess_eval_shader = state.new_usertype<TessellationEvaluationShader>(
            "TessellationEvaluationShader",
            sol::constructors<TessellationEvaluationShader(), TessellationEvaluationShader(FilePath)>(),
            sol::base_classes, sol::bases<Shader>()
        );
        tess_eval_shader["load_from_source"] = sol::overload(
            PickOverload<std::string const&>(&TessellationEvaluationShader::load_from_source),
            PickOverload<char const*>(&TessellationEvaluationShader::load_from_source)
        );
        tess_eval_shader["clone"] = &TessellationEvaluationShader::clone;
    }
#endif

    {
        sol::usertype<VertexShader> vertex_shader = state.new_usertype<VertexShader>(
            "VertexShader", sol::constructors<VertexShader(), VertexShader(FilePath)>(), sol::base_classes,
            sol::bases<Shader>()
        );
        vertex_shader["load_from_source"] = sol::overload(
            PickOverload<std::string const&>(&VertexShader::load_from_source),
            PickOverload<char const*>(&VertexShader::load_from_source)
        );
        vertex_shader["clone"] = &VertexShader::clone;
    }
}

}
