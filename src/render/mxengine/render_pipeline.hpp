#pragma once

// #include "Core/BoundingObjects/FrustrumCuller.h"
#include "objects/rectangle_object.hpp"
#include "objects/skybox_object.hpp"
#include "objects/render_helper_object.hpp"
#include "objects/point_light_instanced_object.hpp"
#include "objects/spot_light_instanced_object.hpp"
// #include "RenderUtilities/RenderStatistics.h"
// #include "Core/Resources/ACESCurve.h"
// #include "Core/Resources/Material.h"
// #include "Utilities/String/String.h"

#include "platform/graphic_api.hpp"

namespace xen {
class Skybox;
class CameraEffects;
class CameraToneMapping;
class CameraSSR;
class CameraSSGI;
class CameraSSAO;
class CameraGodRay;

struct DebugBufferUnit {
    std::unique_ptr<VertexArray> vao;
    size_t vertex_count;
};

struct CameraUnit {
    std::unique_ptr<FrameBuffer> g_buffer;
    std::unique_ptr<Texture> albedo_texture;
    std::unique_ptr<Texture> normal_texture;
    std::unique_ptr<Texture> material_texture;
    std::unique_ptr<Texture> depth_texture;
    std::unique_ptr<Texture> average_white_texture;
    std::unique_ptr<Texture> hdr_texture;
    std::unique_ptr<Texture> swap_texture1;
    std::unique_ptr<Texture> swap_texture2;

    // FrustrumCuller Culler;
    Matrix4 inverse_view_matrix;
    Matrix4 view_matrix;
    Matrix4 static_view_matrix;

    std::unique_ptr<Texture> output_texture;
    Vector3f viewport_position;

    float skybox_intensity;
    Matrix3 inversed_skybox_rotation;
    std::unique_ptr<CubeMap> skybox_texture;
    std::unique_ptr<CubeMap> irradiance_texture;

    float gamma;
    float aspect_ratio;

    bool is_perspective;
    bool render_to_texture;

    CameraEffects const* Effects;
    CameraToneMapping const* ToneMapping;
    CameraSSR const* SSR;
    CameraSSGI const* SSGI;
    CameraSSAO const* SSAO;
    CameraGodRay const* GodRay;
};

struct EnvironmentUnit {
    std::unordered_map<std::string, std::unique_ptr<Shader>> Shaders;
    std::unordered_map<std::string, std::unique_ptr<ComputeShader>> ComputeShaders;

    std::unique_ptr<Texture> DefaultMaterialMap;
    std::unique_ptr<Texture> DefaultNormalMap;
    std::unique_ptr<Texture> DefaultBlackMap;
    std::unique_ptr<Texture> DefaultGreyMap;
    std::unique_ptr<Texture> DefaultShadowMap;
    std::unique_ptr<Texture> AverageWhiteTexture;
    std::unique_ptr<Texture> DownSampleTexture;
    std::unique_ptr<Texture> EnvironmentBRDFLUT;
    std::unique_ptr<CubeMap> DefaultShadowCubeMap;
    std::unique_ptr<CubeMap> DefaultSkybox;

    VertexArrayHandle RenderVAO;
    std::unique_ptr<ShaderStorageBuffer> RenderSSBO;

    FrameBufferHandle DepthFrameBuffer;
    FrameBufferHandle PostProcessFrameBuffer;
    FrameBufferHandle BloomFrameBuffer;
    std::array<std::unique_ptr<Texture>, 2> BloomTextures;

    SkyboxObject SkyboxCubeObject;
    DebugBufferUnit DebugBufferObject;
    RectangleObject RectangularObject;
    size_t DefaultBaseInstance;

    Vector2ui Viewport;
    float TimeDelta;

    uint8_t MainCameraIndex;
    bool OverlayDebugDraws;
    bool RenderToDefaultFrameBuffer;
};

struct DirectionalLightUnit {
    std::unique_ptr<Texture> ShadowMap;
    std::array<Matrix4, 3> ProjectionMatrices;
    std::array<Matrix4, 3> BiasedProjectionMatrices;
    Vector3f Direction;
    float AmbientIntensity;
    Vector3f Color;
    float Intensity;
};

struct PointLightUnit : PointLightBaseData {
    std::unique_ptr<CubeMap> ShadowMap;
    Matrix4 ProjectionMatrices[6];
};

struct SpotLightUnit : SpotLightBaseData {
    TextureHandle ShadowMap;
    Matrix4 ProjectionMatrix;
    Matrix4 BiasedProjectionMatrix;
};

struct LightingSystem {
    std::vector<DirectionalLightUnit> DirectionalLights;
    std::vector<PointLightUnit> PointLights;
    std::vector<SpotLightUnit> SpotLights;
    SpotLightInstancedObject SpotLightsInstanced;
    PointLightInstancedObject PointLightsInstanced;
    RenderHelperObject PointLight;
    RenderHelperObject SpotLight;
};

struct RenderGroup {
    size_t BaseInstance;
    size_t InstanceCount;
    size_t UnitCount;
};

struct RenderUnit {
    size_t MaterialIndex;
    size_t VertexOffset;
    size_t VertexCount;
    size_t IndexOffset;
    size_t IndexCount;

    Matrix4 ModelMatrix;
    Matrix3 NormalMatrix;

    Vector3f MinAABB, MaxAABB;
#if defined(MXENGINE_DEBUG)
    const char* DebugName;
#endif
};

struct RenderList {
    std::vector<RenderGroup> Groups;
    std::vector<size_t> UnitsIndex;
};

struct ParticleSystemUnit {
    size_t ParticleBufferOffset;
    Matrix4 Transform;
    float ParticleLifetime;
    float Fading;
    size_t InvocationCount;
    size_t MaterialIndex;
    bool IsRelative;
};

struct RenderPipeline {
    EnvironmentUnit Environment;
    LightingSystem Lighting;

    RenderList ShadowCasters;
    RenderList MaskedShadowCasters;
    RenderList TransparentObjects;
    RenderList MaskedObjects;
    RenderList OpaqueObjects;
    std::vector<RenderUnit> RenderUnits;

    std::vector<ParticleSystemUnit> OpaqueParticleSystems;
    std::vector<ParticleSystemUnit> TransparentParticleSystems;
    std::vector<Material> MaterialUnits;
    std::vector<CameraUnit> Cameras;
    RenderStatistics Statistics;
};
}