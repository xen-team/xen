#pragma once

#include <render/shader.hpp>

namespace xen {
class ShaderLoader {
private:
    static std::string filepath;
    static std::unordered_map<std::size_t, std::shared_ptr<Shader>> shader_cache;
    static std::hash<std::string> hasher;

public:
    static std::shared_ptr<Shader> load_shader(std::string_view path);
    static void set_shader_filepath(std::string_view path) { filepath = path; }
};
}