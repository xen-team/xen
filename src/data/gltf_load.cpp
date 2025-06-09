#include <BulletCollision/CollisionShapes/btConvexHullShape.h>
#include <data/gltf_format.hpp>
#include <data/image.hpp>
#include <data/image_format.hpp>
#include <data/mesh.hpp>
#include <math/transform/transform.hpp>
#include <render/mesh_renderer.hpp>
#include <utils/filepath.hpp>
#include <utils/file_utils.hpp>

#include "fastgltf/core.hpp"
#include "fastgltf/math.hpp"
#include "fastgltf/tools.hpp"
#include "physics/colliders/convex_hull_collider.hpp"
#include "physics/colliders/triangle_mesh_collider.hpp"
#include "physics/rigidbody.hpp"
#include "world.hpp"

#include <tracy/Tracy.hpp>

namespace xen {
namespace {
Transform load_transform(fastgltf::Node const& node)
{
    auto const* transform = std::get_if<fastgltf::TRS>(&node.transform);

    if (transform == nullptr) { // Shouldn't happen with the option that splits a matrix in TRS components
        throw std::invalid_argument("[GltfLoad] Unexpected node transform type.");
    }

    return Transform(
        Vector3f(transform->translation.x(), transform->translation.y(), transform->translation.z()),
        Quaternion(transform->rotation.x(), transform->rotation.y(), transform->rotation.z(), transform->rotation.w()),
        Vector3f(transform->scale.x(), transform->scale.y(), transform->scale.z())
    );
}

void compute_node_transform_recursive(
    size_t node_index, fastgltf::Asset const& asset, std::optional<Transform> const& parent_transform,
    std::vector<std::optional<Transform>>& mesh_transforms
)
{
    ZoneScopedN("[GltfLoad]::compute_node_transform");

    fastgltf::Node const& current_node = asset.nodes[node_index];

    Transform current_local = load_transform(current_node);
    Transform current_global = current_local;

    if (parent_transform.has_value()) {
        current_global.set_position(
            parent_transform->get_position() +
            parent_transform->get_rotation() * (current_local.get_position() * parent_transform->get_scale())
        );
        current_global.set_rotation((parent_transform->get_rotation() * current_local.get_rotation()).normalize());
        current_global.scale(parent_transform->get_scale());
    }

    if (current_node.meshIndex.has_value()) {
        size_t const mesh_index = *current_node.meshIndex;
        if (mesh_index < mesh_transforms.size()) {
            mesh_transforms[mesh_index] = current_global;
        }
    }

    for (size_t const child_index : current_node.children) {
        compute_node_transform_recursive(child_index, asset, current_global, mesh_transforms);
    }
}

std::vector<std::optional<Transform>> load_transforms(fastgltf::Asset const& asset)
{
    std::vector<std::optional<Transform>> transforms;
    transforms.resize(asset.meshes.size());

    if (!asset.scenes.empty()) {
        size_t scene_index = asset.defaultScene.value_or(0);
        fastgltf::Scene const& scene = asset.scenes[scene_index];
        for (size_t node_index : scene.nodeIndices) {
            compute_node_transform_recursive(node_index, asset, std::nullopt, transforms);
        }
    }
    else {
        Log::warning("[GltfLoad] No scenes found, transform loading might be incomplete.");
    }

    return transforms;
}

template <typename T, typename FuncT>
void load_vertex_data(
    fastgltf::Asset const& asset, fastgltf::Primitive const& primitive, std::string_view attribute_name,
    FuncT&& callback
)
{
    static_assert(std::is_invocable_v<FuncT, T, size_t>);

    ZoneScopedN("[GltfLoad]::load_vertex_data");

    auto const attribute_iter = primitive.findAttribute(attribute_name);

    if (attribute_iter == primitive.attributes.end()) {
        return;
    }

    fastgltf::iterateAccessorWithIndex<T>(
        asset, asset.accessors[attribute_iter->accessorIndex], std::forward<FuncT>(callback)
    );
}

void load_vertices(
    fastgltf::Asset const& asset, fastgltf::Primitive const& primitive, std::optional<Transform> const& transform,
    Submesh& submesh
)
{
    ZoneScopedN("[GltfLoad]::load_vertices");

    Log::debug("[GltfLoad] Loading vertices...");

    auto const position_it = primitive.findAttribute("POSITION");

    if (position_it == primitive.attributes.end()) {
        throw std::invalid_argument("Error: Required 'POSITION' attribute not found in the glTF file.");
    }

    fastgltf::Accessor const& position_accessor = asset.accessors[position_it->accessorIndex];

    if (!position_accessor.bufferViewIndex.has_value()) {
        return;
    }

    std::vector<Vertex>& vertices = submesh.get_vertices();
    vertices.resize(position_accessor.count);

    fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec3>(
        asset, position_accessor,
        [&vertices](fastgltf::math::fvec3 position, size_t vertex_index) {
            vertices[vertex_index].position = Vector3f(position.x(), position.y(), position.z());
        }
    );

    load_vertex_data<fastgltf::math::fvec2>(
        asset, primitive, "TEXCOORD_0",
        [&vertices](fastgltf::math::fvec2 uv, size_t vertex_index) {
            auto& vertex_texcoords = vertices[vertex_index].texcoords;
            vertex_texcoords = Vector2f(uv.x(), uv.y());
        }
    );

    load_vertex_data<fastgltf::math::fvec3>(
        asset, primitive, "NORMAL",
        [&vertices](fastgltf::math::fvec3 normal, size_t vertex_index) {
            vertices[vertex_index].normal = Vector3f(normal.x(), normal.y(), normal.z());
        }
    );

    bool const has_tangents = (primitive.findAttribute("TANGENT") != primitive.attributes.end());

    if (has_tangents) {
        load_vertex_data<fastgltf::math::fvec4>(
            asset, primitive, "TANGENT",
            [&vertices](fastgltf::math::fvec4 tangent, size_t vertex_index) {
                // The tangent's input W component is either 1 or -1 and represents the handedness
                // See: https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#meshes-overview
                vertices[vertex_index].tangent = Vector3f(tangent.x(), tangent.y(), tangent.z()) * tangent.w();
            }
        );
    }
    else {
        submesh.compute_tangents();
    }

    if (transform.has_value()) {
        for (Vertex& vert : submesh.get_vertices()) {
            vert.position =
                transform->get_position() + transform->get_rotation() * (vert.position * transform->get_scale());
            vert.normal = (transform->get_rotation() * vert.normal).normalize();
            vert.tangent = (transform->get_rotation() * vert.tangent).normalize();
        }
    }

    Log::debug("[GltfLoad] Loaded vertices");
}

void load_indices(
    fastgltf::Asset const& asset, fastgltf::Accessor const& indices_accessor, std::vector<uint32_t>& indices
)
{
    ZoneScopedN("[GltfLoad]::load_indices");

    Log::debug("[GltfLoad] Loading indices...");

    if (!indices_accessor.bufferViewIndex.has_value()) {
        throw std::invalid_argument("Error: Missing glTF buffer to load indices from.");
    }

    indices.resize(indices_accessor.count);
    fastgltf::copyFromAccessor<uint32_t>(asset, indices_accessor, indices.data());

    Log::debug("[GltfLoad] Loaded indices");
}

std::pair<Mesh, MeshRendererData>
load_meshes(fastgltf::Asset const& asset, std::vector<std::optional<Transform>> const& transforms)
{
    ZoneScopedN("[GltfLoad]::load_meshes");

    std::vector<fastgltf::Mesh> const& meshes = asset.meshes;

    Log::vdebug("[GltfLoad] Loading {} mesh(es)...", meshes.size());

    Mesh loaded_mesh;
    MeshRendererData loaded_mesh_renderer;

    for (size_t mesh_index = 0; mesh_index < meshes.size(); ++mesh_index) {
        for (fastgltf::Primitive const& primitive : meshes[mesh_index].primitives) {
            if (!primitive.indicesAccessor.has_value()) {
                throw std::invalid_argument("Error: The glTF file requires having indexed geometry.");
            }

            Submesh& submesh = loaded_mesh.add_submesh();
            SubmeshRenderer& submesh_renderer = loaded_mesh_renderer.add_submesh_renderer();

            // Indices must be loaded first as they are needed to compute the tangents if necessary
            load_indices(asset, asset.accessors[*primitive.indicesAccessor], submesh.get_triangle_indices());
            load_vertices(asset, primitive, transforms[mesh_index], submesh);

            submesh_renderer.load(
                submesh,
                (primitive.type == fastgltf::PrimitiveType::Triangles ? RenderMode::TRIANGLE : RenderMode::POINT)
            );
            submesh_renderer.set_material_index(primitive.materialIndex.value_or(0));
        }
    }

    Log::debug("[GltfLoad] Loaded mesh(es)");

    return {std::move(loaded_mesh), std::move(loaded_mesh_renderer)};
}

std::vector<std::optional<Image>> load_images(
    std::vector<fastgltf::Image> const& images, std::vector<fastgltf::Buffer> const& buffers,
    std::vector<fastgltf::BufferView> const& buffer_views, FilePath const& root_filepath
)
{
    ZoneScopedN("[GltfLoad]::load_images");

    Log::vdebug("[GltfLoad] Loading {} image(s)...", images.size());

    std::vector<std::optional<Image>> loaded_images;
    loaded_images.reserve(images.size());

    auto const loadFailure = [&loaded_images](auto const&) {
        Log::error("[GltfLoad] Cannot find a suitable way of loading an image.");
        loaded_images.emplace_back(std::nullopt);
    };

    for (fastgltf::Image const& image : images) {
        std::visit(
            fastgltf::visitor{
                [&loaded_images, &root_filepath](fastgltf::sources::URI const& image_path) {
                    loaded_images.emplace_back(ImageFormat::load(root_filepath + image_path.uri.path()));
                },
                [&loaded_images](fastgltf::sources::Vector const& image_data) {
                    auto const* image_bytes = reinterpret_cast<unsigned char const*>(image_data.bytes.data());
                    loaded_images.emplace_back(ImageFormat::load_from_data(image_bytes, image_data.bytes.size()));
                },
                [&buffer_views, &buffers, &loaded_images,
                 &loadFailure](fastgltf::sources::BufferView const& bufferViewSource) {
                    fastgltf::BufferView const& image_view = buffer_views[bufferViewSource.bufferViewIndex];
                    fastgltf::Buffer const& image_buffer = buffers[image_view.bufferIndex];

                    std::visit(
                        fastgltf::visitor{
                            [&loaded_images, &image_view](fastgltf::sources::Array const& image_data) {
                                auto const* image_bytes =
                                    reinterpret_cast<unsigned char const*>(image_data.bytes.data());
                                loaded_images.emplace_back(ImageFormat::load_from_data(
                                    image_bytes + image_view.byteOffset, image_view.byteLength
                                ));
                            },
                            loadFailure
                        },
                        image_buffer.data
                    );
                },
                loadFailure
            },
            image.data
        );
    }

    Log::debug("[GltfLoad] Loaded image(s)");

    return loaded_images;
}

Image extract_ambient_occlusion_image(Image const& occlusion_image)
{
    Image ambient_image(occlusion_image.get_size(), ImageColorspace::GRAY, occlusion_image.get_data_type());

    auto const occlusion_length = occlusion_image.get_width() * occlusion_image.get_height();

    for (size_t i = 0; i < occlusion_length; ++i) {
        size_t const final_index = i * occlusion_image.get_channel_count();

        // The occlusion is located in the red (1st) channel
        // See: https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#_material_occlusiontexture
        if (occlusion_image.get_data_type() == ImageDataType::BYTE)
            static_cast<uint8_t*>(ambient_image.data())[i] =
                static_cast<uint8_t const*>(occlusion_image.data())[final_index];
        else
            static_cast<float*>(ambient_image.data())[i] =
                static_cast<float const*>(occlusion_image.data())[final_index];
    }

    return ambient_image;
}

std::pair<Image, Image> extract_metalness_roughness_images(Image const& metal_roughness_image)
{
    Image metalness_image(
        metal_roughness_image.get_size(), ImageColorspace::GRAY, metal_roughness_image.get_data_type()
    );
    Image roughness_image(
        metal_roughness_image.get_size(), ImageColorspace::GRAY, metal_roughness_image.get_data_type()
    );

    for (size_t i = 0; i < metal_roughness_image.get_width() * metal_roughness_image.get_height(); ++i) {
        size_t const final_index = i * metal_roughness_image.get_channel_count();

        // The metalness & roughness are located respectively in the blue (3rd) & green (2nd) channels
        // See:
        // https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#_material_pbrmetallicroughness_metallicroughnesstexture
        if (metal_roughness_image.get_data_type() == ImageDataType::BYTE) {
            static_cast<uint8_t*>(metalness_image.data())[i] =
                static_cast<uint8_t const*>(metal_roughness_image.data())[final_index + 2];
            static_cast<uint8_t*>(roughness_image.data())[i] =
                static_cast<uint8_t const*>(metal_roughness_image.data())[final_index + 1];
        }
        else {
            static_cast<float*>(metalness_image.data())[i] =
                static_cast<float const*>(metal_roughness_image.data())[final_index + 2];
            static_cast<float*>(roughness_image.data())[i] =
                static_cast<float const*>(metal_roughness_image.data())[final_index + 1];
        }
    }

    return {std::move(metalness_image), std::move(roughness_image)};
}

Image merge_images(Image const& image1, Image const& image2)
{
    if (image1.empty()) {
        return image2;
    }

    if (image2.empty() || image1 == image2) {
        return image1;
    }

    if (image1.get_size() != image2.get_size() || image1.get_data_type() != image2.get_data_type()) {
        throw std::invalid_argument("[GltfLoad] The images' attributes need to be the same in order to be merged");
    }

    if (image1.get_data_type() != ImageDataType::BYTE) {
        throw std::invalid_argument("[GltfLoad] Images with a floating-point data type cannot be merged");
    }

    // TODO: the channels to copy from each image should be definable
    uint8_t const total_channel_count = image1.get_channel_count() + image2.get_channel_count();
    Log::rt_assert("Error: There shouldn't be only one channel to be merged." && total_channel_count > 1);

    if (total_channel_count > 4) {
        throw std::invalid_argument("[GltfLoad] Too many channels to merge images into");
    }

    bool const is_srgb =
        (image1.get_colorspace() == ImageColorspace::SRGB || image2.get_colorspace() == ImageColorspace::SRGB);
    ImageColorspace const colorspace =
        (total_channel_count == 2 ?
             ImageColorspace::GRAY_ALPHA :
             (total_channel_count == 3 ? ImageColorspace::RGB :
                                         (is_srgb ? ImageColorspace::SRGBA : ImageColorspace::RGBA)));

    Image res(image1.get_size(), colorspace, image1.get_data_type());

    for (uint32_t height_index = 0; height_index < image1.get_height(); ++height_index) {
        for (uint32_t width_index = 0; width_index < image1.get_width(); ++width_index) {
            for (uint8_t channel_index = 0; channel_index < image1.get_channel_count(); ++channel_index)
                res.set_byte_value(
                    width_index, height_index, channel_index,
                    image1.recover_byte_value(width_index, height_index, channel_index)
                );

            for (uint8_t channel_index = 0; channel_index < image2.get_channel_count(); ++channel_index)
                res.set_byte_value(
                    width_index, height_index, channel_index + image1.get_channel_count(),
                    image2.recover_byte_value(width_index, height_index, channel_index)
                );
        }
    }

    return res;
}

template <template <typename> typename OptionalT, typename TextureInfoT, typename FuncT>
void load_texture(
    OptionalT<TextureInfoT> const& texture_info, std::vector<fastgltf::Texture> const& textures,
    std::vector<std::optional<Image>> const& images, FuncT const& callback
)
{
    static_assert(std::is_base_of_v<fastgltf::TextureInfo, TextureInfoT>);

    ZoneScopedN("[GltfLoad]::load_texture");

    if (!texture_info.has_value()) {
        return;
    }

    fastgltf::Optional<size_t> const& image_index = textures[texture_info->textureIndex].imageIndex;

    if (!image_index.has_value() || !images[*image_index].has_value()) {
        return;
    }

    callback(*images[*image_index]);
}

void load_sheen(
    fastgltf::MaterialSheen const& mat_sheen, std::vector<fastgltf::Texture> const& textures,
    std::vector<std::optional<Image>> const& images, RenderShaderProgram& material_program
)
{
    ZoneScopedN("[GltfLoad]::load_sheen");

    Vector4f const sheen_factors(
        mat_sheen.sheenColorFactor.x(), mat_sheen.sheenColorFactor.y(), mat_sheen.sheenColorFactor.z(),
        mat_sheen.sheenRoughnessFactor
    );
    material_program.set_attribute(sheen_factors, MaterialAttribute::Sheen);

    if (!mat_sheen.sheenColorTexture.has_value() && !mat_sheen.sheenRoughnessTexture.has_value()) {
        return;
    }

    // If the textures are the same, load either of them
    if (mat_sheen.sheenColorTexture && mat_sheen.sheenRoughnessTexture &&
        mat_sheen.sheenColorTexture->textureIndex == mat_sheen.sheenRoughnessTexture->textureIndex) {
        load_texture(mat_sheen.sheenColorTexture, textures, images, [&material_program](Image const& image) {
            material_program.set_texture(Texture2D::create(image, true, true), MaterialTexture::Sheen);
        });

        return;
    }

    // If either only one texture is set or they are different, merge them
    Image sheen_color_image;
    Image sheen_roughness_image;
    load_texture(mat_sheen.sheenColorTexture, textures, images, [&sheen_color_image](Image image) {
        sheen_color_image = std::move(image);
    });
    load_texture(mat_sheen.sheenRoughnessTexture, textures, images, [&sheen_roughness_image](Image image) {
        sheen_roughness_image = std::move(image);
    });
    material_program.set_texture(
        Texture2D::create(merge_images(sheen_color_image, sheen_roughness_image), true, true), MaterialTexture::Sheen
    );
}

void load_materials(
    std::vector<fastgltf::Material> const& materials, std::vector<fastgltf::Texture> const& textures,
    std::vector<std::optional<Image>> const& images, MeshRendererData& mesh_renderer
)
{
    ZoneScopedN("[GltfLoad]::load_materials");

    Log::vdebug("[GltfLoad] Loading {} material(s)...", materials.size());

    mesh_renderer.get_materials().clear();

    for (fastgltf::Material const& material : materials) {
        Material& loaded_material = mesh_renderer.add_material();
        RenderShaderProgram& material_program = loaded_material.get_program();

        material_program.set_attribute(
            Vector3f(
                material.pbrData.baseColorFactor[0], material.pbrData.baseColorFactor[1],
                material.pbrData.baseColorFactor[2]
            ),
            MaterialAttribute::BaseColor
        );
        material_program.set_attribute(
            Vector3f(material.emissiveFactor[0], material.emissiveFactor[1], material.emissiveFactor[2]) *
                material.emissiveStrength,
            MaterialAttribute::Emissive
        );
        material_program.set_attribute(material.pbrData.metallicFactor, MaterialAttribute::Metallic);
        material_program.set_attribute(material.pbrData.roughnessFactor, MaterialAttribute::Roughness);

        load_texture(material.pbrData.baseColorTexture, textures, images, [&material_program](Image const& image) {
            material_program.set_texture(Texture2D::create(image, true, true), MaterialTexture::BaseColor);
        });

        load_texture(material.emissiveTexture, textures, images, [&material_program](Image const& image) {
            material_program.set_texture(Texture2D::create(image, true, true), MaterialTexture::Emissive);
        });

        load_texture(material.occlusionTexture, textures, images, [&material_program](Image const& image) {
            Image const ambientOcclusionImg = extract_ambient_occlusion_image(image);
            material_program.set_texture(Texture2D::create(ambientOcclusionImg), MaterialTexture::Ambient);
        });

        load_texture(material.normalTexture, textures, images, [&material_program](Image const& image) {
            material_program.set_texture(Texture2D::create(image), MaterialTexture::Normal);
        });

        load_texture(
            material.pbrData.metallicRoughnessTexture, textures, images,
            [&material_program](Image const& image) {
                auto const [metalness_image, roughness_image] = extract_metalness_roughness_images(image);
                material_program.set_texture(Texture2D::create(metalness_image), MaterialTexture::Metallic);
                material_program.set_texture(Texture2D::create(roughness_image), MaterialTexture::Roughness);
            }
        );

        if (material.sheen) {
            load_sheen(*material.sheen, textures, images, material_program);
        }

        loaded_material.load_type(MaterialType::COOK_TORRANCE);
    }

    Log::debug("[GltfLoad] Loaded material(s)");
}
}

namespace GltfFormat {
std::pair<Mesh, MeshRendererData> load(FilePath const& filepath)
{
    ZoneScopedN("GltfFormat::load");
    ZoneTextF("Path: %s", filepath.to_utf8().c_str());

    Log::debug("[GltfLoad] Loading glTF file ('" + filepath + "')...");

    if (!FileUtils::is_readable(filepath))
        throw std::invalid_argument(
            "Error: The glTF file '" + filepath + "' either does not exist or cannot be opened."
        );

    fastgltf::Expected<fastgltf::GltfDataBuffer> data = fastgltf::GltfDataBuffer::FromPath(filepath.get_path());

    if (data.error() != fastgltf::Error::None) {
        throw std::invalid_argument("Error: Could not load the glTF file.");
    }

    FilePath const parent_path = filepath.recover_path_to_file();

    constexpr fastgltf::Extensions extensions = fastgltf::Extensions::KHR_materials_sheen;
    fastgltf::Parser parser(extensions);

    fastgltf::Expected<fastgltf::Asset> asset = parser.loadGltf(
        data.get(), parent_path.get_path(),
        fastgltf::Options::LoadExternalBuffers | fastgltf::Options::DecomposeNodeMatrices
    );

    if (asset.error() != fastgltf::Error::None) {
        throw std::invalid_argument("Error: Failed to load glTF: " + fastgltf::getErrorMessage(asset.error()));
    }

    std::vector<std::optional<Transform>> const transforms = load_transforms(asset.get());
    auto [mesh, mesh_renderer] = load_meshes(asset.get(), transforms);

    // auto& ent = world.add_entity_with_component<Transform>();
    // auto& map_rigidbody_component = ent.add_component<Rigidbody>(0.0f, 0.7f);

    std::vector<std::optional<Image>> const images =
        load_images(asset->images, asset->buffers, asset->bufferViews, parent_path);
    load_materials(asset->materials, asset->textures, images, mesh_renderer);

    Log::vdebug(
        "[GltfLoad] Loaded glTF file ({} submesh(es), {} vertices, {} triangles, {} material(s))",
        mesh.get_submeshes().size(), mesh.recover_vertex_count(), mesh.recover_triangle_count(),
        mesh_renderer.get_materials().size()
    );

    return {std::move(mesh), std::move(mesh_renderer)};
}

Rigidbody& create_map_rigidbody_from_mesh(Entity& entity, std::shared_ptr<Mesh> map_mesh)
{
    ZoneScopedN("[GltfColliderLoad]::create_map_rigidbody_from_mesh");

    if (map_mesh->get_submeshes().empty()) {
        Log::warning("[GltfColliderLoad] Map mesh has no submeshes. Cannot create rigidbody.");
        // return nullptr;
    }

    auto tri_mesh_collider = std::make_unique<TriangleMeshCollider>(*map_mesh, Transform());

    if (!tri_mesh_collider->get_triangle_mesh_interface() ||
        tri_mesh_collider->get_triangle_mesh_interface()->getNumTriangles() == 0) {
        Log::error("[GltfColliderLoad] TriangleMeshCollider was created but contains no triangles. Aborting Rigidbody "
                   "creation.");
        // return nullptr;
    }

    auto& map_rigidbody =
        entity.add_component<Rigidbody>(std::move(tri_mesh_collider), 0.f, 0.f, Vector3f(0.f), Vector3f(0.f));

    // Log::info("[GltfColliderLoad] Successfully prepared map Rigidbody. Call start() to activate with appropriate
    // transform.");
    return map_rigidbody;
}
}
}