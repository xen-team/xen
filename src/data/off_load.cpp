#include <data/off_format.hpp>
#include <data/mesh.hpp>
#include <utils/filepath.hpp>

#include <tracy/Tracy.hpp>

namespace xen::OffFormat {
Mesh load(FilePath const& filepath)
{
    ZoneScopedN("OffFormat::load");
    ZoneTextF("Path: %s", filepath.to_utf8().c_str());

    Log::debug("[OffLoad] Loading OFF file ('" + filepath + "')...");

    std::ifstream file(filepath, std::ios_base::binary);

    if (!file) {
        throw std::invalid_argument("Error: Could not open the OFF file '" + filepath + '\'');
    }

    Mesh mesh;
    Submesh& submesh = mesh.add_submesh();

    size_t vertex_count{};
    size_t face_count{};

    file.ignore(3);
    file >> vertex_count >> face_count;
    file.ignore(100, '\n');

    std::vector<Vertex>& vertices = submesh.get_vertices();
    vertices.resize(vertex_count * 3);

    std::vector<uint>& indices = submesh.get_triangle_indices();
    indices.reserve(face_count * 3);

    for (size_t vertex_index = 0; vertex_index < vertex_count; ++vertex_index) {
        file >> vertices[vertex_index].position.x >> vertices[vertex_index].position.y >>
            vertices[vertex_index].position.z;
    }

    for (size_t face_index = 0; face_index < face_count; ++face_index) {
        uint16_t part_count{};
        file >> part_count;

        std::vector<size_t> parts_indices(part_count);
        file >> parts_indices[0] >> parts_indices[1] >> parts_indices[2];

        indices.emplace_back(static_cast<uint>(parts_indices[0]));
        indices.emplace_back(static_cast<uint>(parts_indices[1]));
        indices.emplace_back(static_cast<uint>(parts_indices[2]));

        for (uint16_t part_index = 3; part_index < part_count; ++part_index) {
            file >> parts_indices[part_index];

            indices.emplace_back(static_cast<uint>(parts_indices[0]));
            indices.emplace_back(static_cast<uint>(parts_indices[part_index - 1]));
            indices.emplace_back(static_cast<uint>(parts_indices[part_index]));
        }
    }

    indices.shrink_to_fit();

    mesh.compute_tangents();

    Log::vdebug(
        "[OffLoad] Loaded OFF file ({} vertices, {} triangles)", mesh.recover_vertex_count(),
        mesh.recover_triangle_count()
    );

    return mesh;
}
}