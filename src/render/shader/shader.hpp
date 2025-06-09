#pragma once

#include <data/owner_value.hpp>
#include <utils/filepath.hpp>

namespace xen {
class Shader {
public:
    Shader(Shader const&) = delete;
    Shader(Shader&&) noexcept = default;

    Shader& operator=(Shader const&) = delete;
    Shader& operator=(Shader&&) noexcept = default;

    virtual ~Shader() { destroy(); }

    [[nodiscard]] uint32_t get_index() const { return index; }

    [[nodiscard]] FilePath const& get_path() const { return path; }

    [[nodiscard]] bool is_valid() const { return index.is_valid(); }

    [[nodiscard]] bool is_compiled() const;

    void import(FilePath filepath);

    /// Reloads the shader file. The shader must have been previously imported from a file for this function to load
    /// anything.
    /// \see import()
    void load() const;

    void compile() const;

    void destroy();

protected:
    Shader() = default;

    void load_source(std::string const& source) const;

protected:
    OwnerValue<uint32_t> index{};
    FilePath path{};
};

class VertexShader final : public Shader {
public:
    VertexShader();
    explicit VertexShader(FilePath filepath) : VertexShader() { import(std::move(filepath)); }

    static VertexShader load_from_source(std::string const& source);
    static VertexShader load_from_source(std::string_view source) { return load_from_source(std::string(source)); }
    static VertexShader load_from_source(char const* source) { return load_from_source(std::string(source)); }

    VertexShader clone() const;
};

#if !defined(USE_OPENGL_ES)
class TessellationControlShader final : public Shader {
public:
    TessellationControlShader();
    explicit TessellationControlShader(FilePath filepath) : TessellationControlShader() { import(std::move(filepath)); }

    static TessellationControlShader load_from_source(std::string const& source);
    static TessellationControlShader load_from_source(std::string_view source)
    {
        return load_from_source(std::string(source));
    }
    static TessellationControlShader load_from_source(char const* source)
    {
        return load_from_source(std::string(source));
    }

    TessellationControlShader clone() const;
};

class TessellationEvaluationShader final : public Shader {
public:
    TessellationEvaluationShader();
    explicit TessellationEvaluationShader(FilePath filepath) : TessellationEvaluationShader()
    {
        import(std::move(filepath));
    }

    static TessellationEvaluationShader load_from_source(std::string const& source);
    static TessellationEvaluationShader load_from_source(std::string_view source)
    {
        return load_from_source(std::string(source));
    }
    static TessellationEvaluationShader load_from_source(char const* source)
    {
        return load_from_source(std::string(source));
    }

    TessellationEvaluationShader clone() const;
};

class GeometryShader final : public Shader {
public:
    GeometryShader();
    explicit GeometryShader(FilePath filepath) : GeometryShader() { import(std::move(filepath)); }

    static GeometryShader load_from_source(std::string const& source);
    static GeometryShader load_from_source(std::string_view source) { return load_from_source(std::string(source)); }
    static GeometryShader load_from_source(char const* source) { return load_from_source(std::string(source)); }

    GeometryShader clone() const;
};
#endif

class FragmentShader final : public Shader {
public:
    FragmentShader();
    explicit FragmentShader(FilePath filepath) : FragmentShader() { import(std::move(filepath)); }

    static FragmentShader load_from_source(std::string const& source);
    static FragmentShader load_from_source(std::string_view source) { return load_from_source(std::string(source)); }
    static FragmentShader load_from_source(char const* source) { return load_from_source(std::string(source)); }

    FragmentShader clone() const;
};

#if !defined(USE_WEBGL)
class ComputeShader final : public Shader {
public:
    ComputeShader();
    explicit ComputeShader(FilePath filepath) : ComputeShader() { import(std::move(filepath)); }

    static ComputeShader load_from_source(std::string const& source);
    static ComputeShader load_from_source(std::string_view source) { return load_from_source(std::string(source)); }
    static ComputeShader load_from_source(char const* source) { return load_from_source(std::string(source)); }

    ComputeShader clone() const;
};
#endif
}