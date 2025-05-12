#pragma once

#include <render/shader/shader_program.hpp>

namespace xen {
enum class MaterialType {
    COOK_TORRANCE,     ///< Cook-Torrance (PBR) material.
    BLINN_PHONG,       ///< Blinn-Phong material.
    SINGLE_TEXTURE_2D, ///< Single 2D texture material; only displays the given texture as-is.
    SINGLE_TEXTURE_3D  ///< Single 3D texture material; only displays the given texture as-is.
};

/// Predefined material attributes names.
namespace MaterialAttribute {
// Common attributes
static constexpr char const* BaseColor = "uniMaterial.baseColor"; ///< Base color factor.
static constexpr char const* Emissive = "uniMaterial.emissive";   ///< Emissive factor.

// PBR attributes
static constexpr char const* Metallic = "uniMaterial.metallicFactor";   ///< Metallic factor.
static constexpr char const* Roughness = "uniMaterial.roughnessFactor"; ///< Roughness factor.
static constexpr char const* Sheen = "uniMaterial.sheenFactors";        ///< Sheen color (RGB) & roughness (A) factors.

// Legacy attributes
static constexpr char const* Ambient = "uniMaterial.ambient";   ///< Ambient factor.
static constexpr char const* Specular = "uniMaterial.specular"; ///< Specular factor.
static constexpr char const* Opacity = "uniMaterial.opacity";   ///< Opacity factor.
}

/// Predefined material textures names.
namespace MaterialTexture {
// Common textures
static constexpr char const* BaseColor = "uniMaterial.baseColorMap"; ///< Base color (albedo or diffuse) map.
static constexpr char const* Emissive = "uniMaterial.emissiveMap";   ///< Emissive map.
static constexpr char const* Ambient =
    "uniMaterial.ambientMap"; ///< Ambient occlusion map (Cook-Torrance) or ambient map (legacy).

// PBR textures
static constexpr char const* Normal = "uniMaterial.normalMap";       ///< Normal map.
static constexpr char const* Metallic = "uniMaterial.metallicMap";   ///< Metalness map.
static constexpr char const* Roughness = "uniMaterial.roughnessMap"; ///< Roughness map.
static constexpr char const* Sheen = "uniMaterial.sheenMap";         ///< Sheen color (RGB) & roughness (A) map.

// Legacy textures
static constexpr char const* Specular = "uniMaterial.specularMap"; ///< Specular map
static constexpr char const* Opacity = "uniMaterial.opacityMap";   ///< Opacity map.
static constexpr char const* Bump = "uniMaterial.bumpMap";         ///< Bump map.
}

class Material {
public:
    Material() = default;
    /// Creates a material with a predefined type.
    /// \param type Material type to apply.
    explicit Material(MaterialType type) { load_type(type); }
    Material(Material&&) noexcept = default;

    Material& operator=(Material const&) = delete;
    Material& operator=(Material&&) noexcept = default;

    [[nodiscard]] RenderShaderProgram const& get_program() const { return program; }

    [[nodiscard]] RenderShaderProgram& get_program() { return program; }

    /// Checks if the material has any attribute or texture.
    /// \return True if the material has no attribute and texture, false otherwise.
    [[nodiscard]] bool empty() const
    {
        return (program.get_attribute_count() == 0 && program.get_texture_count() == 0);
    }

    [[nodiscard]] Material clone() const { return *this; }

    /// Loads a predefined material type, setting default shaders & adding all needed attributes & textures if they do
    /// not exist yet.
    /// \param type Material type to apply.
    void load_type(MaterialType type);

private:
    Material(Material const& material) : program{material.program.clone()} {}

    RenderShaderProgram program{};
};
}