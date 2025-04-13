#include "shader.hpp"
#include "gl_utils.hpp"

#include "debug/log.hpp"
#include "utils/enumerate.hpp"
#include <algorithm>

namespace xen {
constexpr std::array pipeline_stage_to_native = {GL_VERTEX_SHADER, GL_GEOMETRY_SHADER, GL_FRAGMENT_SHADER};
constexpr std::array pipeline_stage_to_string = {"vertex", "geometry", "fragment"};

struct PipelineStageInfo {
    Shader::PipelineStage stage;
    std::string source;
    fs::path path;
};

Shader::BindableId
Shader::create_shader_program(std::vector<ShaderId>& shader_ids, std::vector<PipelineStageInfo> const& stage_infos)
{
    shader_ids.resize(stage_infos.size());
    for (auto const& [i, stage_info] : enumerate(stage_infos)) {
        Log::debug("OpenGL::Shader", " compiling ", pipeline_stage_to_string[i], " shader");
        shader_ids[i] = ShaderBase::create_shader(pipeline_stage_to_native[i], stage_info.source, stage_info.path);
    }

    // link stages into one shader program
    BindableId program = ShaderBase::create_program(shader_ids);
    Log::debug("OpenGL::Shader", " created shader program with id: ", program);

    for (auto const& shader : shader_ids) {
        ShaderBase::delete_shader(shader);
    }

    return program;
}

void Shader::load_debug_variables(std::vector<PipelineStageInfo> const& stage_infos)
{
#if defined(XEN_DEBUG)
    for (auto const& stage_info : stage_infos) {
        debug_file_paths[static_cast<size_t>(stage_info.stage)] = fs::proximate(stage_info.path);

        std::ranges::replace(debug_file_paths[static_cast<size_t>(stage_info.stage)], '\\', '/');

        auto const filepaths = ShaderBase::get_shader_include_files(stage_info.source, stage_info.path);
        included_file_paths.insert(
            included_file_paths.end(), std::make_move_iterator(filepaths.begin()),
            std::make_move_iterator(filepaths.end())
        );
    }
#endif
}

void Shader::load(fs::path const& vertex_path, fs::path const& geometry_path, fs::path const& fragment_path)
{
    std::string vertex;
    std::string fragment;
    std::string geometry;
    std::ifstream vf(vertex_path);
    std::ifstream ff(fragment_path);
    std::ifstream gf(geometry_path);
    vf >> vertex;
    ff >> fragment;
    gf >> geometry;

    std::vector stage_infos = {
        PipelineStageInfo{.stage = VERTEX, .source = vertex, .path = vertex_path},
        PipelineStageInfo{.stage = GEOMETRY, .source = geometry, .path = geometry_path},
        PipelineStageInfo{.stage = FRAGMENT, .source = fragment, .path = fragment_path},
    };

    std::vector<ShaderId> ids;

    auto program = Shader::create_shader_program(ids, stage_infos);
    load_debug_variables(stage_infos);
    set_native_handle(program);
}

void Shader::load_from_string(
    std::string_view vertex_source, std::string_view geometry_source, std::string_view fragment_source
)
{
    std::vector stage_infos = {
        PipelineStageInfo{.stage = VERTEX, .source = std::string(vertex_source), .path = fs::path("_.vs")},
        PipelineStageInfo{.stage = GEOMETRY, .source = std::string(geometry_source), .path = fs::path("_.gs")},
        PipelineStageInfo{.stage = FRAGMENT, .source = std::string(fragment_source), .path = fs::path("_.fs")},
    };

    std::vector<ShaderId> ids;

    auto program = Shader::create_shader_program(ids, stage_infos);
    load_debug_variables(stage_infos);
    set_native_handle(program);
}

std::string_view const& Shader::get_debug_file_path(Shader::PipelineStage stage) const
{
#if defined(XEN_DEBUG)
    return debug_file_paths[static_cast<size_t>(stage)];
#else
    return "";
#endif
}

std::vector<std::string> const& Shader::get_included_file_paths() const
{
#if defined(XEN_DEBUG)
    return included_file_paths;
#else
    const static std::vector<std::string> EmptyVector;
    return EmptyVector;
#endif
}
}