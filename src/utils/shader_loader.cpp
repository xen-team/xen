#include "shader_loader.hpp"
#include <render/shader.hpp>

namespace xen {
// Static declarations
std::string ShaderLoader::filepath;
std::unordered_map<std::size_t, std::shared_ptr<Shader>> ShaderLoader::shader_cache;
std::hash<std::string> ShaderLoader::hasher;

std::shared_ptr<Shader> ShaderLoader::load_shader(std::string_view path)
{
    std::string shader_path = filepath + path.data();
    std::size_t const hash = hasher(shader_path);

    auto find_iter = shader_cache.find(hash);
    if (find_iter != shader_cache.end()) {
        return find_iter->second;
    }

    auto this_shader = shader_cache.emplace_hint(find_iter, hash, std::make_shared<Shader>(std::move(shader_path)));
    return this_shader->second;
}
}