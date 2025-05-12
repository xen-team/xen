#include "shader.hpp"

#include <render/renderer.hpp>
#include <utils/file_utils.hpp>
#include <utils/str_utils.hpp>

#include <tracy/Tracy.hpp>

namespace xen {
void Shader::import(FilePath filepath)
{
    path = std::move(filepath);
    load();
}

void Shader::load() const
{
    if (path.empty()) { // Shader imported directly from source, no path available
        return;
    }

    Log::debug("[Shader] Loading (ID: " + std::to_string(index) + ", path " + path + ")...");
    load_source(FileUtils::read_file_to_string(path));
    Log::debug("[Shader] Loaded");
}

void Shader::compile() const
{
    Log::debug("[Shader] Compiling (ID: " + std::to_string(index) + ")...");
    Renderer::compile_shader(index);
    Log::debug("[Shader] Compiled");
}

bool Shader::is_compiled() const
{
    return Renderer::is_shader_compiled(index);
}

void Shader::load_source(std::string const& source) const
{
    ZoneScopedN("Shader::load_source");

    Log::debug("[Shader] Loading source (ID: " + std::to_string(index) + ")...");

    // Removing spaces in front so that we can directly check the header tags
    std::string shader_source = StrUtils::trim_left_copy(source);

    // If the #version tag is missing, add it with the current version
    if (!shader_source.starts_with("#version")) {
        std::string header = "#version " + std::to_string(Renderer::get_major_version()) +
                             std::to_string(Renderer::get_minor_version()) + '0';

#if defined(USE_OPENGL_ES)
        header += " es";

        if (!StrUtils::starts_with(shader_source, "precision"))
            header += "\nprecision highp float;\nprecision highp int;";
#endif

        shader_source = header + "\n#line 0\n" + shader_source;
    }

    Renderer::send_shader_source(index, shader_source);

    Log::debug("[Shader] Loaded source");
}

void Shader::destroy()
{
    ZoneScopedN("Shader::destroy");

    if (!index.is_valid()) {
        return;
    }

    Log::debug("[Shader] Destroying (ID: " + std::to_string(index) + ")...");
    Renderer::delete_shader(index);
    index.reset();
    Log::debug("[Shader] Destroyed");
}

VertexShader::VertexShader()
{
    Log::debug("[Shader] Creating vertex shader...");
    index = Renderer::create_shader(ShaderType::VERTEX);
    Log::debug("[Shader] Created vertex shader (ID: " + std::to_string(index) + ")");
}

VertexShader VertexShader::load_from_source(std::string const& source)
{
    VertexShader vert_shader;
    vert_shader.load_source(source);
    return vert_shader;
}

VertexShader VertexShader::clone() const
{
    VertexShader res;

    if (!path.empty()) {
        res.import(path);
    }
    else {
        res.load_source(Renderer::recover_shader_source(index));
    }

    return res;
}

#if !defined(USE_OPENGL_ES)
TessellationControlShader::TessellationControlShader()
{
    Log::debug("[Shader] Creating tessellation control shader...");
    index = Renderer::create_shader(ShaderType::TESSELLATION_CONTROL);
    Log::debug("[Shader] Created tessellation control shader (ID: " + std::to_string(index) + ")");
}

TessellationControlShader TessellationControlShader::load_from_source(std::string const& source)
{
    TessellationControlShader tess_ctrl_shader;
    tess_ctrl_shader.load_source(source);
    return tess_ctrl_shader;
}

TessellationControlShader TessellationControlShader::clone() const
{
    TessellationControlShader res;

    if (!path.empty()) {
        res.import(path);
    }
    else {
        res.load_source(Renderer::recover_shader_source(index));
    }

    return res;
}

TessellationEvaluationShader::TessellationEvaluationShader()
{
    Log::debug("[Shader] Creating tessellation evaluation shader...");
    index = Renderer::create_shader(ShaderType::TESSELLATION_EVALUATION);
    Log::debug("[Shader] Created tessellation evaluation shader (ID: " + std::to_string(index) + ")");
}

TessellationEvaluationShader TessellationEvaluationShader::load_from_source(std::string const& source)
{
    TessellationEvaluationShader tess_eval_shader;
    tess_eval_shader.load_source(source);
    return tess_eval_shader;
}

TessellationEvaluationShader TessellationEvaluationShader::clone() const
{
    TessellationEvaluationShader res;

    if (!path.empty()) {
        res.import(path);
    }
    else {
        res.load_source(Renderer::recover_shader_source(index));
    }

    return res;
}

GeometryShader::GeometryShader()
{
    Log::debug("[Shader] Creating geometry shader...");
    index = Renderer::create_shader(ShaderType::GEOMETRY);
    Log::debug("[Shader] Created geometry shader (ID: " + std::to_string(index) + ")");
}

GeometryShader GeometryShader::load_from_source(std::string const& source)
{
    GeometryShader geom_shader;
    geom_shader.load_source(source);
    return geom_shader;
}

GeometryShader GeometryShader::clone() const
{
    GeometryShader res;

    if (!path.empty()) {
        res.import(path);
    }
    else {
        res.load_source(Renderer::recover_shader_source(index));
    }

    return res;
}
#endif

FragmentShader::FragmentShader()
{
    Log::debug("[Shader] Creating fragment shader...");
    index = Renderer::create_shader(ShaderType::FRAGMENT);
    Log::debug("[Shader] Created fragment shader (ID: " + std::to_string(index) + ")");
}

FragmentShader FragmentShader::load_from_source(std::string const& source)
{
    FragmentShader frag_shader;
    frag_shader.load_source(source);
    return frag_shader;
}

FragmentShader FragmentShader::clone() const
{
    FragmentShader res;

    if (!path.empty()) {
        res.import(path);
    }
    else {
        res.load_source(Renderer::recover_shader_source(index));
    }

    return res;
}

#if !defined(USE_WEBGL)
ComputeShader::ComputeShader()
{
    Log::debug("[Shader] Creating compute shader...");
    index = Renderer::create_shader(ShaderType::COMPUTE);
    Log::debug("[Shader] Created compute shader (ID: " + std::to_string(index) + ")");
}

ComputeShader ComputeShader::load_from_source(std::string const& source)
{
    ComputeShader comp_shader;
    comp_shader.load_source(source);
    return comp_shader;
}

ComputeShader ComputeShader::clone() const
{
    ComputeShader res;

    if (!path.empty()) {
        res.import(path);
    }
    else {
        res.load_source(Renderer::recover_shader_source(index));
    }

    return res;
}
#endif

}
