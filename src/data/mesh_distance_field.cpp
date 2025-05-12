#include "mesh_distance_field.hpp"

#include <data/bvh.hpp>
#include <data/image.hpp>
#include <utils/ray.hpp>
#include <utils/threading.hpp>

#include <tracy/Tracy.hpp>

namespace xen {
MeshDistanceField::MeshDistanceField(AABB const& area, Vector3ui const& size) :
    area{area}, size{size}, distance_field(size.length(), std::numeric_limits<float>::max())
{
    if (size.x < 2 || size.y < 2 || size.z < 2) {
        throw std::invalid_argument("[MeshDistanceField] The size & depth must all be equal to or greater than 2.");
    }
}

float MeshDistanceField::get_distance(Vector3ui const& size) const
{
    return distance_field[compute_index(size)];
}

void MeshDistanceField::compute(size_t sample_count)
{
    ZoneScopedN("MeshDistanceField::compute");

    if (bvh == nullptr) {
        throw std::runtime_error("[MeshDistanceField] Computing a mesh distance field requires having given a BVH.");
    }

    std::ranges::fill(distance_field, std::numeric_limits<float>::max());

    Vector3f const area_extents = area.get_max_position() - area.get_min_position();
    Vector3f const step_size = area_extents / static_cast<Vector3f>(size - 1);

    parallelize(
        0, size.z,
        [this, &step_size, sample_count](IndexRange const& range) {
            ZoneScopedN("MeshDistanceField::compute");

            for (size_t depth_index = range.begin_index; depth_index < range.end_index; ++depth_index) {
                for (size_t height_index = 0; height_index < size.y; ++height_index) {
                    for (size_t width_index = 0; width_index < size.x; ++width_index) {
                        Vector3f const ray_pos =
                            area.get_min_position() + Vector3f(
                                                          static_cast<float>(width_index) * step_size.x,
                                                          static_cast<float>(height_index) * step_size.y,
                                                          static_cast<float>(depth_index) * step_size.z
                                                      );
                        float& distance =
                            distance_field[compute_index(Vector3ui(width_index, height_index, depth_index))];

                        for (auto const& ray_direction_arr : Math::compute_fibonacci_sphere_points(sample_count)) {
                            Vector3f ray_direction(ray_direction_arr[0], ray_direction_arr[1], ray_direction_arr[2]);
                            RayHit hit{};

                            if (!bvh->query(Ray(ray_pos, ray_direction), &hit)) {
                                continue;
                            }

                            if (ray_direction.dot(hit.normal) > 0.f) {
                                hit.distance = -hit.distance;
                            }

                            if (std::abs(hit.distance) < std::abs(distance)) {
                                distance = hit.distance;
                            }
                        }
                    }
                }
            }
        },
        get_system_thread_count() * 2
    );
}

std::vector<Image> MeshDistanceField::recover_slices() const
{
    ZoneScopedN("MeshDistanceField::recover_slices");

    std::vector<Image> slices;
    slices.reserve(size.z);

    for (size_t depth_index = 0; depth_index < size.z; ++depth_index) {
        Image& slice = slices.emplace_back(size, ImageColorspace::GRAY, ImageDataType::FLOAT);

        for (size_t height_index = 0; height_index < size.y; ++height_index) {
            for (size_t width_index = 0; width_index < size.x; ++width_index) {
                float const distance = get_distance(Vector3ui(width_index, height_index, depth_index));
                slice.set_pixel(width_index, height_index, distance);
            }
        }
    }

    return slices;
}

}
