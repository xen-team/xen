#include <data/image.hpp>
#include <data/image_format.hpp>
#include <data/mesh.hpp>
#include <data/obj_format.hpp>
#include <render/mesh_renderer.hpp>
#include <utils/filepath.hpp>
#include <utils/file_utils.hpp>

#include <tracy/Tracy.hpp>

namespace xen::ObjFormat {

namespace {
inline Texture2DPtr
load_texture(FilePath const& texture_filepath, Color const& default_color, bool should_use_srgb = false)
{
    ZoneScopedN("[ObjLoad]::load_texture");
    ZoneTextF("Path: %s", texture_filepath.to_utf8().c_str());

    if (!FileUtils::is_readable(texture_filepath)) {
        Log::warning(
            "[ObjLoad] Cannot load texture '" + texture_filepath +
            "'; either the file does not exist or it cannot be opened."
        );
        return Texture2D::create(default_color);
    }

    // Always apply a vertical flip to imported textures, since OpenGL maps them upside down
    return Texture2D::create(ImageFormat::load(texture_filepath, true), true, should_use_srgb);
}

inline void load_mtl(
    FilePath const& mtl_filepath, std::vector<Material>& materials,
    std::unordered_map<std::string, std::size_t>& material_correspond_indices
)
{
    ZoneScopedN("[ObjLoad]::loadMtl");
    ZoneTextF("Path: %s", mtl_filepath.to_utf8().c_str());

    Log::debug("[ObjLoad] Loading MTL file ('" + mtl_filepath + "')...");

    std::ifstream file(mtl_filepath, std::ios_base::binary);

    if (!file) {
        Log::error("[ObjLoad] Could not open the MTL file '" + mtl_filepath + "'.");
        materials.emplace_back(MaterialType::COOK_TORRANCE);
        return;
    }

    Material material;
    MaterialType material_type = MaterialType::BLINN_PHONG;

    while (!file.eof()) {
        std::string tag;
        std::string next_value;
        file >> tag >> next_value;

        if (tag[0] == 'K') { // Standard properties [K*]
            std::string second_value;
            std::string third_value;
            file >> second_value >> third_value;

            Vector3f const values(std::stof(next_value), std::stof(second_value), std::stof(third_value));

            if (tag[1] == 'd') // Diffuse/albedo factor [Kd]
                material.get_program().set_attribute(values, MaterialAttribute::BaseColor);
            else if (tag[1] == 'e') // Emissive factor [Ke]
                material.get_program().set_attribute(values, MaterialAttribute::Emissive);
            else if (tag[1] == 'a') // Ambient factor [Ka]
                material.get_program().set_attribute(values, MaterialAttribute::Ambient);
            else if (tag[1] == 's') // Specular factor [Ks]
                material.get_program().set_attribute(values, MaterialAttribute::Specular);
        }
        else if (tag[0] == 'P') { // PBR properties [P*]
            float const factor = std::stof(next_value);

            if (tag[1] == 'm') { // Metallic factor [Pm]
                material.get_program().set_attribute(factor, MaterialAttribute::Metallic);
            }
            else if (tag[1] == 'r') { // Roughness factor [Pr]
                material.get_program().set_attribute(factor, MaterialAttribute::Roughness);
            }
            else if (tag[1] == 's') { // Sheen factors [Ps]
                std::string second_value;
                std::string third_value;
                std::string fourth_value;
                file >> second_value >> third_value >> fourth_value;
                material.get_program().set_attribute(
                    Vector4f(factor, std::stof(second_value), std::stof(third_value), std::stof(fourth_value)),
                    MaterialAttribute::Sheen
                );
            }

            material_type = MaterialType::COOK_TORRANCE;
        }
        else if (tag[0] == 'm') { // Import texture [map_*]
            FilePath const texture_filepath = mtl_filepath.recover_path_to_file() + next_value;

            if (tag[4] == 'K') {   // Standard maps [map_K*]
                if (tag[5] == 'd') // Diffuse/albedo map [map_Kd]
                    material.get_program().set_texture(
                        load_texture(texture_filepath, Color::White, true), MaterialTexture::BaseColor
                    );
                else if (tag[5] == 'e') // Emissive map [map_Ke]
                    material.get_program().set_texture(
                        load_texture(texture_filepath, Color::White, true), MaterialTexture::Emissive
                    );
                else if (tag[5] == 'a') // Ambient/ambient occlusion map [map_Ka]
                    material.get_program().set_texture(
                        load_texture(texture_filepath, Color::White, true), MaterialTexture::Ambient
                    );
                else if (tag[5] == 's') // Specular map [map_Ks]
                    material.get_program().set_texture(
                        load_texture(texture_filepath, Color::White, true), MaterialTexture::Specular
                    );
            }
            else if (tag[4] == 'P') { // PBR maps [map_P*]
                if (tag[5] == 'm')    // Metallic map [map_Pm]
                    material.get_program().set_texture(
                        load_texture(texture_filepath, Color::Red), MaterialTexture::Metallic
                    );
                else if (tag[5] == 'r') // Roughness map [map_Pr]
                    material.get_program().set_texture(
                        load_texture(texture_filepath, Color::Red), MaterialTexture::Roughness
                    );
                else if (tag[5] == 's') // Sheen map [map_Ps]
                    material.get_program().set_texture(
                        load_texture(texture_filepath, Color::White, true), MaterialTexture::Sheen
                    ); // TODO: should be an RGBA texture with an alpha of 1

                material_type = MaterialType::COOK_TORRANCE;
            }
            else if (tag[4] == 'd') { // Opacity (dissolve) map [map_d]
                Texture2DPtr map = load_texture(texture_filepath, Color::White);
                map->set_filter(TextureFilter::NEAREST, TextureFilter::NEAREST, TextureFilter::NEAREST);
                material.get_program().set_texture(std::move(map), MaterialTexture::Opacity);
            }
            else if (tag[4] == 'b') { // Bump map [map_bump]
                material.get_program().set_texture(load_texture(texture_filepath, Color::White), MaterialTexture::Bump);
            }
        }
        else if (tag[0] == 'd') { // Opacity (dissolve) factor [d]
            material.get_program().set_attribute(std::stof(next_value), MaterialAttribute::Opacity);
        }
        else if (tag[0] == 'T') {
            if (tag[1] == 'r') // Transparency factor (alias, 1 - d) [Tr]
                material.get_program().set_attribute(1.f - std::stof(next_value), MaterialAttribute::Opacity);
        }
        else if (tag[0] == 'b') { // Bump map (alias) [bump]
            material.get_program().set_texture(
                load_texture(mtl_filepath.recover_path_to_file() + next_value, Color::White), MaterialTexture::Bump
            );
        }
        else if (tag[0] == 'n') {
            if (tag[1] == 'o') { // Normal map [norm]
                material.get_program().set_texture(
                    load_texture(mtl_filepath.recover_path_to_file() + next_value, Color::Aqua), MaterialTexture::Normal
                );
            }
            else if (tag[1] == 'e') { // New material [newmtl]
                material_correspond_indices.emplace(next_value, material_correspond_indices.size());

                if (material.empty())
                    continue;

                material.load_type(material_type);
                materials.emplace_back(std::move(material));

                material = Material();
                material_type = MaterialType::BLINN_PHONG;
            }
        }
        else {
            std::getline(file, tag); // Skip the rest of the line
        }
    }

    material.load_type(material_type);
    materials.emplace_back(std::move(material));

    Log::debug("[ObjLoad] Loaded MTL file (" + std::to_string(materials.size()) + " material(s) loaded)");
}
}

std::pair<Mesh, MeshRenderer> load(FilePath const& filepath)
{
    ZoneScopedN("ObjFormat::load");
    ZoneTextF("Path: %s", filepath.to_utf8().c_str());

    Log::debug("[ObjLoad] Loading OBJ file ('" + filepath + "')...");

    std::ifstream file(filepath, std::ios_base::binary);

    if (!file) {
        throw std::invalid_argument("Error: Couldn't open the OBJ file '" + filepath + '\'');
    }

    Mesh mesh;
    MeshRenderer mesh_renderer;

    mesh.add_submesh();
    mesh_renderer.add_submesh_renderer();

    std::unordered_map<std::string, size_t> material_correspond_indices;

    std::vector<Vector3f> positions;
    std::vector<Vector2f> texcoords;
    std::vector<Vector3f> normals;

    std::vector<std::vector<int64_t>> pos_indices(1);
    std::vector<std::vector<int64_t>> texcoords_indices(1);
    std::vector<std::vector<int64_t>> normals_indices(1);

    while (!file.eof()) {
        std::string line;
        file >> line;

        if (line[0] == 'v') {
            if (line[1] == 'n') { // Normal
                Vector3f normal_triplet;

                file >> normal_triplet.x >> normal_triplet.y >> normal_triplet.z;

                normals.emplace_back(std::move(normal_triplet));
            }
            else if (line[1] == 't') { // Texcoords
                Vector2f texcoords_triplet;

                file >> texcoords_triplet.x >> texcoords_triplet.y;

                texcoords.emplace_back(std::move(texcoords_triplet));
            }
            else { // Position
                Vector3f position_triplet;

                file >> position_triplet.x >> position_triplet.y >> position_triplet.z;

                positions.emplace_back(std::move(position_triplet));
            }
        }
        else if (line[0] == 'f') { // Faces
            std::getline(file, line);

            constexpr char delimiter = '/';
            auto const nb_vertices = static_cast<uint16_t>(std::count(line.cbegin(), line.cend(), ' '));
            auto const nb_parts =
                static_cast<uint8_t>(std::count(line.cbegin(), line.cend(), delimiter) / nb_vertices + 1);
            bool const quad_faces = (nb_vertices == 4);

            std::stringstream indices_stream(line);
            std::vector<int64_t> part_indices(nb_parts * nb_vertices);
            std::string vertex;

            for (size_t vert_index = 0; vert_index < nb_vertices; ++vert_index) {
                indices_stream >> vertex;

                std::stringstream vert_parts(vertex);
                std::string part;
                uint8_t part_index = 0;

                while (std::getline(vert_parts, part, delimiter)) {
                    if (!part.empty()) {
                        part_indices[part_index * nb_parts + vert_index + (part_index * quad_faces)] = std::stol(part);
                    }

                    ++part_index;
                }
            }

            if (quad_faces) {
                pos_indices.back().emplace_back(part_indices[0]);
                pos_indices.back().emplace_back(part_indices[2]);
                pos_indices.back().emplace_back(part_indices[3]);

                texcoords_indices.back().emplace_back(part_indices[4]);
                texcoords_indices.back().emplace_back(part_indices[6]);
                texcoords_indices.back().emplace_back(part_indices[7]);

                normals_indices.back().emplace_back(part_indices[8]);
                normals_indices.back().emplace_back(part_indices[10]);
                normals_indices.back().emplace_back(part_indices[11]);
            }

            pos_indices.back().emplace_back(part_indices[0]);
            pos_indices.back().emplace_back(part_indices[1]);
            pos_indices.back().emplace_back(part_indices[2]);

            texcoords_indices.back().emplace_back(part_indices[3 + quad_faces]);
            texcoords_indices.back().emplace_back(part_indices[4 + quad_faces]);
            texcoords_indices.back().emplace_back(part_indices[5 + quad_faces]);

            auto const quad_stride = static_cast<uint8_t>(quad_faces * 2);

            normals_indices.back().emplace_back(part_indices[6 + quad_stride]);
            normals_indices.back().emplace_back(part_indices[7 + quad_stride]);
            normals_indices.back().emplace_back(part_indices[8 + quad_stride]);
        }
        else if (line[0] == 'm') { // Material import (mtllib)
            std::string mtl_filename;
            file >> mtl_filename;

            std::string const mtl_filepath = filepath.recover_path_to_file() + mtl_filename;
            load_mtl(mtl_filepath, mesh_renderer.get_materials(), material_correspond_indices);
        }
        else if (line[0] == 'u') { // Material usage (usemtl)
            if (material_correspond_indices.empty()) {
                continue;
            }

            std::string material_name;
            file >> material_name;

            auto const correspond_material = material_correspond_indices.find(material_name);

            if (correspond_material == material_correspond_indices.cend()) {
                Log::error("[ObjLoad] No corresponding material found with the name '" + material_name + "'.");
            }
            else {
                mesh_renderer.get_submesh_renderers().back().set_material_index(correspond_material->second);
            }
        }
        else if (line[0] == 'o' || line[0] == 'g') {
            if (!pos_indices.front().empty()) {
                size_t const new_size = pos_indices.size() + 1;
                pos_indices.resize(new_size);
                texcoords_indices.resize(new_size);
                normals_indices.resize(new_size);

                mesh.add_submesh();
                mesh_renderer.add_submesh_renderer().set_material_index(std::numeric_limits<size_t>::max());
            }

            std::getline(file, line);
        }
        else {
            std::getline(file, line); // Skip the rest of the line
        }
    }

    auto const pos_count = static_cast<int64_t>(positions.size());
    auto const tex_count = static_cast<int64_t>(texcoords.size());
    auto const norm_count = static_cast<int64_t>(normals.size());

    std::map<std::array<size_t, 3>, uint> indices_map;

    for (size_t submesh_index = 0; submesh_index < mesh.get_submeshes().size(); ++submesh_index) {
        Submesh& submesh = mesh.get_submeshes()[submesh_index];
        indices_map.clear();

        for (size_t part_index = 0; part_index < pos_indices[submesh_index].size(); ++part_index) {
            // Face (vertices indices triplets), containing position/texcoords/normals
            // vert_indices[i][j] -> vertex i, feature j (j = 0 -> position, j = 1 -> texcoords, j = 2 -> normal)
            std::array<std::array<size_t, 3>, 3> vert_indices{};

            // First vertex information
            int64_t temp_index = pos_indices[submesh_index][part_index];
            vert_indices[0][0] =
                (temp_index < 0 ? static_cast<size_t>(temp_index + pos_count) : static_cast<size_t>(temp_index - 1));

            temp_index = texcoords_indices[submesh_index][part_index];
            vert_indices[0][1] =
                (temp_index < 0 ? static_cast<size_t>(temp_index + tex_count) : static_cast<size_t>(temp_index - 1));

            temp_index = normals_indices[submesh_index][part_index];
            vert_indices[0][2] =
                (temp_index < 0 ? static_cast<size_t>(temp_index + norm_count) : static_cast<size_t>(temp_index - 1));

            ++part_index;

            // Second vertex information
            temp_index = pos_indices[submesh_index][part_index];
            vert_indices[1][0] =
                (temp_index < 0 ? static_cast<size_t>(temp_index + pos_count) : static_cast<size_t>(temp_index - 1));

            temp_index = texcoords_indices[submesh_index][part_index];
            vert_indices[1][1] =
                (temp_index < 0 ? static_cast<size_t>(temp_index + tex_count) : static_cast<size_t>(temp_index - 1));

            temp_index = normals_indices[submesh_index][part_index];
            vert_indices[1][2] =
                (temp_index < 0 ? static_cast<size_t>(temp_index + norm_count) : static_cast<size_t>(temp_index - 1));

            ++part_index;

            // Third vertex information
            temp_index = pos_indices[submesh_index][part_index];
            vert_indices[2][0] =
                (temp_index < 0 ? static_cast<size_t>(temp_index + pos_count) : static_cast<size_t>(temp_index - 1));

            temp_index = texcoords_indices[submesh_index][part_index];
            vert_indices[2][1] =
                (temp_index < 0 ? static_cast<size_t>(temp_index + tex_count) : static_cast<size_t>(temp_index - 1));

            temp_index = normals_indices[submesh_index][part_index];
            vert_indices[2][2] =
                (temp_index < 0 ? static_cast<size_t>(temp_index + norm_count) : static_cast<size_t>(temp_index - 1));

            std::array<Vector3f, 3> const face_positions = {
                positions[vert_indices[0][0]], positions[vert_indices[1][0]], positions[vert_indices[2][0]]
            };

            std::array<Vector2f, 3> face_texcoords{};
            if (!texcoords.empty()) {
                face_texcoords[0] = texcoords[vert_indices[0][1]];
                face_texcoords[1] = texcoords[vert_indices[1][1]];
                face_texcoords[2] = texcoords[vert_indices[2][1]];
            }

            std::array<Vector3f, 3> face_normals{};
            if (!normals.empty()) {
                face_normals[0] = normals[vert_indices[0][2]];
                face_normals[1] = normals[vert_indices[1][2]];
                face_normals[2] = normals[vert_indices[2][2]];
            }

            for (uint8_t vert_part_index = 0; vert_part_index < 3; ++vert_part_index) {
                auto const index_it = indices_map.find(vert_indices[vert_part_index]);

                if (index_it != indices_map.cend()) {
                    submesh.get_triangle_indices().emplace_back(index_it->second);
                    continue;
                }

                Vertex vert{
                    face_positions[vert_part_index], face_texcoords[vert_part_index], face_normals[vert_part_index]
                };

                submesh.get_triangle_indices().emplace_back(static_cast<uint>(indices_map.size()));
                indices_map.emplace(vert_indices[vert_part_index], static_cast<uint>(indices_map.size()));
                submesh.get_vertices().emplace_back(std::move(vert));
            }
        }
    }

    mesh.compute_tangents();

    // Creating the mesh renderer from the mesh's data
    mesh_renderer.load(mesh);

    Log::vdebug(
        "[ObjLoad] Loaded OBJ file ({} submesh(es), {} vertices, {} triangles, {} material(s))",
        mesh.get_submeshes().size(), mesh.recover_vertex_count(), mesh.recover_triangle_count(),
        mesh_renderer.get_materials().size()
    );

    return {std::move(mesh), std::move(mesh_renderer)};
}
}