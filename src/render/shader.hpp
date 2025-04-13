#pragma once

namespace xen {
class Shader {
    friend class ShaderLoader;

private:
    using ShaderId = uint16_t;
    using UniformId = int32_t;

    class UniformCache {
        std::unordered_map<std::string, UniformId> cache;
        ShaderId shader_id;

    public:
        constexpr static UniformId invalid_location = -1;

        UniformCache(ShaderId shader_id) : shader_id{shader_id} {};

        UniformId get_uniform_location(std::string_view name);
    };

    ShaderId id = 0;
    UniformCache cache = id;

    // Filepath maybe overhead for shader class because of non-usability
    std::string filepath;

public:
    Shader(std::string_view path);

public:
    ~Shader();

    void enable() const;
    void disable() const;

    void set_uniform(std::string_view name, float value);
    void set_uniform(std::string_view name, int value);
    void set_uniform(std::string_view name, bool value);
    void set_uniform(std::string_view name, Vector2f const& vector);
    void set_uniform(std::string_view name, Vector2i const& vector);
    void set_uniform(std::string_view name, Vector3f const& vector);
    void set_uniform(std::string_view name, Vector3i const& vector);
    void set_uniform(std::string_view name, Vector4f const& vector);
    void set_uniform(std::string_view name, Vector4i const& vector);
    void set_uniform(std::string_view name, Matrix2 const& matrix);
    void set_uniform(std::string_view name, Matrix3 const& matrix);
    void set_uniform(std::string_view name, Matrix4 const& matrix);

    void set_uniform_array(std::string_view name, std::span<float const> value);
    void set_uniform_array(std::string_view name, std::span<int const> value);
    void set_uniform_array(std::string_view name, std::span<bool const> value);
    void set_uniform_array(std::string_view name, std::span<Vector2f const> value);
    void set_uniform_array(std::string_view name, std::span<Vector2i const> value);
    void set_uniform_array(std::string_view name, std::span<Vector3f const> value);
    void set_uniform_array(std::string_view name, std::span<Vector3i const> value);
    void set_uniform_array(std::string_view name, std::span<Vector4f const> value);
    void set_uniform_array(std::string_view name, std::span<Vector4i const> value);
    void set_uniform_array(std::string_view name, std::span<Matrix2 const> value);
    void set_uniform_array(std::string_view name, std::span<Matrix3 const> value);
    void set_uniform_array(std::string_view name, std::span<Matrix4 const> value);

    [[nodiscard]] ShaderId get_shader_id() const { return id; }

private:
    std::optional<UniformId> try_get_uniform_location(std::string_view name);

    constexpr static GLenum shader_type_from_string(std::string_view type)
    {
        if (type == "vertex") {
            return GL_VERTEX_SHADER;
        }
        if (type == "fragment") {
            return GL_FRAGMENT_SHADER;
        }
        if (type == "geometry") {
            return GL_GEOMETRY_SHADER;
        }
        if (type == "hull") {
            return GL_TESS_CONTROL_SHADER;
        }
        if (type == "domain") {
            return GL_TESS_EVALUATION_SHADER;
        }
        if (type == "compute") {
            return GL_COMPUTE_SHADER;
        }

        Log::rt_assert(
            true,
            "Render::Shader "
            "shader_type_from_string acounted an error, no type named ",
            type
        );

        return 0;
    }

    std::unordered_map<GLenum, std::string> pre_process_shader_binary(std::string_view source);
    void compile(std::unordered_map<GLenum, std::string> const& shader_sources);
};
}