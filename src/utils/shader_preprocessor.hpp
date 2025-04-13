#include <filesystem>
#include <string>

#if defined(XEN_DEBUG)
#include <vector>
#endif

namespace xen {
class ShaderPreprocessor {
public:
private:
    std::string source;

#if defined(XEN_DEBUG)
    std::vector<std::string> include_file_paths;
    bool are_included_filepaths_loaded = false;
#endif
public:
    constexpr ShaderPreprocessor(std::string_view shader_source);

    constexpr ShaderPreprocessor& load_includes(std::filesystem::path const& lookup_path);

    constexpr ShaderPreprocessor& emit_prefix_line(std::string_view line);

    constexpr ShaderPreprocessor& emit_postfix_line(std::string_view line);

    [[nodiscard]] constexpr std::vector<std::string> get_include_files() const;

    [[nodiscard]] constexpr std::string_view get_result() { return source; }
};
}