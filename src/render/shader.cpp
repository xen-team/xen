#include "shader.hpp"

#include <fstream>

namespace xen {
Shader::UniformId Shader::UniformCache::get_uniform_location(std::string_view name)
{
    std::string const name_str = std::string(name);

    if (auto it = cache.find(name_str); it != cache.end()) {
        return it->second;
    }

    Shader::UniformId location = glGetUniformLocation(shader_id, name.data());

    if (location == UniformCache::invalid_location) {
        Log::warning(
            "OpenGL::Shader "
            "uniform was not found: ",
            name
        );
    }

    cache[name_str] = location;
    return location;
}

std::optional<Shader::UniformId> Shader::try_get_uniform_location(std::string_view name)
{
    auto const location = cache.get_uniform_location(name);
    if (location == Shader::UniformCache::invalid_location) {
        return std::nullopt;
    }

    return location;
}

Shader::Shader(std::string_view path) : filepath(path)
{
    std::ifstream f(path.data(), std::ios::binary);

    std::string shader_binary;
    f >> shader_binary;
    auto shader_sources = pre_process_shader_binary(shader_binary);

    compile(shader_sources);
}

Shader::~Shader()
{
    GL_CALL(glDeleteProgram(id));
}

void Shader::enable() const
{
    GL_CALL(glUseProgram(id));
}

void Shader::disable() const
{
    GL_CALL(glUseProgram(0));
}

void Shader::set_uniform(std::string_view name, float value)
{
    auto const location = try_get_uniform_location(name);

    if (!location.has_value()) {
        return;
    }

    GL_CALL(glUniform1f(*location, value));
}

void Shader::set_uniform(std::string_view name, int value)
{
    auto const location = try_get_uniform_location(name);

    if (!location.has_value()) {
        return;
    }

    GL_CALL(glUniform1i(*location, value));
}

void Shader::set_uniform(std::string_view name, bool value)
{
    auto const location = try_get_uniform_location(name);

    if (!location.has_value()) {
        return;
    }

    GL_CALL(glUniform1i(*location, static_cast<int>(value)));
}

void Shader::set_uniform(std::string_view name, Vector2f const& vector)
{
    auto const location = try_get_uniform_location(name);

    if (!location.has_value()) {
        return;
    }

    GL_CALL(glUniform2f(*location, vector.x, vector.y));
}

void Shader::set_uniform(std::string_view name, Vector2i const& vector)
{
    auto const location = try_get_uniform_location(name);

    if (!location.has_value()) {
        return;
    }

    GL_CALL(glUniform2i(*location, vector.x, vector.y));
}

void Shader::set_uniform(std::string_view name, Vector3f const& vector)
{
    auto const location = try_get_uniform_location(name);

    if (!location.has_value()) {
        return;
    }

    GL_CALL(glUniform3f(*location, vector.x, vector.y, vector.z));
}

void Shader::set_uniform(std::string_view name, Vector3i const& vector)
{
    auto const location = try_get_uniform_location(name);

    if (!location.has_value()) {
        return;
    }

    GL_CALL(glUniform3i(*location, vector.x, vector.y, vector.z));
}

void Shader::set_uniform(std::string_view name, Vector4f const& vector)
{
    auto const location = try_get_uniform_location(name);

    if (!location.has_value()) {
        return;
    }

    GL_CALL(glUniform4f(*location, vector.x, vector.y, vector.z, vector.w));
}

void Shader::set_uniform(std::string_view name, Vector4i const& vector)
{
    auto const location = try_get_uniform_location(name);

    if (!location.has_value()) {
        return;
    }

    GL_CALL(glUniform4i(*location, vector.x, vector.y, vector.z, vector.w));
}

void Shader::set_uniform(std::string_view name, Matrix2 const& matrix)
{
    auto const location = try_get_uniform_location(name);

    if (!location.has_value()) {
        return;
    }

    GL_CALL(glUniformMatrix3fv(*location, 1, GL_FALSE, &matrix[0][0]));
}

void Shader::set_uniform(std::string_view name, Matrix3 const& matrix)
{
    auto const location = try_get_uniform_location(name);

    if (!location.has_value()) {
        return;
    }

    GL_CALL(glUniformMatrix3fv(*location, 1, GL_FALSE, &matrix[0][0]));
}

void Shader::set_uniform(std::string_view name, Matrix4 const& matrix)
{
    auto const location = try_get_uniform_location(name);

    if (!location.has_value()) {
        return;
    }

    GL_CALL(glUniformMatrix4fv(*location, 1, GL_FALSE, &matrix[0][0]));
}

void Shader::set_uniform_array(std::string_view name, std::span<float const> value)
{
    auto const location = try_get_uniform_location(name);

    if (!location.has_value()) {
        return;
    }

    GL_CALL(glUniform1fv(*location, value.size(), value.data()));
}
void Shader::set_uniform_array(std::string_view name, std::span<int const> value)
{
    auto const location = try_get_uniform_location(name);

    if (!location.has_value()) {
        return;
    }

    GL_CALL(glUniform1iv(*location, value.size(), value.data()));
}
void Shader::set_uniform_array(std::string_view name, std::span<bool const> value)
{
    auto const location = try_get_uniform_location(name);

    if (!location.has_value()) {
        return;
    }

    // Potentially unsafe, do some tests
    GL_CALL(glUniform1iv(*location, value.size(), reinterpret_cast<int const*>(value.data())));
}
void Shader::set_uniform_array(std::string_view name, std::span<Vector2f const> value)
{
    auto const location = try_get_uniform_location(name);

    if (!location.has_value()) {
        return;
    }

    GL_CALL(glUniform2fv(*location, value.size(), &value.data()->x));
}
void Shader::set_uniform_array(std::string_view name, std::span<Vector2i const> value)
{
    auto const location = try_get_uniform_location(name);

    if (!location.has_value()) {
        return;
    }

    GL_CALL(glUniform2iv(*location, value.size(), &value.data()->x));
}
void Shader::set_uniform_array(std::string_view name, std::span<Vector3f const> value)
{
    auto const location = try_get_uniform_location(name);

    if (!location.has_value()) {
        return;
    }

    GL_CALL(glUniform3fv(*location, value.size(), &value.data()->x));
}
void Shader::set_uniform_array(std::string_view name, std::span<Vector3i const> value)
{
    auto const location = try_get_uniform_location(name);

    if (!location.has_value()) {
        return;
    }

    GL_CALL(glUniform3iv(*location, value.size(), &value.data()->x));
}
void Shader::set_uniform_array(std::string_view name, std::span<Vector4f const> value)
{
    auto const location = try_get_uniform_location(name);

    if (!location.has_value()) {
        return;
    }

    GL_CALL(glUniform4fv(*location, value.size(), &value.data()->x));
}
void Shader::set_uniform_array(std::string_view name, std::span<Vector4i const> value)
{
    auto const location = try_get_uniform_location(name);

    if (!location.has_value()) {
        return;
    }

    GL_CALL(glUniform4iv(*location, value.size(), &value.data()->x));
}
void Shader::set_uniform_array(std::string_view name, std::span<Matrix2 const> value)
{
    auto const location = try_get_uniform_location(name);

    if (!location.has_value()) {
        return;
    }

    GL_CALL(glUniform1fv(*location, value.size(), &value[0][0][0]));
}
void Shader::set_uniform_array(std::string_view name, std::span<Matrix3 const> value)
{
    auto const location = try_get_uniform_location(name);

    if (!location.has_value()) {
        return;
    }

    GL_CALL(glUniformMatrix3fv(*location, value.size(), GL_FALSE, &value[0][0][0]));
}
void Shader::set_uniform_array(std::string_view name, std::span<Matrix4 const> value)
{
    auto const location = try_get_uniform_location(name);

    if (!location.has_value()) {
        return;
    }

    GL_CALL(glUniform1fv(*location, value.size(), &value[0][0][0]));
}

std::unordered_map<GLenum, std::string> Shader::pre_process_shader_binary(std::string_view source)
{
    std::unordered_map<GLenum, std::string> shader_sources;

    std::string_view shader_type_token = "#shader_type";
    size_t const shader_type_token_length = shader_type_token.length();
    size_t pos = 0;

    while ((pos = source.find(shader_type_token, pos)) != std::string::npos) {
        size_t const eol = source.find_first_of("\r\n", pos);
        if (eol == std::string::npos) {
            Log::error(
                "Render::Shader "
                "accounter an error while pre_processing shader: ",
                source
            );

            break;
        }

        size_t const begin = pos + shader_type_token_length + 1;
        std::string_view shader_type = source.substr(begin, eol - begin);

        GLenum shader_enum = shader_type_from_string(shader_type);
        if (shader_enum == 0) {
            break;
        }

        size_t const next_line_pos = source.find_first_not_of("\r\n", eol);
        if (next_line_pos == std::string::npos) {
            break;
        }

        size_t const next_shader_pos = source.find(shader_type_token, next_line_pos);
        shader_sources[shader_enum] = source.substr(next_line_pos, next_shader_pos - next_line_pos);

        pos = next_shader_pos;
    }
    return shader_sources;
}

void Shader::compile(std::unordered_map<GLenum, std::string> const& shader_sources)
{
    id = glCreateProgram();

    // Attach different components of the shader (vertex, fragment, geometry, hull, domain, or compute)
    for (auto const& item : shader_sources) {
        GLenum const type = item.first;
        std::string const& source = item.second;

        GLuint const shader = glCreateShader(type);
        GLchar const* shader_source = source.c_str();
        glShaderSource(shader, 1, &shader_source, nullptr);
        glCompileShader(shader);

        // Check to see if compiling was successful
        GLint was_compiled;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &was_compiled);

        if (was_compiled == GL_FALSE || source.empty()) {
            int length;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);

            if (length > 0) {
                std::vector<char> error(length);
                glGetShaderInfoLog(shader, length, &length, error.data());

                std::string const error_str(error.begin(), error.end());

                Log::error("Shader Compile Error: ", filepath, " - ", error_str);
            }
            else {
                Log::error("Shader Compile Error: ", filepath, " - Unknown Error");
            }
            glDeleteShader(shader);
            break;
        }

        glAttachShader(id, shader);
        glDeleteShader(shader);
    }

    // Validate shader
    glLinkProgram(id);
    glValidateProgram(id);
}
}