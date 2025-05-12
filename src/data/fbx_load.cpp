#include <data/fbx_format.hpp>
#include <data/image.hpp>
#include <data/image_format.hpp>
#include <data/mesh.hpp>
#include <render/mesh_renderer.hpp>
#include <utils/filepath.hpp>
#include <utils/file_utils.hpp>

#include <tracy/Tracy.hpp>

#include <fbxsdk.h>

namespace {
inline Texture2DPtr
load_texture(FilePath const& texture_filepath, Color const& default_color, bool should_use_srgb = false)
{
    ZoneScopedN("[FbxLoad]::load_texture");
    ZoneTextF("Path: %s", texture_filepath.c_str());

    if (!FileUtils::is_readable(texture_filepath)) {
        Log::warning(
            "[FbxLoad] Cannot load texture '" + texture_filepath +
            "'; either the file does not exist or it cannot be opened."
        );
        return Texture2D::create(default_color);
    }

    // Always apply a vertical flip to imported textures, since OpenGL maps them upside down
    return Texture2D::create(ImageFormat::load(texture_filepath, true), true, should_use_srgb);
}

void load_materials(fbxsdk::FbxScene* scene, std::vector<Material>& materials, FilePath const& filepath)
{
    ZoneScopedN("[FbxLoad]::load_materials");

    for (int mat_index = 0; mat_index < scene->GetMaterialCount(); ++mat_index) {
        fbxsdk::FbxSurfaceMaterial const* fbx_material = scene->GetMaterial(mat_index);
        Material material;

        // Recovering properties

        fbxsdk::FbxPropertyT<fbxsdk::FbxDouble3> const& diffuse =
            fbx_material->FindProperty(fbxsdk::FbxSurfaceMaterial::sDiffuse);
        if (diffuse.IsValid()) {
            material.get_program().set_attribute(
                Vector3f(
                    static_cast<float>(diffuse.Get()[0]), static_cast<float>(diffuse.Get()[1]),
                    static_cast<float>(diffuse.Get()[2])
                ),
                MaterialAttribute::BaseColor
            );
        }

        fbxsdk::FbxPropertyT<fbxsdk::FbxDouble3> const& emissive =
            fbx_material->FindProperty(fbxsdk::FbxSurfaceMaterial::sEmissive);
        if (emissive.IsValid()) {
            material.get_program().set_attribute(
                Vector3f(
                    static_cast<float>(emissive.Get()[0]), static_cast<float>(emissive.Get()[1]),
                    static_cast<float>(emissive.Get()[2])
                ),
                MaterialAttribute::Emissive
            );
        }

        fbxsdk::FbxPropertyT<fbxsdk::FbxDouble3> const& ambient =
            fbx_material->FindProperty(fbxsdk::FbxSurfaceMaterial::sAmbient);
        if (ambient.IsValid()) {
            material.get_program().set_attribute(
                Vector3f(
                    static_cast<float>(ambient.Get()[0]), static_cast<float>(ambient.Get()[1]),
                    static_cast<float>(ambient.Get()[2])
                ),
                MaterialAttribute::Ambient
            );
        }

        fbxsdk::FbxPropertyT<fbxsdk::FbxDouble3> const& specular =
            fbx_material->FindProperty(fbxsdk::FbxSurfaceMaterial::sSpecular);
        if (specular.IsValid()) {
            material.get_program().set_attribute(
                Vector3f(
                    static_cast<float>(specular.Get()[0]), static_cast<float>(specular.Get()[1]),
                    static_cast<float>(specular.Get()[2])
                ),
                MaterialAttribute::Specular
            );
        }

        fbxsdk::FbxPropertyT<fbxsdk::FbxDouble> const& transparency =
            fbx_material->FindProperty(fbxsdk::FbxSurfaceMaterial::sTransparencyFactor);
        if (transparency.IsValid())
            material.get_program().set_attribute(
                1.f - static_cast<float>(transparency.Get()), MaterialAttribute::Opacity
            );

        // Recovering textures

        FilePath const texture_path = filepath.recover_path_to_file();

        auto const* diffuse_texture = static_cast<fbxsdk::FbxFileTexture*>(
            diffuse.GetSrcObject(fbxsdk::FbxCriteria::ObjectType(fbxsdk::FbxFileTexture::ClassId))
        );
        if (diffuse_texture) {
            Texture2DPtr diffuse_map =
                load_texture(texture_path + diffuse_texture->get_relative_filename(), Color::White, true);
            material.get_program().set_texture(std::move(diffuse_map), MaterialTexture::BaseColor);
        }

        auto const* emissive_texture = static_cast<fbxsdk::FbxFileTexture*>(
            emissive.GetSrcObject(fbxsdk::FbxCriteria::ObjectType(fbxsdk::FbxFileTexture::ClassId))
        );
        if (emissive_texture) {
            Texture2DPtr emissive_map =
                load_texture(texture_path + emissive_texture->get_relative_filename(), Color::White, true);
            material.get_program().set_texture(std::move(emissive_map), MaterialTexture::Emissive);
        }

        auto const* ambient_texture = static_cast<fbxsdk::FbxFileTexture*>(
            ambient.GetSrcObject(fbxsdk::FbxCriteria::ObjectType(fbxsdk::FbxFileTexture::ClassId))
        );
        if (ambient_texture) {
            Texture2DPtr ambient_map =
                load_texture(texture_path + ambient_texture->get_relative_filename(), Color::White, true);
            material.get_program().set_texture(std::move(ambient_map), MaterialTexture::Ambient);
        }

        auto const* specularTexture = static_cast<fbxsdk::FbxFileTexture*>(
            specular.GetSrcObject(fbxsdk::FbxCriteria::ObjectType(fbxsdk::FbxFileTexture::ClassId))
        );
        if (specularTexture) {
            Texture2DPtr specular_map =
                load_texture(texture_path + specularTexture->get_relative_filename(), Color::White, true);
            material.get_program().set_texture(std::move(specular_map), MaterialTexture::Specular);
        }

        // Normal map not yet handled for standard materials
        /*
        const auto normalMapProp = fbx_material->FindProperty(fbxsdk::FbxSurfaceMaterial::sNormalMap);
        if (normalMapProp.IsValid()) {
          const auto* normalTexture =
        static_cast<fbxsdk::FbxFileTexture*>(normalMapProp.GetSrcObject(fbxsdk::FbxCriteria::ObjectType(fbxsdk::FbxFileTexture::ClassId)));

          if (normalTexture) {
            Texture2DPtr normalMap = load_texture(texture_path +
        normalTexture->get_relative_filename(), Color::Aqua);
        material.get_program().set_texture(std::move(normalMap), MaterialTexture::Normal);
          }
        }
        */

        material.load_type(MaterialType::BLINN_PHONG);
        materials.emplace_back(std::move(material));
    }
}
}

