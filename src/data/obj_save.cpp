#include <data/image.hpp>
#include <data/image_format.hpp>
#include <data/mesh.hpp>
#include <data/obj_format.hpp>
#include <render/material.hpp>
#include <render/mesh_renderer.hpp>
#include <utils/filepath.hpp>

#include <tracy/Tracy.hpp>

namespace xen::ObjFormat {
namespace {
template <typename T, size_t Size = 1>
constexpr void write_attribute(
    std::ofstream& file, std::string_view tag, RenderShaderProgram const& program, std::string_view uniform_name
)
{
    if (!program.has_attribute(uniform_name.data())) {
        return;
    }

    file << '\t' << tag;

    if constexpr (Size == 1) {
        file << ' ' << program.get_attribute<T>(uniform_name.data());
    }
    else if constexpr (Size == 2) {
        auto const& vector_attribute = program.get_attribute<Vector2f>(uniform_name.data());
        file << ' ' << vector_attribute.x << ' ' << vector_attribute.y;
    }
    else if constexpr (Size == 3) {
        auto const& vector_attribute = program.get_attribute<Vector3f>(uniform_name.data());
        file << ' ' << vector_attribute.x << ' ' << vector_attribute.y << ' ' << vector_attribute.z;
    }
    else if constexpr (Size == 4) {
        auto const& vector_attribute = program.get_attribute<Vector4f>(uniform_name.data());
        file << ' ' << vector_attribute.x << ' ' << vector_attribute.y << ' ' << vector_attribute.z << ' '
             << vector_attribute.w;
    }

    file << '\n';
}

inline void write_texture(
    std::ofstream& file, std::string_view tag, std::string const& material_name, std::string_view suffix,
    RenderShaderProgram const& program, std::string_view uniform_name
)
{
    ZoneScopedN("[ObjSave]::write_texture");

    if (!program.has_texture(uniform_name.data())) {
        return;
    }

    auto const* texture = dynamic_cast<Texture2D const*>(&program.get_texture(uniform_name.data()));

    if (texture == nullptr || texture->get_width() == 0 || texture->get_height() == 0 ||
        texture->get_colorspace() == TextureColorspace::INVALID) {
        return;
    }

    std::string const texture_path = material_name + '_' + suffix + ".png";

    file << '\t' << tag << ' ' << texture_path << '\n';
    ImageFormat::save(texture_path, texture->recover_image(), true);
}

void save_mtl(FilePath const& mtl_filepath, std::vector<Material> const& materials)
{
    ZoneScopedN("[ObjSave]::save_mtl");
    ZoneTextF("Path: %s", mtl_filepath.to_utf8().c_str());

    std::ofstream mtl_file(mtl_filepath, std::ios_base::binary);

    mtl_file << "# MTL file created with xen - https://github.com/xen-team/xen\n";

    std::string const mtl_filename = mtl_filepath.recover_filename(false).to_utf8();

    for (size_t mat_index = 0; mat_index < materials.size(); ++mat_index) {
        RenderShaderProgram const& material_program = materials[mat_index].get_program();
        std::string const material_name = mtl_filename + '_' + std::to_string(mat_index);

        mtl_file << "\nnewmtl " << material_name << '\n';

        write_attribute<float, 3>(mtl_file, "Kd", material_program, MaterialAttribute::BaseColor);
        write_attribute<float, 3>(mtl_file, "Ke", material_program, MaterialAttribute::Emissive);
        write_attribute<float, 3>(mtl_file, "Ka", material_program, MaterialAttribute::Ambient);
        write_attribute<float, 3>(mtl_file, "Ks", material_program, MaterialAttribute::Specular);
        write_attribute<float, 1>(mtl_file, "d", material_program, MaterialAttribute::Opacity);
        write_attribute<float, 1>(mtl_file, "Pm", material_program, MaterialAttribute::Metallic);
        write_attribute<float, 1>(mtl_file, "Pr", material_program, MaterialAttribute::Roughness);
        write_attribute<float, 4>(mtl_file, "Ps", material_program, MaterialAttribute::Sheen);

#if !defined(USE_OPENGL_ES)
        write_texture(mtl_file, "map_Kd", material_name, "baseColor", material_program, MaterialTexture::BaseColor);
        write_texture(mtl_file, "map_Ke", material_name, "emissive", material_program, MaterialTexture::Emissive);
        write_texture(mtl_file, "map_Ka", material_name, "ambient", material_program, MaterialTexture::Ambient);
        write_texture(mtl_file, "map_Ks", material_name, "specular", material_program, MaterialTexture::Specular);
        write_texture(mtl_file, "map_d", material_name, "opacity", material_program, MaterialTexture::Opacity);
        write_texture(mtl_file, "map_bump", material_name, "bump", material_program, MaterialTexture::Bump);
        write_texture(mtl_file, "norm", material_name, "normal", material_program, MaterialTexture::Normal);
        write_texture(mtl_file, "map_Pm", material_name, "metallic", material_program, MaterialTexture::Metallic);
        write_texture(mtl_file, "map_Pr", material_name, "roughness", material_program, MaterialTexture::Roughness);
        write_texture(mtl_file, "map_Ps", material_name, "sheen", material_program, MaterialTexture::Sheen);
#endif
    }
}
}

void save(FilePath const& filepath, Mesh const& mesh, MeshRenderer const* mesh_renderer)
{
    ZoneScopedN("ObjFormat::save");
    ZoneTextF("Path: %s", filepath.to_utf8().c_str());

    Log::debug("[ObjSave] Saving OBJ file ('" + filepath + "')...");

    std::ofstream file(filepath, std::ios_base::binary);

    if (!file)
        throw std::invalid_argument(
            "Error: Unable to create an OBJ file as '" + filepath + "'; path to file must exist"
        );

    file << "# OBJ file created with xen - https://github.com/xen-team/xen\n\n";

    if (mesh_renderer && !mesh_renderer->get_materials().empty()) {
        std::string const mtl_filename = filepath.recover_filename(false) + ".mtl";
        FilePath const mtl_filepath = filepath.recover_path_to_file() + mtl_filename;

        file << "mtllib " << mtl_filepath << "\n\n";

        save_mtl(mtl_filepath, mesh_renderer->get_materials());
    }

    std::map<Vector3f, size_t> pos_correspond_indices;
    std::map<Vector2f, size_t> tex_correspond_indices;
    std::map<Vector3f, size_t> norm_correspond_indices;

    for (Submesh const& submesh : mesh.get_submeshes()) {
        for (Vertex const& vertex : submesh.get_vertices()) {
            if (pos_correspond_indices.find(vertex.position) == pos_correspond_indices.cend()) {
                file << "v " << vertex.position.x << ' ' << vertex.position.y << ' ' << vertex.position.z << '\n';
                pos_correspond_indices.emplace(vertex.position, pos_correspond_indices.size() + 1);
            }

            if (tex_correspond_indices.find(vertex.texcoords) == tex_correspond_indices.cend()) {
                file << "vt " << vertex.texcoords.x << ' ' << vertex.texcoords.y << '\n';
                tex_correspond_indices.emplace(vertex.texcoords, tex_correspond_indices.size() + 1);
            }

            if (norm_correspond_indices.find(vertex.normal) == norm_correspond_indices.cend()) {
                file << "vn " << vertex.normal.x << ' ' << vertex.normal.y << ' ' << vertex.normal.z << '\n';
                norm_correspond_indices.emplace(vertex.normal, norm_correspond_indices.size() + 1);
            }
        }
    }

    std::string const filename = filepath.recover_filename(false).to_utf8();

    for (size_t submesh_index = 0; submesh_index < mesh.get_submeshes().size(); ++submesh_index) {
        Submesh const& submesh = mesh.get_submeshes()[submesh_index];

        file << "\no " << filename << '_' << submesh_index << '\n';

        if (mesh_renderer && !mesh_renderer->get_materials().empty())
            file << "usemtl " << filename << '_'
                 << mesh_renderer->get_submesh_renderers()[submesh_index].get_material_index() << '\n';

        for (size_t i = 0; i < submesh.get_triangle_index_count(); i += 3) {
            file << "f ";

            // First vertex
            Vertex vertex = submesh.get_vertices()[submesh.get_triangle_indices()[i]];

            auto pos_index = pos_correspond_indices.find(vertex.position)->second;
            auto tex_index = tex_correspond_indices.find(vertex.texcoords)->second;
            auto norm_index = norm_correspond_indices.find(vertex.normal)->second;

            file << pos_index << '/' << tex_index << '/' << norm_index << ' ';

            // Second vertex
            vertex = submesh.get_vertices()[submesh.get_triangle_indices()[i + 1]];

            pos_index = pos_correspond_indices.find(vertex.position)->second;
            tex_index = tex_correspond_indices.find(vertex.texcoords)->second;
            norm_index = norm_correspond_indices.find(vertex.normal)->second;

            file << pos_index << '/' << tex_index << '/' << norm_index << ' ';

            // Third vertex
            vertex = submesh.get_vertices()[submesh.get_triangle_indices()[i + 2]];

            pos_index = pos_correspond_indices.find(vertex.position)->second;
            tex_index = tex_correspond_indices.find(vertex.texcoords)->second;
            norm_index = norm_correspond_indices.find(vertex.normal)->second;

            file << pos_index << '/' << tex_index << '/' << norm_index << '\n';
        }
    }

    Log::debug("[ObjSave] Saved OBJ file");
}
}