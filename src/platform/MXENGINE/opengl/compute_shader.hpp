#include "shader_base.hpp"

namespace xen {
class ComputeShader : public ShaderBase {
    // #if defined(XEN_DEBUG)
    //     std::string debug_file_path;
    //     std::vector<std::string> included_file_paths;
    // #endif

    static BindableId create_shader_program(std::string_view source, std::string_view path);

    // void load_debug_variables(std::string_view source, std::string_view path);

public:
    void load(std::string_view path);

    void load_from_string(std::string_view source);

    // std::string GetDebugFilePath() const;
    // std::vector<std::string> const& GetIncludedFilePaths() const;
};
}