#include "shape.hpp"

namespace xen {

// Line functions

bool Line::intersects(Line const&) const
{
    throw std::runtime_error("Error: Not implemented yet.");
}

bool Line::intersects(Plane const& plane) const
{
    Vector3f const line_vec = end_pos - begin_pos;
    float const line_vec_plane_angle = line_vec.dot(plane.get_normal());

    // If near 0, the line & the plane are parallel to each other
    if (Math::almost_equal(line_vec_plane_angle, 0.f)) {
        return false;
    }

    float const line_start_plane_angle = begin_pos.dot(plane.get_normal());

    // Calculating the relative distance along the line where it is intersected by the plane
    // If this distance is below 0 or above 1, the intersection isn't between the line's two extremities
    float const intersect_dist = (plane.get_distance() - line_start_plane_angle) / line_vec_plane_angle;
    return ((intersect_dist >= 0.f) && (intersect_dist <= 1.f));
}

bool Line::intersects(Sphere const& sphere) const
{
    Vector3f const projection_point = compute_projection(sphere.get_center());
    return sphere.contains(projection_point);
}

bool Line::intersects(Triangle const&) const
{
    throw std::runtime_error("Error: Not implemented yet.");
}

bool Line::intersects(Quad const&) const
{
    throw std::runtime_error("Error: Not implemented yet.");
}

bool Line::intersects(const AABB& aabb) const
{
    Ray const line_ray(begin_pos, (end_pos - begin_pos).normalize());
    RayHit hit;

    if (!line_ray.intersects(aabb, &hit)) {
        return false;
    }

    // Some implementations check for the hit distance to be positive or 0. However, since our ray-AABB intersection
    // check returns true
    //  with a negative distance when the ray's origin is inside the box, this check would be meaningless
    // Actually, if reaching here, none of the potential cases should require to check that the hit distance is
    // non-negative

    // In certain cases, it's even harmful to do so. Given a line segment defined by points A & B, one being in a box &
    // the other outside:
    //
    // ----------
    // |        |
    // |   A x-----x B
    // |        |
    // ----------
    //
    // Depending on the order of the points, the result would not be symmetrical: B->A would return a positive distance,
    // telling there's an
    //  intersection, and A->B a negative distance, telling there's none

    return (hit.distance * hit.distance <= length_squared());
}

bool Line::intersects(const OBB&) const
{
    throw std::runtime_error("Error: Not implemented yet.");
}

void Line::translate(Vector3f const& translation)
{
    begin_pos += translation;
    end_pos += translation;
}

Vector3f Line::compute_projection(Vector3f const& point) const
{
    Vector3f const line_vec = end_pos - begin_pos;
    float const point_dist = line_vec.dot(point - begin_pos) / line_vec.length_squared();

    // Clamping point_dist between 0 & 1, since it can be outside these bounds if not directly projectable
    //
    //        < 0        |    >= 0 & <= 1    |        > 1
    // __________________________________________________________
    //                   |                   |
    // P                 |         P         |                  P
    // |                 |         |         |                  |
    // v                 |         v         |                  v
    //    A----------B   |   A----------B    |   A----------B

    return begin_pos + line_vec * std::clamp(point_dist, 0.f, 1.f);
}

AABB Line::compute_bounding_box() const
{
    auto const [x_min, x_max] = std::minmax(begin_pos.x, end_pos.x);
    auto const [y_min, y_max] = std::minmax(begin_pos.y, end_pos.y);
    auto const [z_min, z_max] = std::minmax(begin_pos.z, end_pos.z);

    return AABB(Vector3f(x_min, y_min, z_min), Vector3f(x_max, y_max, z_max));
}

// Plane functions

bool Plane::intersects(Plane const& plane) const
{
    float const planes_angle = normal.dot(plane.get_normal());
    return !Math::almost_equal(std::abs(planes_angle), 1.f);
}

bool Plane::intersects(Sphere const& sphere) const
{
    Vector3f const projection_point = compute_projection(sphere.get_center());
    return sphere.contains(projection_point);
}

bool Plane::intersects(Triangle const&) const
{
    throw std::runtime_error("Error: Not implemented yet.");
}

bool Plane::intersects(Quad const&) const
{
    throw std::runtime_error("Error: Not implemented yet.");
}

bool Plane::intersects(const AABB& aabb) const
{
    Vector3f const half_extents = aabb.compute_half_extents();

    float const top_box_dist =
        half_extents.x * std::abs(normal.x) + half_extents.y * std::abs(normal.y) + half_extents.z * std::abs(normal.z);
    float const box_dist = normal.dot(aabb.compute_centroid()) - distance;

    return (std::abs(box_dist) <= top_box_dist);
}

bool Plane::intersects(const OBB&) const
{
    throw std::runtime_error("Error: Not implemented yet.");
}

AABB Plane::compute_bounding_box() const
{
    throw std::runtime_error("Error: Not implemented yet.");
}

// Sphere functions

bool Sphere::contains(Vector3f const& point) const
{
    float const point_squared_dist = (center_pos - point).length_squared();
    return (point_squared_dist <= (radius * radius));
}

bool Sphere::intersects(Sphere const& sphere) const
{
    float const squared_dist = (center_pos - sphere.get_center()).length_squared();
    float const radius_sum = radius + sphere.get_radius();

    return (squared_dist <= radius_sum * radius_sum);
}

bool Sphere::intersects(Triangle const& triangle) const
{
    Vector3f const projection_point = triangle.compute_projection(center_pos);
    return contains(projection_point);
}

bool Sphere::intersects(Quad const& quad) const
{
    Vector3f const projection_point = quad.compute_projection(center_pos);
    return contains(projection_point);
}

bool Sphere::intersects(const AABB& aabb) const
{
    Vector3f const projection_point = aabb.compute_projection(center_pos);
    return contains(projection_point);
}

bool Sphere::intersects(const OBB&) const
{
    throw std::runtime_error("Error: Not implemented yet.");
}

AABB Sphere::compute_bounding_box() const
{
    return AABB(center_pos - radius, center_pos + radius);
}

// Triangle functions

bool Triangle::intersects(Triangle const&) const
{
    throw std::runtime_error("Error: Not implemented yet.");
}

bool Triangle::intersects(Quad const&) const
{
    throw std::runtime_error("Error: Not implemented yet.");
}

bool Triangle::intersects(const AABB&) const
{
    throw std::runtime_error("Error: Not implemented yet.");
}

bool Triangle::intersects(const OBB&) const
{
    throw std::runtime_error("Error: Not implemented yet.");
}

void Triangle::translate(Vector3f const& translation)
{
    first_pos += translation;
    second_pos += translation;
    third_pos += translation;
}

Vector3f Triangle::compute_projection(Vector3f const&) const
{
    throw std::runtime_error("Error: Not implemented yet.");
}

AABB Triangle::compute_bounding_box() const
{
    auto const [x_min, x_max] = std::minmax({first_pos.x, second_pos.x, third_pos.x});
    auto const [y_min, y_max] = std::minmax({first_pos.y, second_pos.y, third_pos.y});
    auto const [z_min, z_max] = std::minmax({first_pos.z, second_pos.z, third_pos.z});

    return AABB(Vector3f(x_min, y_min, z_min), Vector3f(x_max, y_max, z_max));
}

Vector3f Triangle::compute_normal() const
{
    Vector3f const first_edge = second_pos - first_pos;
    Vector3f const second_edge = third_pos - first_pos;

    return first_edge.cross(second_edge).normalize();
}

void Triangle::make_counter_clockwise(Vector3f const& normal)
{
    if (is_counter_clockwise(normal)) {
        return;
    }

    // It doesn't matter which ones are swapped, as long as two of them are
    // The 3 points being adjacent, the ordering will be reversed all the same
    std::swap(first_pos, second_pos);
}

// Quad functions

bool Quad::intersects(Quad const&) const
{
    throw std::runtime_error("Error: Not implemented yet.");
}

bool Quad::intersects(const AABB&) const
{
    throw std::runtime_error("Error: Not implemented yet.");
}

bool Quad::intersects(const OBB&) const
{
    throw std::runtime_error("Error: Not implemented yet.");
}

void Quad::translate(Vector3f const& translation)
{
    top_left_pos += translation;
    top_right_pos += translation;
    bottom_right_pos += translation;
    bottom_left_pos += translation;
}

Vector3f Quad::compute_projection(Vector3f const&) const
{
    throw std::runtime_error("Error: Not implemented yet.");
}

AABB Quad::compute_bounding_box() const
{
    auto const [x_min, x_max] = std::minmax({top_left_pos.x, top_right_pos.x, bottom_right_pos.x, bottom_left_pos.x});
    auto const [y_min, y_max] = std::minmax({top_left_pos.y, top_right_pos.y, bottom_right_pos.y, bottom_left_pos.y});
    auto const [z_min, z_max] = std::minmax({top_left_pos.z, top_right_pos.z, bottom_right_pos.z, bottom_left_pos.z});

    return AABB(Vector3f(x_min, y_min, z_min), Vector3f(x_max, y_max, z_max));
}

// AABB functions

bool AABB::contains(Vector3f const& point) const
{
    bool const is_in_bounds_x = point.x >= min_pos.x && point.x <= max_pos.x;
    bool const is_in_bounds_y = point.y >= min_pos.y && point.y <= max_pos.y;
    bool const is_in_bounds_z = point.z >= min_pos.z && point.z <= max_pos.z;

    return (is_in_bounds_x && is_in_bounds_y && is_in_bounds_z);
}

bool AABB::intersects(const AABB& aabb) const
{
    Vector3f const& min_point1 = min_pos;
    Vector3f const& max_point1 = max_pos;

    Vector3f const& min_point2 = aabb.get_min_position();
    Vector3f const& max_point2 = aabb.get_max_position();

    // We determine for each axis if there are extremities that are overlapping
    // If the max point of one AABB is further on an axis than the min point of the other, they intersect each other on
    // this axis
    //
    //            max1
    //             v
    //    ----------
    //    |        |
    //    |     ----------
    //    |     |  |     |
    //    ------|---     |
    //          |        |
    //          ----------
    //          ^
    //        min2

    bool const intersects_x = (min_point1.x <= max_point2.x) && (max_point1.x >= min_point2.x);
    bool const intersects_y = (min_point1.y <= max_point2.y) && (max_point1.y >= min_point2.y);
    bool const intersects_z = (min_point1.z <= max_point2.z) && (max_point1.z >= min_point2.z);

    return (intersects_x && intersects_y && intersects_z);
}

bool AABB::intersects(const OBB&) const
{
    throw std::runtime_error("Error: Not implemented yet.");
}

void AABB::translate(Vector3f const& translation)
{
    min_pos += translation;
    max_pos += translation;
}

Vector3f AABB::compute_projection(Vector3f const& point) const
{
    float const closest_x = std::max(std::min(point.x, max_pos.x), min_pos.x);
    float const closest_y = std::max(std::min(point.y, max_pos.y), min_pos.y);
    float const closest_z = std::max(std::min(point.z, max_pos.z), min_pos.z);

    return Vector3f(closest_x, closest_y, closest_z);
}

// OBB functions

void OBB::set_rotation(Quaternion const& rotation)
{
    this->rotation = rotation;
    inv_rotation = this->rotation.inverse();
}

bool OBB::contains(Vector3f const&) const
{
    throw std::runtime_error("Error: Not implemented yet.");
}

bool OBB::intersects(const OBB&) const
{
    throw std::runtime_error("Error: Not implemented yet.");
}

Vector3f OBB::compute_projection(Vector3f const&) const
{
    throw std::runtime_error("Error: Not implemented yet.");
}

AABB OBB::compute_bounding_box() const
{
    throw std::runtime_error("Error: Not implemented yet.");
}
}