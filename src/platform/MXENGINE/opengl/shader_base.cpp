#include "shader_base.hpp"
#include "gl_utils.hpp"

#include "utils/shader_preprocessor.hpp"
#include "debug/log.hpp"

namespace xen {
ShaderBase::BindableId ShaderBase::attached_shader = 0;

ShaderBase::UniformCache::UniformCache(BindableId shader_id) : shader_id(shader_id) {}

ShaderBase::UniformId ShaderBase::UniformCache::get_uniform_location(std::string_view name)
{
    if (auto it = cache.find(std::string(name)); it != cache.end()) {
        return it->second;
    }

    GL_CALL(ShaderBase::UniformId location = glGetUniformLocation(shader_id, name.data()));

    if (location == UniformCache::invalid_location) {
        Log::warning(
            "OpenGL::Shader"
            "uniform was not found: ",
            name
        );
    }

    cache[std::string(name)] = location;
    return location;
}

ShaderBase::UniformId ShaderBase::UniformCache::get_uniform_location_silent(std::string_view name)
{
    if (auto it = cache.find(std::string(name)); it != cache.end()) {
        return it->second;
    }

    GL_CALL(ShaderBase::UniformId location = glGetUniformLocation(shader_id, name.data()));
    cache[std::string(name)] = location;
    return location;
}

std::string ShaderBase::get_shader_version()
{
    return "pivoorg";
    // return "#version " +
    //    ToMxString(GlobalConfig::GetGraphicAPIMajorVersion() * 100 + GlobalConfig::GetGraphicAPIMinorVersion() * 10);
}

void ShaderBase::free()
{
    if (id != 0) {
        Log::warning("OpenGL::Shader", " freed shader program with id: ", id);
        GL_CALL(glDeleteProgram(id));
    }
    id = 0;
}

// returns true if any errors occured
bool print_errors_to_log_shader(ShaderBase::BindableId shader_id, GLenum stage)
{
    GLint result;
    GL_CALL(glGetShaderiv(shader_id, stage, &result));
    if (result == GL_FALSE) {
        GLint length;
        GL_CALL(glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &length));
        std::string msg;
        msg.resize(length);
        GL_CALL(glGetShaderInfoLog(shader_id, length, &length, &msg[0]));
        if (!msg.empty()) {
            msg.pop_back(); // extra \n character
            Log::error("OpenGL::ErrorHandler ", msg);
        }
        return true;
    }
    return false;
}

// returns true if any errors occured
bool print_erros_to_log_program(ShaderBase::BindableId program_id, GLenum stage)
{
    GLint result;
    GL_CALL(glGetProgramiv(program_id, stage, &result));
    if (result == GL_FALSE) {
        GLint length;
        GL_CALL(glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &length));
        std::string msg;
        msg.resize(length);
        GL_CALL(glGetProgramInfoLog(program_id, length, &length, &msg[0]));
        if (!msg.empty()) {
            msg.pop_back(); // extra \n character
            Log::error("OpenGL::ErrorHandler ", msg);
        }
        return true;
    }
    return false;
}

ShaderBase::BindableId ShaderBase::create_program(std::vector<ShaderId> const& ids)
{
    GL_CALL(BindableId program = glCreateProgram());

    for (ShaderId const id : ids) {
        GL_CALL(glAttachShader(program, id));
    }

    GL_CALL(glLinkProgram(program));
    if (print_erros_to_log_program(program, GL_LINK_STATUS)) {
        Log::warning("OpenGL::Shader", " failed to link shader program with id: ", program);
    }

    GL_CALL(glValidateProgram(program));
    if (print_erros_to_log_program(program, GL_VALIDATE_STATUS)) {
        Log::warning("OpenGL::Shader", " failed to validate shader program with id: ", program);
    }

    for (ShaderId const id : ids) {
        GL_CALL(glDetachShader(program, id));
    }

    return program;
}

std::vector<std::string>
ShaderBase::get_shader_include_files(std::string_view source_code, std::filesystem::path const& path)
{
    ShaderPreprocessor preprocessor(source_code);
    return preprocessor.load_includes(path.parent_path()).get_include_files();
}

ShaderBase::ShaderId
ShaderBase::create_shader(ShaderEnum type, std::string_view source_code, std::filesystem::path const& path)
{
    GL_CALL(ShaderId shaderId = glCreateShader(static_cast<GLenum>(type)));

    ShaderPreprocessor preprocessor(source_code);

    auto const modified_source_code =
        preprocessor.load_includes(path.parent_path()).emit_prefix_line(ShaderBase::get_shader_version()).get_result();

    auto sourceptr = modified_source_code.data();
    GL_CALL(glShaderSource(shaderId, 1, &sourceptr, nullptr));
    GL_CALL(glCompileShader(shaderId));
    if (print_errors_to_log_shader(shaderId, GL_COMPILE_STATUS)) {
        Log::warning("OpenGL::Shader", " failed to compile shader stage: ", path);
    }

    return shaderId;
}

