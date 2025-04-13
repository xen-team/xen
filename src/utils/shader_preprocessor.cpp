#include "shader_preprocessor.hpp"

#include "debug/log.hpp"

#include <regex>
#include <fstream>

using namespace std::string_literals;

namespace xen {
constexpr ShaderPreprocessor::ShaderPreprocessor(std::string_view shader_source) : source(shader_source) {}

static constexpr auto find_all_include_paths(std::string_view source)
{
    std::vector<std::pair<std::string, std::string>> includes;

    std::regex r(R"(#include\s+\"(.+)\")");
    std::regex_iterator path_it(source.begin(), source.end(), r);
    std::regex_iterator<char const*> path_end;

    for (; path_it != path_end; path_it++) {
        std::string include_command(
            source.begin() + path_it->position(0), source.begin() + path_it->position(0) + path_it->length(0)
        );
        std::string filepath(
            source.begin() + path_it->position(1), source.begin() + path_it->position(1) + path_it->length(1)
        );
        includes.emplace_back(std::move(filepath), std::move(include_command));
    }

    return includes;
}

constexpr ShaderPreprocessor& ShaderPreprocessor::load_includes(std::filesystem::path const& lookup_path)
{
#if defined(MXENGINE_DEBUG)
    this->are_included_filepaths_loaded = true;
#endif

    auto paths = find_all_include_paths(source);
    for (auto const& [path, include] : paths) {
        auto filepath = lookup_path / path;
        if (!std::filesystem::exists(filepath)) {
            Log::error("ShaderPreprocessor::LoadIncludes", " included file was not found: ", path);
            return *this;
        }
#if defined(MXENGINE_DEBUG)
        include_file_paths.push_back(path);
#endif

        std::regex reg(include);

        std::string context;
        std::ifstream f(filepath);
        f >> context;

        source = std::regex_replace(source, reg, context);
    }
    // maybe there are new includes in included files
    if (!paths.empty()) {
        load_includes(lookup_path);
    }

    return *this;
}

constexpr ShaderPreprocessor& ShaderPreprocessor::emit_prefix_line(std::string_view line)
{
    source.insert(0, "\n");
    source.insert(0, line);
    return *this;
}

constexpr ShaderPreprocessor& ShaderPreprocessor::emit_postfix_line(std::string_view line)
{
    source += '\n';
    source += line;
    return *this;
}

constexpr std::vector<std::string> ShaderPreprocessor::get_include_files() const
{
#if defined(XEN_DEBUG)
    if (!this->are_included_filepaths_loaded) {
        Log::warning("Xen::ShaderPreprocessor", " included filepaths are not loaded as LoadIncludes() was not called");
    }
    return this->include_file_paths;
#else
    Log::warning("Xen::ShaderPreprocessor", " included filepaths are not saved in non-debug build");
    return {};
#endif
}
}