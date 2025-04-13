#pragma once

// #include "Utilities/Math/Math.h"
// #include "Core/Macro/Macro.h"
// #include "Utilities/STL/MxString.h"
// #include "Utilities/STL/MxVector.h"
// #include "Utilities/STL/MxHashMap.h"

#include "math/vector2.hpp"
#include "math/vector3.hpp"
#include "math/vector4.hpp"
#include "math/matrix2.hpp"
#include "math/matrix3.hpp"
#include "math/matrix4.hpp"

#include <unordered_map>
#include <filesystem>

namespace xen {

class ShaderBase {
public:
    using UniformId = int;
    using ShaderId = uint16_t;
    using BindableId = uint16_t;
    using ShaderEnum = int;

    class UniformCache {
        std::unordered_map<std::string, UniformId> cache;
        BindableId shader_id;

    public:
        constexpr static UniformId invalid_location = -1;

        UniformCache(BindableId shader_id);

        UniformId get_uniform_location(std::string_view name);
        UniformId get_uniform_location_silent(std::string_view unifornamem_name);
    };

private:
    static BindableId attached_shader;

    BindableId id = 0;
    mutable UniformCache uniform_cache;

public:
    static std::string get_shader_version();

    ShaderBase();
    ~ShaderBase();
    ShaderBase(ShaderBase const&) = delete;
    ShaderBase(ShaderBase&& shader) noexcept;
    ShaderBase& operator=(ShaderBase const&) = delete;
    ShaderBase& operator=(ShaderBase&& shader) noexcept;

    void bind() const;

    void unbind() const;

    void invalidate_uniform_cache();

    void ignore_non_exist_uniform(std::string_view name) const;

    [[nodiscard]] BindableId get_native_handle() const { return id; }

    UniformId get_uniform_location(std::string_view name) const;

    void set_uniform(std::string_view name, float f) const;
    void set_uniform(std::string_view name, int i) const;
    void set_uniform(std::string_view name, bool b) const;
    void set_uniform(std::string_view name, Vector2f const& v) const;
    void set_uniform(std::string_view name, Vector3f const& v) const;
    void set_uniform(std::string_view name, Vector4f const& v) const;
    void set_uniform(std::string_view name, Vector2i const& v) const;
    void set_uniform(std::string_view name, Vector3i const& v) const;
    void set_uniform(std::string_view name, Vector4i const& v) const;
    void set_uniform(std::string_view name, Matrix2 const& m) const;
    void set_uniform(std::string_view name, Matrix3 const& m) const;
    void set_uniform(std::string_view name, Matrix4 const& m) const;

private:
    void free();

    [[nodiscard]] std::optional<UniformId> try_get_uniform_location(std::string_view name) const;

protected:
    static BindableId create_program(std::vector<ShaderId> const& ids);

    static ShaderId create_shader(ShaderEnum type, std::string_view source_code, std::filesystem::path const& filepath);

    static std::vector<std::string>
    get_shader_include_files(std::string_view source_code, std::filesystem::path const& filepath);

    static void delete_shader(ShaderId id);

    void set_native_handle(BindableId id);
};
}