void ShaderBase::delete_shader(ShaderId id)
{
    GL_CALL(glDeleteShader(id));
}

void ShaderBase::set_native_handle(BindableId id)
{
    free();
    this->id = id;
    uniform_cache = UniformCache{this->id};
}

void ShaderBase::bind() const
{
    GL_CALL(glUseProgram(id));
    ShaderBase::attached_shader = id;
}

void ShaderBase::unbind() const
{
    GL_CALL(glUseProgram(0));
    ShaderBase::attached_shader = 0;
}

ShaderBase::ShaderBase() : id(0), uniform_cache(id) {}

ShaderBase::~ShaderBase()
{
    free();
}

ShaderBase::ShaderBase(ShaderBase&& other) noexcept : id(other.id), uniform_cache(std::move(other.uniform_cache))
{
    other.id = 0;
}

ShaderBase& ShaderBase::operator=(ShaderBase&& other) noexcept
{
    free();

    id = other.id;
    uniform_cache = std::move(other.uniform_cache);

    return *this;
}

std::optional<ShaderBase::UniformId> ShaderBase::try_get_uniform_location(std::string_view name) const
{
    assert(id == ShaderBase::attached_shader);

    auto const location = get_uniform_location(name);
    if (location == ShaderBase::UniformCache::invalid_location) {
        return std::nullopt;
    }

    return location;
}

void ShaderBase::set_uniform(std::string_view name, float f) const
{
    auto location = try_get_uniform_location(name);

    if (!location.has_value()) {
        return;
    }

    GL_CALL(glUniform1f(*location, f));
}
void ShaderBase::set_uniform(std::string_view name, int i) const
{
    auto location = try_get_uniform_location(name);

    if (!location.has_value()) {
        return;
    }

    GL_CALL(glUniform1f(*location, i));
}
void ShaderBase::set_uniform(std::string_view name, bool b) const
{
    auto location = try_get_uniform_location(name);

    if (!location.has_value()) {
        return;
    }

    GL_CALL(glUniform1i(*location, static_cast<int>(b)));
}
void ShaderBase::set_uniform(std::string_view name, Vector2f const& v) const
{
    auto location = try_get_uniform_location(name);

    if (!location.has_value()) {
        return;
    }

    GL_CALL(glUniform2f(*location, v[0], v[1]));
}
void ShaderBase::set_uniform(std::string_view name, Vector3f const& v) const
{
    auto location = try_get_uniform_location(name);

    if (!location.has_value()) {
        return;
    }

    GL_CALL(glUniform3f(*location, v[0], v[1], v[2]));
}
void ShaderBase::set_uniform(std::string_view name, Vector4f const& v) const
{
    auto location = try_get_uniform_location(name);

    if (!location.has_value()) {
        return;
    }

    GL_CALL(glUniform4f(*location, v[0], v[1], v[2], v[3]));
}
void ShaderBase::set_uniform(std::string_view name, Vector2i const& v) const
{
    auto location = try_get_uniform_location(name);

    if (!location.has_value()) {
        return;
    }

    GL_CALL(glUniform2i(*location, v[0], v[1]));
}
void ShaderBase::set_uniform(std::string_view name, Vector3i const& v) const
{
    auto location = try_get_uniform_location(name);

    if (!location.has_value()) {
        return;
    }

    GL_CALL(glUniform3i(*location, v[0], v[1], v[2]));
}
void ShaderBase::set_uniform(std::string_view name, Vector4i const& v) const
{
    auto location = try_get_uniform_location(name);

    if (!location.has_value()) {
        return;
    }

    GL_CALL(glUniform4i(*location, v[0], v[1], v[2], v[3]));
}
void ShaderBase::set_uniform(std::string_view name, Matrix2 const& m) const
{
    auto location = try_get_uniform_location(name);

    if (!location.has_value()) {
        return;
    }

    GL_CALL(glUniformMatrix2fv(*location, 1, false, &m[0][0]));
}
void ShaderBase::set_uniform(std::string_view name, Matrix3 const& m) const
{
    auto location = try_get_uniform_location(name);

    if (!location.has_value()) {
        return;
    }

    GL_CALL(glUniformMatrix3fv(*location, 1, false, &m[0][0]));
}
void ShaderBase::set_uniform(std::string_view name, Matrix4 const& m) const
{
    auto location = try_get_uniform_location(name);

    if (!location.has_value()) {
        return;
    }

    GL_CALL(glUniformMatrix4fv(*location, 1, false, &m[0][0]));
}

void ShaderBase::ignore_non_exist_uniform(std::string_view name) const
{
    get_uniform_location(name);
}

ShaderBase::UniformId ShaderBase::get_uniform_location(std::string_view name) const
{
    return get_uniform_location(name);
}

void ShaderBase::invalidate_uniform_cache()
{
    uniform_cache = UniformCache{id};
}
}