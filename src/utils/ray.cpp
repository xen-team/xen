#include "ray.hpp"

#include <utils/shape.hpp>

namespace {
constexpr bool solve_quadratic(float a, float b, float c, float& first_hit_distance, float& second_hit_distance)
{
    float const discriminant = b * b - 4.f * a * c;

    if (discriminant < 0.f) {
        return false;
    }
    else if (discriminant > 0.f) {
        float const q = -0.5f * ((b > 0) ? b + std::sqrt(discriminant) : b - std::sqrt(discriminant));

        first_hit_distance = q / a;
        second_hit_distance = c / q;
    }
    else { // discriminant == 0
        float const hit_distance = -0.5f * b / a;

        first_hit_distance = hit_distance;
        second_hit_distance = hit_distance;
    }

    if (first_hit_distance > second_hit_distance) {
        std::swap(first_hit_distance, second_hit_distance);
    }

    return true;
}
}

namespace xen {
bool Ray::intersects(Vector3f const& point, RayHit* hit) const
{
    if (point == origin) {
        if (hit) {
            hit->position = point;
            hit->distance = 0.f;
        }

        return true;
    }

    Vector3f const point_direction = point - origin;
    Vector3f const normalized_point_direction = point_direction.normalize();

    if (!Math::almost_equal(normalized_point_direction.dot(direction), 1.f)) {
        return false;
    }

    if (hit) {
        hit->position = point;
        hit->normal = -normalized_point_direction;
        hit->distance = point_direction.length();
    }

    return true;
}

bool Ray::intersects(Plane const& plane, RayHit* hit) const
{
    float const direction_angle = direction.dot(plane.get_normal());

    if (direction_angle >= 0.f) { // The plane is facing in the same direction as the ray
        return false;
    }

    float const origin_angle = origin.dot(plane.get_normal());
    float const hit_distance = (plane.get_distance() - origin_angle) / direction_angle;

    if (hit_distance <= 0.f) { // The plane is behind the ray
        return false;
    }

    if (hit) {
        hit->position = origin + direction * hit_distance;
        hit->normal = plane.get_normal();
        hit->distance = hit_distance;
    }

    return true;
}

bool Ray::intersects(Sphere const& sphere, RayHit* hit) const
{
    Vector3f const sphere_direction = origin - sphere.get_center();

    float const ray_squared_length = direction.dot(direction);
    float const ray_diff = 2.f * direction.dot(sphere_direction);
    float const sphere_diff = sphere_direction.length_squared() - sphere.get_radius() * sphere.get_radius();

    float first_hit_distance{};
    float second_hit_distance{};

    if (!solve_quadratic(ray_squared_length, ray_diff, sphere_diff, first_hit_distance, second_hit_distance)) {
        return false;
    }

    // If the hit distances are negative, we've hit a sphere located behind the ray's origin
    if (first_hit_distance < 0.f) {
        first_hit_distance = second_hit_distance;

        if (first_hit_distance < 0.f) {
            return false;
        }
    }

    if (hit) {
        Vector3f const hit_pos = origin + direction * first_hit_distance;

        hit->position = hit_pos;
        hit->normal = (hit_pos - sphere.get_center()).normalize();
        hit->distance = first_hit_distance;
    }

    return true;
}

bool Ray::intersects(Triangle const& triangle, RayHit* hit) const
{
    Vector3f const first_edge = triangle.get_second_pos() - triangle.get_first_pos();
    Vector3f const second_edge = triangle.get_third_pos() - triangle.get_first_pos();
    Vector3f const p_vec = direction.cross(second_edge);
    float const determinant = first_edge.dot(p_vec);

    if (Math::almost_equal(std::abs(determinant), 0.f)) {
        return false;
    }

    float const inverse_determinant = 1.f / determinant;

    Vector3f const inverse_plane_direction = origin - triangle.get_first_pos();
    float const first_bary_coord = inverse_plane_direction.dot(p_vec) * inverse_determinant;

    if (first_bary_coord < 0.f || first_bary_coord > 1.f) {
        return false;
    }

    Vector3f const q_vec = inverse_plane_direction.cross(first_edge);
    float const second_bary_coord = q_vec.dot(direction) * inverse_determinant;

    if (second_bary_coord < 0.f || first_bary_coord + second_bary_coord > 1.f) {
        return false;
    }

    float const hit_distance = second_edge.dot(q_vec) * inverse_determinant;

    if (hit_distance <= 0.f) {
        return false;
    }

    if (hit) {
        hit->position = origin + direction * hit_distance;
        hit->normal = first_edge.cross(second_edge)
                          .normalize(); // Directly computing the normal using the already calculated triangle's edges
        hit->distance = hit_distance;
    }

    return true;
}

bool Ray::intersects(const AABB& aabb, RayHit* hit) const
{
    // Branchless algorithm based on Tavianator's:
    //  - https://tavianator.com/fast-branchless-raybounding-box-intersections/
    //  - https://tavianator.com/cgit/dimension.git/tree/libdimension/bvh/bvh.c#n196

    Vector3f const min_dist = (aabb.get_min_position() - origin) * inverse_direction;
    Vector3f const max_dist = (aabb.get_max_position() - origin) * inverse_direction;

    float const min_dist_x = std::min(min_dist.x, max_dist.x);
    float const max_dist_x = std::max(min_dist.x, max_dist.x);

    float const min_dist_y = std::min(min_dist.y, max_dist.y);
    float const max_dist_y = std::max(min_dist.y, max_dist.y);

    float const min_dist_z = std::min(min_dist.z, max_dist.z);
    float const max_dist_z = std::max(min_dist.z, max_dist.z);

    float const min_hit_dist = std::max(min_dist_x, std::max(min_dist_y, min_dist_z));
    float const max_hit_dist = std::min(max_dist_x, std::min(max_dist_y, max_dist_z));

    if (max_hit_dist < std::max(min_hit_dist, 0.f)) {
        return false;
    }

    // If reaching here with a negative distance (min_hit_dist < 0), this means that the ray's origin is inside the box
    // Currently, in this case, the computed hit position represents the intersection behind the ray

    if (hit) {
        hit->position = origin + direction * min_hit_dist;

        // Normal computing method based on John Novak's:
        // https://blog.johnnovak.net/2016/10/22/the-nim-ray-tracer-project-part-4-calculating-box-normals/
        Vector3f const hit_direction = (hit->position - aabb.compute_centroid()) / aabb.compute_half_extents();
        hit->normal =
            Vector3f(std::trunc(hit_direction.x), std::trunc(hit_direction.y), std::trunc(hit_direction.z)).normalize();

        hit->distance = min_hit_dist;
    }

    return true;
}

Vector3f Ray::compute_projection(Vector3f const& point) const
{
    float const point_dist = direction.dot(point - origin);
    return (origin + direction * std::max(point_dist, 0.f));
}
}