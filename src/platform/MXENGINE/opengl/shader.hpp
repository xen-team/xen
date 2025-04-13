#pragma once

#include "shader_base.hpp"

#if defined(XEN_DEBUG)
#include <array>
#include <vector>
#endif

#include <string>
#include <filesystem>

namespace fs = std::filesystem;

namespace xen {
struct PipelineStageInfo;

class Shader : public ShaderBase {
public:
    enum PipelineStage : uint8_t {
        VERTEX = 0, // should be equal to 0, next to 1, etc.,
        GEOMETRY,
        FRAGMENT,
        STAGE_COUNT // should be last
    };
#if defined(XEN_DEBUG)
    std::array<std::string, PipelineStage::STAGE_COUNT> debug_file_paths;
    std::vector<std::string> included_file_paths;
#endif

    static BindableId
    create_shader_program(std::vector<ShaderId>& shader_ids, std::vector<PipelineStageInfo> const& stage_infos);

    void load_debug_variables(std::vector<PipelineStageInfo> const& stage_infos);

public:
    void load(fs::path const& vertex_path, fs::path const& geometry_path, fs::path const& fragment_path);

    void load_from_string(
        std::string_view vertex_source, std::string_view geometry_source, std::string_view fragment_source
    );

    std::string_view const& get_debug_file_path(Shader::PipelineStage stage) const;
    std::vector<std::string> const& get_included_file_paths() const;
};
}