namespace xen::FbxFormat {
std::pair<Mesh, MeshRenderer> load(FilePath const& filepath)
{
    ZoneScopedN("FbxFormat::load");
    ZoneTextF("Path: %s", filepath.to_utf8().c_str());

    fbxsdk::FbxManager* manager = fbxsdk::FbxManager::Create();
    manager->SetIOSettings(fbxsdk::FbxIOSettings::Create(manager, IOSROOT));

    fbxsdk::FbxScene* scene = fbxsdk::FbxScene::Create(manager, filepath.filename().c_str());

    // Importing the contents of the file into the scene
    {
        fbxsdk::FbxImporter* importer = fbxsdk::FbxImporter::Create(manager, "");
        importer->Initialize(filepath.c_str(), -1, manager->GetIOSettings());
        importer->Import(scene);
        importer->Destroy();
    }

    // Overriding the coordinates system & scene unit to match what we expect
    // See:
    // https://help.autodesk.com/view/FBX/2020/ENU/?guid=FBX_Developer_Help_nodes_and_scene_graph_fbx_scenes_scene_axis_and_unit_conversion_html
    fbxsdk::FbxAxisSystem::OpenGL.ConvertScene(scene); // OpenGL basis
    fbxsdk::FbxSystemUnit::m.ConvertScene(scene);      // Units in meters

    Mesh mesh;
    MeshRenderer mesh_renderer;

    mesh.get_submeshes().reserve(scene->GetGeometryCount());
    mesh_renderer.get_submesh_renderers().reserve(scene->GetGeometryCount());

    // Recovering geometry
    for (int mesh_index = 0; mesh_index < scene->GetGeometryCount(); ++mesh_index) {
        auto* fbx_mesh = static_cast<fbxsdk::FbxMesh*>(scene->GetGeometry(mesh_index));
        Submesh submesh;
        SubmeshRenderer submesh_renderer;

        // Recovering the mesh's global transform
        // See:
        // https://help.autodesk.com/view/FBX/2020/ENU/?guid=FBX_Developer_Help_nodes_and_scene_graph_fbx_nodes_html#transformation-data
        fbxsdk::FbxAMatrix const global_transform = fbx_mesh->GetNode()->EvaluateGlobalTransform();

        ////////////
        // Values //
        ////////////

        std::vector<Vertex>& vertices = submesh.get_vertices();

        // Recovering positions
        vertices.resize(fbx_mesh->GetControlPointsCount());

        for (int pos_index = 0; pos_index < fbx_mesh->GetControlPointsCount(); ++pos_index) {
            fbxsdk::FbxVector4 const pos = global_transform.MultT(fbx_mesh->GetControlPointAt(pos_index));

            vertices[pos_index].position.x = static_cast<float>(pos[0]);
            vertices[pos_index].position.y = static_cast<float>(pos[1]);
            vertices[pos_index].position.z = static_cast<float>(pos[2]);
        }

        // Recovering texture coordinates (UVs)
        fbx_mesh->InitTextureUV(fbx_mesh->GetControlPointsCount());
        fbxsdk::FbxGeometryElementUV const* mesh_texcoords = fbx_mesh->GetElementUV();

        if (mesh_texcoords) {
            for (int tex_index = 0; tex_index < mesh_texcoords->GetDirectArray().GetCount(); ++tex_index) {
                fbxsdk::FbxVector2 const& tex = mesh_texcoords->GetDirectArray()[tex_index];

                vertices[tex_index].texcoords.x = static_cast<float>(tex[0]);
                vertices[tex_index].texcoords.y = static_cast<float>(tex[1]);
            }
        }

        // Recovering normals
        fbx_mesh->GenerateNormals(true, true); // Re-generate normals by vertex
        fbxsdk::FbxGeometryElementNormal const* mesh_normals = fbx_mesh->GetElementNormal();

        if (mesh_normals) {
            for (int norm_index = 0; norm_index < mesh_normals->GetDirectArray().GetCount(); ++norm_index) {
                fbxsdk::FbxVector4 const norm =
                    global_transform.Inverse().MultT(mesh_normals->GetDirectArray()[norm_index]);

                vertices[norm_index].normal.x = static_cast<float>(norm[0]);
                vertices[norm_index].normal.y = static_cast<float>(norm[1]);
                vertices[norm_index].normal.z = static_cast<float>(norm[2]);
            }
        }

        // Recovering tangents
        // Not working yet, fetching/calculating way too many tangents (around 4x the amount of vertices)
        /*
        fbx_mesh->GenerateTangentsData(mesh_texcoords->GetName()); // Generate tangents using UVs
        const fbxsdk::FbxGeometryElementTangent* meshTangents = fbx_mesh->GetElementTangent();

        if (meshTangents) {
          for (int tanIndex = 0; tanIndex < meshTangents->GetDirectArray().GetCount(); ++tanIndex) {
            const fbxsdk::FbxVector4 tan = global_transform.MultR(meshTangents->GetDirectArray()[tanIndex]);

            vertices[tanIndex].tangent.x = static_cast<float>(tan[0]);
            vertices[tanIndex].tangent.y = static_cast<float>(tan[1]);
            vertices[tanIndex].tangent.z = static_cast<float>(tan[2]);
          }
        }
        */

        std::vector<uint>& indices = submesh.get_triangle_indices();

        // Process recovered data
        indices.reserve(static_cast<size_t>(fbx_mesh->GetPolygonCount()) * 3);

        for (int poly_index = 0; poly_index < fbx_mesh->GetPolygonCount(); ++poly_index) {
            indices.emplace_back(fbx_mesh->GetPolygonVertex(poly_index, 0));
            indices.emplace_back(fbx_mesh->GetPolygonVertex(poly_index, 1));
            indices.emplace_back(fbx_mesh->GetPolygonVertex(poly_index, 2));

            for (int poly_vert_index = 3; poly_vert_index < fbx_mesh->GetPolygonSize(poly_index); ++poly_vert_index) {
                indices.emplace_back(fbx_mesh->GetPolygonVertex(poly_index, 0));
                indices.emplace_back(fbx_mesh->GetPolygonVertex(poly_index, poly_vert_index - 1));
                indices.emplace_back(fbx_mesh->GetPolygonVertex(poly_index, poly_vert_index));
            }
        }

        auto const& mesh_material = fbx_mesh->GetElementMaterial();
        if (mesh_material) {
            if (mesh_material->GetMappingMode() == FbxLayerElement::EMappingMode::eAllSame) {
                // TODO: small hack to avoid segfaulting when mesh count > material count, but clearly wrong; find
                // another way
                submesh_renderer.set_material_index(std::min(mesh_index, scene->GetMaterialCount() - 1));
            }
            else {
                Log::error("[FbxLoad] Materials can't be mapped to anything other than the whole submesh.");
            }
        }

        mesh.add_submesh(std::move(submesh));
        mesh_renderer.add_submesh_renderer(std::move(submesh_renderer));

        mesh_renderer.get_submesh_renderers().back().load(mesh.get_submeshes().back());
    }

    load_materials(scene, mesh_renderer.get_materials(), filepath);

    scene->Destroy();
    manager->Destroy();

    return {std::move(mesh), std::move(mesh_renderer)};
}
}