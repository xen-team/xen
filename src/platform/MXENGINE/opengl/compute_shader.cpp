#include "compute_shader.hpp"
#include "gl_utils.hpp"
#include "debug/log.hpp"

namespace xen {
// template <>
// void ComputeShader::LoadDebugVariables<FilePath>(MxString const& source, FilePath const& path)
// {
// #if defined(MXENGINE_DEBUG)
//     this->debugFilePath = ToMxString(FileManager::GetProximatePath(path, FileManager::GetWorkingDirectory()));
//     std::replace(this->debugFilePath.begin(), this->debugFilePath.end(), '\\', '/');
//     this->includedFilePaths = ShaderBase::GetShaderIncludeFiles(source, path);
// #endif
// }

ComputeShader::BindableId ComputeShader::create_shader_program(std::string_view source, std::string_view path)
{
    Log::debug("OpenGL::Shader", " compiling compute shader");

    ShaderId const shader_id = ShaderBase::create_shader(GL_COMPUTE_SHADER, source, path);

    BindableId program = ShaderBase::create_program({shader_id});

    Log::debug("OpenGL::Shader", " created shader program with id: ", program);

    ShaderBase::delete_shader(shader_id);

    return program;
}

void ComputeShader::load_from_string(std::string_view source)
{
    BindableId program = ComputeShader::create_shader_program(source, "_compute.glsl");
    // this->LoadDebugVariables(source, "_compute.glsl");
    set_native_handle(program);
}

// MxString const& ComputeShader::GetDebugFilePath() const
// {
// #if defined(MXENGINE_DEBUG)
//     return this->debugFilePath;
// #else
//     const static MxString EmptyFilePath;
//     return EmptyFilePath;
// #endif
// }

// MxVector<MxString> const& ComputeShader::GetIncludedFilePaths() const
// {
// #if defined(MXENGINE_DEBUG)
//     return this->includedFilePaths;
// #else
//     const static MxVector<MxString> EmptyVector;
//     return EmptyVector;
// #endif
// }
}