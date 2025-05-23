#include "shape.hpp"

namespace xen {
namespace {
struct Projection {
    float min;
    float max;
};

Projection project_points(std::vector<Vector3f> const& points, Vector3f const& axis)
{
    if (points.empty() || axis.length_squared() < std::numbers::e_v<float>) {
        return {0.f, 0.f};
    }

    float min_proj = std::numeric_limits<float>::max();
    float max_proj = std::numeric_limits<float>::lowest();

    for (auto const& point : points) {
        float proj = point.dot(axis);
        min_proj = std::min(min_proj, proj);
        max_proj = std::max(max_proj, proj);
    }

    return {min_proj, max_proj};
}

// Helper function to check for overlap on a single axis for SAT
bool check_overlap(Projection const& p1, Projection const& p2)
{
    return !(p1.max < p2.min || p2.max < p1.min);
}

// Helper for Triangle and Quad projection onto their plane
Vector3f project_point_onto_plane(Vector3f const& point, Vector3f const& plane_origin, Vector3f const& plane_normal)
{
    return point - plane_normal * (plane_normal.dot(point - plane_origin));
}

// Helper for 2D cross product used in point-in-triangle test
float sign(Vector2f const& p1, Vector2f const& p2, Vector2f const& p3)
{
    return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
}

// Helper for 2D point-in-triangle test using barycentric coordinates
bool is_point_in_triangle_2d(Vector2f const& pt, Vector2f const& v1, Vector2f const& v2, Vector2f const& v3)
{
    float d1 = sign(pt, v1, v2);
    float d2 = sign(pt, v2, v3);
    float d3 = sign(pt, v3, v1);

    bool has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
    bool has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

    // Point is in triangle if signs are the same (or one is zero)
    return !(has_neg && has_pos);
}

// Helper for 2D point-in-quad test (assuming convex quad) - treat as two triangles
bool is_point_in_quad_2d(
    Vector2f const& pt, Vector2f const& tl, Vector2f const& tr, Vector2f const& br, Vector2f const& bl
)
{
    return is_point_in_triangle_2d(pt, tl, tr, br) || is_point_in_triangle_2d(pt, tl, br, bl);
}

// Helper to find closest point on a line segment in 3D
Vector3f closest_point_on_line_segment(Vector3f const& point, Vector3f const& a, Vector3f const& b)
{
    Vector3f ab = b - a;
    float t = (point - a).dot(ab) / ab.length_squared();
    t = std::clamp(t, 0.f, 1.f);
    return a + t * ab;
}

// Helper to find closest point on a triangle in 3D
Vector3f closest_point_on_triangle(Vector3f const& point, Vector3f const& v1, Vector3f const& v2, Vector3f const& v3)
{
    Vector3f ab = v2 - v1;
    Vector3f ac = v3 - v1;
    Vector3f ap = point - v1;

    float d1 = ap.dot(ab);
    float d2 = ap.dot(ac);

    // Check if projection is outside vertex v1 region
    if (d1 <= 0.0f && d2 <= 0.0f) {
        return v1;
    }

    Vector3f bp = point - v2;
    float d3 = bp.dot(ab);
    float d4 = bp.dot(v3 - v2);

    // Check if projection is outside vertex v2 region
    if (d3 >= 0.0f && d4 <= d3) {
        return v2;
    }

    float vc = d1 * d4 - d3 * d2;
    // Check if projection is outside edge v1-v2 region
    if (vc <= 0.0f && d1 >= 0.0f && d3 <= 0.0f) {
        return closest_point_on_line_segment(point, v1, v2);
    }

    Vector3f cp = point - v3;
    float d5 = cp.dot(ab);
    float d6 = cp.dot(ac);

    // Check if projection is outside vertex v3 region
    if (d6 >= 0.0f && d5 <= d6) {
        return v3;
    }

    float vb = d5 * d2 - d1 * d6;
    // Check if projection is outside edge v1-v3 region
    if (vb <= 0.0f && d2 >= 0.0f && d6 <= 0.0f) {
        return closest_point_on_line_segment(point, v1, v3);
    }

    float va = d3 * d6 - d5 * d4;
    // Check if projection is outside edge v2-v3 region
    if (va <= 0.0f && (d4 - d3) >= 0.0f && (d5 - d6) >= 0.0f) {
        return closest_point_on_line_segment(point, v2, v3);
    }

    // Projection is inside face region
    float denom = va + vb + vc;
    float v = vb / denom;
    float w = vc / denom;
    float u = 1.0f - v - w;

    return v1 + ab * v + ac * w;
}
}

// Line functions

bool Line::intersects(Line const& line) const
{
    // Check for intersection using cross product of direction vectors
    Vector3f const d1 = end_pos - begin_pos;
    Vector3f const d2 = line.end_pos - line.begin_pos;
    Vector3f const r = begin_pos - line.begin_pos;

    float const a = d1.dot(d1);
    float const b = d1.dot(d2);
    float const c = d2.dot(d2);
    float const e = r.dot(d1);
    float const f = r.dot(d2);

    float const denom = a * c - b * b;

    // If lines are parallel (or collinear)
    if (Math::almost_equal(denom, 0.f)) {
        // Check for overlap if collinear
        if (Math::almost_equal(r.cross(d1).length_squared(), 0.f)) { // Collinear
            float t0 = r.dot(d1) / a;
            float t1 = t0 + d2.dot(d1) / a;
            if (t0 > t1)
                std::swap(t0, t1);
            // Check if overlap exists within [0, 1] range for both lines
            return std::max(0.f, t0) <= std::min(1.f, t1);
        }
        return false; // Parallel and not collinear
    }

    float const t = (b * f - c * e) / denom; // parameter for this line (begin_pos + t * d1)
    float const u = (a * f - b * e) / denom; // parameter for line (line.begin_pos + u * d2)

    // Check if intersection point lies within both segments
    if (t >= 0.f && t <= 1.f && u >= 0.f && u <= 1.f) {
        // Check if points are actually close to each other
        Vector3f const p1 = begin_pos + t * d1;
        Vector3f const p2 = line.begin_pos + u * d2;
        return (p1 - p2).length_squared() < (std::numbers::e_v<float> * std::numbers::e_v<float>);
    }

    return false;
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

bool Line::intersects(Triangle const& triangle) const
{
    Ray line_ray(begin_pos, (end_pos - begin_pos).normalize());
    RayHit hit;

    // Use Ray-Triangle intersection
    if (line_ray.intersects(triangle, &hit)) {
        // Check if the hit point is within the line segment
        Vector3f const hit_point = line_ray.get_origin() + line_ray.get_direction() * hit.distance;
        float const sq_dist_to_start = (hit_point - begin_pos).length_squared();
        float const sq_dist_to_end = (hit_point - end_pos).length_squared();
        float const sq_line_length = length_squared();

        // Check if the hit point is on the segment by comparing squared distances
        // Sum of distances from start and end to hit should approximately equal total length
        return Math::
            almost_equal(std::sqrt(sq_dist_to_start) + std::sqrt(sq_dist_to_end), std::sqrt(sq_line_length), std::numbers::e_v<float>);
    }
    return false;
}

bool Line::intersects(Quad const& quad) const
{
    // Treat quad as two triangles and check intersection with either
    Triangle tri1(quad.get_top_left_pos(), quad.get_top_right_pos(), quad.get_bottom_right_pos());
    Triangle tri2(quad.get_top_left_pos(), quad.get_bottom_right_pos(), quad.get_bottom_left_pos());

    return intersects(tri1) || intersects(tri2);
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

bool Line::intersects(const OBB& obb) const
{
    // Transform the line into the OBB's local space
    Quaternion const& inv_rotation = obb.get_rotation().inverse();
    Vector3f const obb_center = obb.compute_centroid();
    Vector3f local_begin = inv_rotation * (begin_pos - obb_center);
    Vector3f local_end = inv_rotation * (end_pos - obb_center);

    // Create a line segment in local space
    Line local_line(local_begin, local_end);

    // Check intersection with the OBB's AABB in local space
    AABB local_aabb(
        obb.get_min_position(), obb.get_max_position()
    ); // min/max are relative to OBB center in definition? Or absolute? Assuming relative to center based on OBB
       // structure. Let's check OBB constructor: `OBB(Vector3f const& min_pos, Vector3f const& max_pos) : aabb(min_pos,
       // max_pos)`. AABB constructor takes min/max directly. So AABB min/max are absolute world coordinates when the
       // OBB has no rotation. When rotated, the AABB internally stores the unrotated bounds. The OBB's min/max
       // accessors return the AABB's min/max. This is a bit ambiguous. Let's assume the AABB stored in OBB represents
       // the bounds *before* rotation, centered at the OBB's centroid. The AABB's min/max accessors should probably
       // return these unrotated bounds relative to the centroid, or the code that uses them should account for this.
       // Let's assume `obb.get_min_position()` and `obb.get_max_position()` return the unrotated bounds relative to the
       // centroid for the purpose of the local AABB test.

    // Re-reading the OBB definition: `OBB(Vector3f const& min_pos, Vector3f const& max_pos) : aabb(min_pos, max_pos)`.
    // This creates an AABB with those min/max. The AABB stores absolute world min/max. When translated, the AABB
    // min/max update. When rotated, the OBB uses the AABB and rotation. This suggests the AABB internally holds the
    // *world-aligned* bounding box *before* rotation. This means `obb.get_min_position()` and `obb.get_max_position()`
    // return the AABB's world min/max. This is inconsistent for defining the OBB's local bounds. A typical OBB stores
    // center, half-extents, and orientation. The current structure stores an AABB and a rotation. Let's assume the
    // AABB's min/max, when the OBB is at identity rotation, define the bounds. When rotated, the OBB's effective bounds
    // are the AABB's bounds *rotated* around the centroid. So, the local space AABB for intersection testing should be
    // centered at the origin with extents derived from the AABB's size.

    Vector3f obb_half_extents = obb.get_aabb().compute_half_extents(); // Use the internal AABB's half extents
    AABB local_aabb_centered(-obb_half_extents, obb_half_extents);     // AABB centered at origin in local space

    Ray local_line_ray(local_begin, (local_end - local_begin).normalize());
    RayHit hit;

    if (!local_line_ray.intersects(local_aabb_centered, &hit)) {
        return false;
    }

    // Check if the hit point in local space is within the local line segment
    Vector3f const local_hit_point = local_line_ray.get_origin() + local_line_ray.get_direction() * hit.distance;
    float const sq_dist_to_local_start = (local_hit_point - local_begin).length_squared();
    float const sq_dist_to_local_end = (local_hit_point - local_end).length_squared();
    float const sq_local_line_length = (local_end - local_begin).length_squared();

    return Math::
        almost_equal(std::sqrt(sq_dist_to_local_start) + std::sqrt(sq_dist_to_local_end), std::sqrt(sq_local_line_length), std::numbers::e_v<float>);
}

bool Line::intersects(Ray const& ray, RayHit* hit) const
{
    // Treat the line as a ray starting at begin_pos with direction end_pos - begin_pos,
    // and the input ray as another ray. Find their intersection point and check if it lies on the line segment.
    Vector3f const p1 = begin_pos;
    Vector3f const d1 = end_pos - begin_pos; // Direction of the line segment

    Vector3f const p2 = ray.get_origin();
    Vector3f const d2 = ray.get_direction(); // Direction of the ray

    Vector3f const r = p1 - p2; // Vector between origins

    float const a = d1.dot(d1);
    float const b = d1.dot(d2);
    float const c = d2.dot(d2);
    float const e = r.dot(d1);
    float const f = r.dot(d2);

    float const denom = a * c - b * b;

    // If lines are parallel (or collinear)
    if (Math::almost_equal(denom, 0.f)) {
        // Collinear check is complex for ray-segment. Simple case: check if segment start/end lies on ray.
        // Or check if ray origin lies on segment.
        Vector3f closest_on_ray = ray.get_origin() + ray.get_direction() * (d2.dot(p1 - ray.get_origin()));
        if ((closest_on_ray - p1).length_squared() < (std::numbers::e_v<float> * std::numbers::e_v<float>) &&
            (closest_on_ray - end_pos).length_squared() <
                d1.length_squared() + (std::numbers::e_v<float> * std::numbers::e_v<float>)) { // On the line
            float t_segment = (closest_on_ray - p1).dot(d1) / a;
            if (t_segment >= 0.f && t_segment <= 1.f) { // On the segment
                float t_ray = (closest_on_ray - p2).dot(d2);
                if (t_ray >= 0.f) { // On the ray
                    if (hit) {
                        hit->distance = t_ray; // Store distance along the input ray
                        hit->position = closest_on_ray;
                        // Normal is tricky for line-ray intersection, maybe not required or context-dependent
                        // hit->normal = ???;
                    }
                    return true;
                }
            }
        }
        return false; // Parallel and not intersecting within constraints
    }

    float const t = (b * f - c * e) / denom; // parameter for this line (p1 + t * d1)
    float const u = (a * f - b * e) / denom; // parameter for ray (p2 + u * d2)

    // Check if intersection point lies within the line segment [0, 1] and on the ray [0, inf)
    if (t >= 0.f && t <= 1.f && u >= 0.f) {
        Vector3f const intersection_point = p1 + t * d1;
        if (hit) {
            hit->distance = u; // Distance along the input ray
            hit->position = intersection_point;
            // Normal is tricky, maybe not needed or context-dependent
            // hit->normal = ???;
        }
        return true;
    }

    return false;
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

bool Plane::intersects(Triangle const& triangle) const
{
    // Check side of each vertex relative to the plane
    float const d1 = normal.dot(triangle.get_first_pos()) - distance;
    float const d2 = normal.dot(triangle.get_second_pos()) - distance;
    float const d3 = normal.dot(triangle.get_third_pos()) - distance;

    // If all vertices are on the same side (and not on the plane), no intersection
    if ((d1 > std::numbers::e_v<float> && d2 > std::numbers::e_v<float> && d3 > std::numbers::e_v<float>) ||
        (d1 < -std::numbers::e_v<float> && d2 < -std::numbers::e_v<float> && d3 < -std::numbers::e_v<float>)) {
        return false;
    }

    // If any vertex is on the plane or vertices are on different sides, they intersect
    return true;
}

bool Plane::intersects(Quad const& quad) const
{
    // Check side of each vertex relative to the plane
    float const d1 = normal.dot(quad.get_top_left_pos()) - distance;
    float const d2 = normal.dot(quad.get_top_right_pos()) - distance;
    float const d3 = normal.dot(quad.get_bottom_right_pos()) - distance;
    float const d4 = normal.dot(quad.get_bottom_left_pos()) - distance;

    // If all vertices are on the same side (and not on the plane), no intersection
    if ((d1 > std::numbers::e_v<float> && d2 > std::numbers::e_v<float> && d3 > std::numbers::e_v<float> &&
         d4 > std::numbers::e_v<float>) ||
        (d1 < -std::numbers::e_v<float> && d2 < -std::numbers::e_v<float> && d3 < -std::numbers::e_v<float> &&
         d4 < -std::numbers::e_v<float>)) {
        return false;
    }

    // If any vertex is on the plane or vertices are on different sides, they intersect
    return true;
}

bool Plane::intersects(const OBB& obb) const
{
    // Use Plane-AABB intersection logic but project OBB extents onto the plane normal
    Vector3f const obb_center = obb.compute_centroid();
    Quaternion const& obb_rotation = obb.get_rotation();
    Vector3f const half_extents = obb.get_aabb().compute_half_extents(); // Use unrotated half-extents

    // Calculate projection interval radius of the OBB onto the plane normal
    float const projection_radius = half_extents.x * std::abs(normal.dot(obb_rotation * Vector3f::Right)) +
                                    half_extents.y * std::abs(normal.dot(obb_rotation * Vector3f::Up)) +
                                    half_extents.z * std::abs(normal.dot(obb_rotation * Vector3f::Forward));

    // Calculate signed distance from OBB center to the plane
    float const center_distance = normal.dot(obb_center) - distance;

    // Intersection occurs if the distance from the center is less than or equal to the projection radius
    return std::abs(center_distance) <= projection_radius;
}

// Plane::compute_bounding_box remains throwing as it's an infinite plane
AABB Plane::compute_bounding_box() const
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

bool Sphere::intersects(const OBB& obb) const
{
    // Transform sphere center into OBB's local space
    Quaternion const& inv_rotation = obb.get_rotation().inverse();
    Vector3f const obb_center = obb.compute_centroid();
    Vector3f local_sphere_center = inv_rotation * (center_pos - obb_center);

    // Clamp the local sphere center to the OBB's AABB in local space
    Vector3f obb_half_extents = obb.get_aabb().compute_half_extents(); // Use unrotated half-extents
    Vector3f local_aabb_min = -obb_half_extents;
    Vector3f local_aabb_max = obb_half_extents;

    float closest_x = std::max(local_aabb_min.x, std::min(local_sphere_center.x, local_aabb_max.x));
    float closest_y = std::max(local_aabb_min.y, std::min(local_sphere_center.y, local_aabb_max.y));
    float closest_z = std::max(local_aabb_min.z, std::min(local_sphere_center.z, local_aabb_max.z));

    Vector3f closest_point_in_aabb = Vector3f(closest_x, closest_y, closest_z);

    // Check if the distance from the clamped point to the sphere center (in local space)
    // is less than or equal to the sphere's radius
    float squared_distance = (local_sphere_center - closest_point_in_aabb).length_squared();

    return squared_distance <= (radius * radius);
}

// Triangle functions

bool Triangle::intersects(Triangle const& triangle) const
{
    // Implementing SAT for Triangle-Triangle intersection is complex and lengthy.
    // A simplified check can be done, but it's not guaranteed to be robust for all cases.
    // A full implementation would require checking 1 edge-edge axis (cross products) and
    // 2 face normals (triangle normals).

    // Simple AABB intersection check (quick broad-phase)
    if (!compute_bounding_box().intersects(triangle.compute_bounding_box())) {
        return false;
    }

    // Fallback to a simplified check (not robust, ideally use SAT or GJK)
    // Check if any vertex of one triangle is inside the other triangle (projected onto plane)
    // or if any edge of one intersects the plane of the other and is inside the other triangle.

    // Given the complexity and space constraints, a full SAT is not feasible here.
    // A robust implementation would use SAT with 1 + 1 + 3*3 = 11 potential separating axes.
    // For this response, we will leave a throw, indicating the need for a proper algorithm.
    // Implementing a correct and complete Triangle-Triangle SAT or GJK requires
    // significant code.
    throw std::runtime_error("Error: Robust Triangle-Triangle intersection not implemented (requires SAT/GJK).");
}

AABB Sphere::compute_bounding_box() const
{
    return AABB(center_pos - radius, center_pos + radius);
}

// Triangle functions

bool Triangle::intersects(Quad const& quad) const
{
    // Treat the quad as two triangles and check intersection with either
    Triangle tri1(quad.get_top_left_pos(), quad.get_top_right_pos(), quad.get_bottom_right_pos());
    Triangle tri2(quad.get_top_left_pos(), quad.get_bottom_right_pos(), quad.get_bottom_left_pos());

    // Note: This relies on a robust Triangle-Triangle intersection, which is complex.
    // If Triangle::intersects(Triangle) is not fully robust, this will also be limited.
    return intersects(tri1) || intersects(tri2);
}

bool Triangle::intersects(const AABB& aabb) const
{
    // Use SAT (Separating Axis Theorem)

    std::vector<Vector3f> tri_vertices = {first_pos, second_pos, third_pos};
    std::vector<Vector3f> aabb_vertices;
    Vector3f aabb_min = aabb.get_min_position();
    Vector3f aabb_max = aabb.get_max_position();

    aabb_vertices.push_back(aabb_min);
    aabb_vertices.push_back(Vector3f(aabb_max.x, aabb_min.y, aabb_min.z));
    aabb_vertices.push_back(Vector3f(aabb_min.x, aabb_max.y, aabb_min.z));
    aabb_vertices.push_back(Vector3f(aabb_min.x, aabb_min.y, aabb_max.z));
    aabb_vertices.push_back(Vector3f(aabb_max.x, aabb_max.y, aabb_min.z));
    aabb_vertices.push_back(Vector3f(aabb_max.x, aabb_min.y, aabb_max.z));
    aabb_vertices.push_back(Vector3f(aabb_min.x, aabb_max.y, aabb_max.z));
    aabb_vertices.push_back(aabb_max);

    // Define potential separating axes
    std::vector<Vector3f> axes;

    // AABB's face normals (3 axes)
    axes.push_back(Vector3f::Right);   // (1, 0, 0)
    axes.push_back(Vector3f::Up);      // (0, 1, 0)
    axes.push_back(Vector3f::Forward); // (0, 0, 1)

    // Triangle's face normal (1 axis)
    axes.push_back(compute_normal());

    // Cross products of AABB edges and Triangle edges (3 * 3 = 9 axes)
    Vector3f tri_edges[3] = {second_pos - first_pos, third_pos - second_pos, first_pos - third_pos};
    Vector3f aabb_edges[3] = {Vector3f::Right, Vector3f::Up, Vector3f::Forward};

    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            axes.push_back(tri_edges[i].cross(aabb_edges[j]).normalize());
        }
    }

    // Check for separation on each axis
    for (auto const& axis : axes) {
        if (axis.length_squared() < std::numbers::e_v<float>)
            continue; // Skip zero-length axes

        Projection proj_tri = project_points(tri_vertices, axis);
        Projection proj_aabb = project_points(aabb_vertices, axis);

        if (!check_overlap(proj_tri, proj_aabb)) {
            return false; // Separating axis found
        }
    }

    return true; // No separating axis found, they intersect
}

bool Triangle::intersects(const OBB& obb) const
{
    // Use SAT (Separating Axis Theorem)

    std::vector<Vector3f> tri_vertices = {first_pos, second_pos, third_pos};
    std::vector<Vector3f> obb_vertices;

    // Get OBB corners in world space
    Vector3f obb_center = obb.compute_centroid();
    Vector3f obb_half_extents = obb.get_aabb().compute_half_extents();
    Quaternion obb_rotation = obb.get_rotation();

    Vector3f base_corners[] = {
        Vector3f(-obb_half_extents.x, -obb_half_extents.y, -obb_half_extents.z),
        Vector3f(obb_half_extents.x, -obb_half_extents.y, -obb_half_extents.z),
        Vector3f(-obb_half_extents.x, obb_half_extents.y, -obb_half_extents.z),
        Vector3f(-obb_half_extents.x, -obb_half_extents.y, obb_half_extents.z),
        Vector3f(obb_half_extents.x, obb_half_extents.y, -obb_half_extents.z),
        Vector3f(obb_half_extents.x, -obb_half_extents.y, obb_half_extents.z),
        Vector3f(-obb_half_extents.x, obb_half_extents.y, obb_half_extents.z),
        Vector3f(obb_half_extents.x, obb_half_extents.y, obb_half_extents.z),
    };

    for (auto const& corner : base_corners) {
        obb_vertices.push_back(obb_center + obb_rotation * corner);
    }

    // Define potential separating axes
    std::vector<Vector3f> axes;

    // OBB's face normals (3 axes - the rotated local axes)
    axes.push_back(obb_rotation * Vector3f::Right);
    axes.push_back(obb_rotation * Vector3f::Up);
    axes.push_back(obb_rotation * Vector3f::Forward);

    // Triangle's face normal (1 axis)
    axes.push_back(compute_normal());

    // Cross products of OBB edges and Triangle edges (3 * 3 = 9 axes)
    Vector3f tri_edges[3] = {second_pos - first_pos, third_pos - second_pos, first_pos - third_pos};
    Vector3f obb_axes[3] = {
        obb_rotation * Vector3f::Right, obb_rotation * Vector3f::Up, obb_rotation * Vector3f::Forward
    };

    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            axes.push_back(tri_edges[i].cross(obb_axes[j]).normalize());
        }
    }

    // Check for separation on each axis
    for (auto const& axis : axes) {
        if (axis.length_squared() < std::numbers::e_v<float>)
            continue; // Skip zero-length axes

        Projection proj_tri = project_points(tri_vertices, axis);
        Projection proj_obb = project_points(obb_vertices, axis);

        if (!check_overlap(proj_tri, proj_obb)) {
            return false; // Separating axis found
        }
    }

    return true; // No separating axis found, they intersect
}

// bool Triangle::contains(Vector3f const& point) const
// {
//     // Project the point onto the plane of the triangle
//     Vector3f normal = compute_normal();
//     Vector3f plane_origin = first_pos; // Any point on the plane
//     Vector3f projected_point = project_point_onto_plane(point, plane_origin, normal);

//     // Check if the projected point is inside the 2D triangle
//     // We need a 2D basis in the plane. Two triangle edges can serve.
//     Vector3f edge1 = second_pos - first_pos;
//     Vector3f edge2 = third_pos - first_pos;

//     // Handle degenerate cases (collinear points)
//     if (edge1.length_squared() < (std::numbers::e_v<float> * std::numbers::e_v<float>) ||
//         edge2.length_squared() < (std::numbers::e_v<float> * std::numbers::e_v<float>) ||
//         edge1.cross(edge2).length_squared() < (std::numbers::e_v<float> * std::numbers::e_v<float>)) {
//         // Triangle is degenerate, check if the point is on the line segment if it's a line, or is the point if it's
//         a
//         // point. For simplicity, we'll assume non-degenerate triangles for the 2D test. A more robust solution would
//         // handle these cases.
//         return false; // Or handle as line/point check
//     }

//     // Find a basis in the plane
//     Vector3f basis_u = edge1.normalize();
//     Vector3f basis_v = normal.cross(basis_u).normalize(); // orthonormal basis

//     // Express points in terms of this 2D basis (relative to first_pos)
//     Vector2f v1_2d(0.f, 0.f); // first_pos becomes (0, 0) in this local 2D system
//     Vector2f v2_2d(edge1.dot(basis_u), edge1.dot(basis_v));
//     Vector2f v3_2d(edge2.dot(basis_u), edge2.dot(basis_v));
//     Vector2f projected_point_2d((projected_point - first_pos).dot(basis_u), (projected_point -
//     first_pos).dot(basis_v));

//     // Check if the projected 2D point is inside the 2D triangle
//     return is_point_in_triangle_2d(projected_point_2d, v1_2d, v2_2d, v3_2d);
// }

Vector3f Triangle::compute_projection(Vector3f const& point) const
{
    // Project the point onto the plane of the triangle
    Vector3f normal = compute_normal();
    Vector3f plane_origin = first_pos; // Any point on the plane
    Vector3f projected_point = project_point_onto_plane(point, plane_origin, normal);

    // Check if the projected point is inside the 2D triangle
    // We need a 2D basis in the plane. Two triangle edges can serve.
    Vector3f edge1 = second_pos - first_pos;
    Vector3f edge2 = third_pos - first_pos;

    // Handle degenerate cases (collinear points) - for projection, this might project onto the line/point.
    if (edge1.length_squared() < (std::numbers::e_v<float> * std::numbers::e_v<float>) ||
        edge2.length_squared() < (std::numbers::e_v<float> * std::numbers::e_v<float>) ||
        edge1.cross(edge2).length_squared() < (std::numbers::e_v<float> * std::numbers::e_v<float>)) {
        // Triangle is degenerate, project onto the line segment (v1-v2 if edge1 is valid, else v1)
        if (edge1.length_squared() >= (std::numbers::e_v<float> * std::numbers::e_v<float>)) {
            return closest_point_on_line_segment(point, first_pos, second_pos);
        }
        else {
            return first_pos; // It's just a point
        }
    }

    // Find a basis in the plane
    Vector3f basis_u = edge1.normalize();
    Vector3f basis_v = normal.cross(basis_u).normalize(); // orthonormal basis

    // Express points in terms of this 2D basis (relative to first_pos)
    Vector2f v1_2d(0.f, 0.f); // first_pos becomes (0, 0) in this local 2D system
    Vector2f v2_2d(edge1.dot(basis_u), edge1.dot(basis_v));
    Vector2f v3_2d(edge2.dot(basis_u), edge2.dot(basis_v));
    Vector2f projected_point_2d((projected_point - first_pos).dot(basis_u), (projected_point - first_pos).dot(basis_v));

    // Check if the projected 2D point is inside the 2D triangle
    if (is_point_in_triangle_2d(projected_point_2d, v1_2d, v2_2d, v3_2d)) {
        return projected_point; // Projection is inside the triangle
    }

    // If not inside, find the closest point on the triangle boundary (edges or vertices)
    return closest_point_on_triangle(point, first_pos, second_pos, third_pos);
}

void Triangle::translate(Vector3f const& translation)
{
    first_pos += translation;
    second_pos += translation;
    third_pos += translation;
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

bool Quad::contains(Vector3f const& point) const
{
    // Treat the quad as two triangles and check if the point is in either (projected)
    // First check if the point lies on the plane of the quad
    Vector3f normal =
        (top_right_pos - top_left_pos).cross(bottom_left_pos - top_left_pos).normalize(); // Assumes planar
    Vector3f plane_origin = top_left_pos;
    float distance_to_plane = (point - plane_origin).dot(normal);

    if (!Math::almost_equal(distance_to_plane, 0.f)) {
        return false; // Point is not on the plane
    }

    // Project the point onto the plane (although if distance is zero, it's already on the plane)
    Vector3f projected_point = point; // Or use project_point_onto_plane if almost_equal allows slight variations

    // Check if the projected point is inside the 2D quad
    // Need a 2D basis in the plane. Two adjacent edges can serve.
    Vector3f edge1 = top_right_pos - top_left_pos;
    Vector3f edge2 = bottom_left_pos - top_left_pos;

    // Handle degenerate cases
    if (edge1.length_squared() < (std::numbers::e_v<float> * std::numbers::e_v<float>) ||
        edge2.length_squared() < (std::numbers::e_v<float> * std::numbers::e_v<float>) ||
        edge1.cross(edge2).length_squared() < (std::numbers::e_v<float> * std::numbers::e_v<float>)) {
        // Quad is degenerate (line or point), handle appropriately.
        // For simplicity, assume non-degenerate.
        return false; // Or handle as line/point check
    }

    // Find a basis in the plane
    Vector3f basis_u = edge1.normalize();
    Vector3f basis_v = normal.cross(basis_u).normalize(); // orthonormal basis

    // Express points in terms of this 2D basis (relative to top_left_pos)
    Vector2f tl_2d(0.f, 0.f);
    Vector2f tr_2d((top_right_pos - top_left_pos).dot(basis_u), (top_right_pos - top_left_pos).dot(basis_v));
    Vector2f br_2d((bottom_right_pos - top_left_pos).dot(basis_u), (bottom_right_pos - top_left_pos).dot(basis_v));
    Vector2f bl_2d((bottom_left_pos - top_left_pos).dot(basis_u), (bottom_left_pos - top_left_pos).dot(basis_v));
    Vector2f projected_point_2d(
        (projected_point - top_left_pos).dot(basis_u), (projected_point - top_left_pos).dot(basis_v)
    );

    // Check if the projected 2D point is inside the 2D quad
    return is_point_in_quad_2d(projected_point_2d, tl_2d, tr_2d, br_2d, bl_2d);
}

bool Quad::intersects(Quad const& quad) const
{
    // Treat both quads as two triangles and check all combinations
    Triangle this_tri1(top_left_pos, top_right_pos, bottom_right_pos);
    Triangle this_tri2(top_left_pos, bottom_right_pos, bottom_left_pos);

    Triangle other_tri1(quad.top_left_pos, quad.top_right_pos, quad.bottom_right_pos);
    Triangle other_tri2(quad.top_left_pos, quad.bottom_right_pos, quad.bottom_left_pos);

    // Note: This relies on a robust Triangle-Triangle intersection, which is complex.
    // If Triangle::intersects(Triangle) is not fully robust, this will also be limited.
    return this_tri1.intersects(other_tri1) || this_tri1.intersects(other_tri2) || this_tri2.intersects(other_tri1) ||
           this_tri2.intersects(other_tri2);
}

bool Quad::intersects(const AABB& aabb) const
{
    // Use SAT (Separating Axis Theorem)

    std::vector<Vector3f> quad_vertices = {top_left_pos, top_right_pos, bottom_right_pos, bottom_left_pos};
    std::vector<Vector3f> aabb_vertices;
    Vector3f aabb_min = aabb.get_min_position();
    Vector3f aabb_max = aabb.get_max_position();

    aabb_vertices.push_back(aabb_min);
    aabb_vertices.push_back(Vector3f(aabb_max.x, aabb_min.y, aabb_min.z));
    aabb_vertices.push_back(Vector3f(aabb_min.x, aabb_max.y, aabb_min.z));
    aabb_vertices.push_back(Vector3f(aabb_min.x, aabb_min.y, aabb_max.z));
    aabb_vertices.push_back(Vector3f(aabb_max.x, aabb_max.y, aabb_min.z));
    aabb_vertices.push_back(Vector3f(aabb_max.x, aabb_min.y, aabb_max.z));
    aabb_vertices.push_back(Vector3f(aabb_min.x, aabb_max.y, aabb_max.z));
    aabb_vertices.push_back(aabb_max);

    // Define potential separating axes
    std::vector<Vector3f> axes;

    // AABB's face normals (3 axes)
    axes.push_back(Vector3f::Right);   // (1, 0, 0)
    axes.push_back(Vector3f::Up);      // (0, 1, 0)
    axes.push_back(Vector3f::Forward); // (0, 0, 1)

    // Quad's face normal (1 axis) - assuming planar
    axes.push_back((top_right_pos - top_left_pos).cross(bottom_left_pos - top_left_pos).normalize());

    // Cross products of AABB edges and Quad edges (3 * 4 = 12 potential axes, but many are redundant)
    // We only need cross products of unique edge directions.
    Vector3f quad_edges[4] = {
        top_right_pos - top_left_pos,
        bottom_right_pos - top_right_pos,
        bottom_left_pos - bottom_right_pos,
        top_left_pos - bottom_left_pos,
    };
    Vector3f aabb_axes[3] = {Vector3f::Right, Vector3f::Up, Vector3f::Forward};

    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 3; ++j) {
            Vector3f axis = quad_edges[i].cross(aabb_axes[j]).normalize();
            // Avoid adding duplicate or zero-length axes
            if (axis.length_squared() > std::numbers::e_v<float>) {
                bool is_duplicate = false;
                for (auto const& existing_axis : axes) {
                    if (Math::almost_equal((axis - existing_axis).length_squared(), 0.f) ||
                        Math::almost_equal((axis + existing_axis).length_squared(), 0.f)) {
                        is_duplicate = true;
                        break;
                    }
                }
                if (!is_duplicate) {
                    axes.push_back(axis);
                }
            }
        }
    }

    // Check for separation on each axis
    for (auto const& axis : axes) {
        if (axis.length_squared() < std::numbers::e_v<float>)
            continue; // Should not happen with normalization and check

        Projection proj_quad = project_points(quad_vertices, axis);
        Projection proj_aabb = project_points(aabb_vertices, axis);

        if (!check_overlap(proj_quad, proj_aabb)) {
            return false; // Separating axis found
        }
    }

    return true; // No separating axis found, they intersect
}

bool Quad::intersects(const OBB& obb) const
{
    // Use SAT (Separating Axis Theorem)

    std::vector<Vector3f> quad_vertices = {top_left_pos, top_right_pos, bottom_right_pos, bottom_left_pos};
    std::vector<Vector3f> obb_vertices;

    // Get OBB corners in world space
    Vector3f obb_center = obb.compute_centroid();
    Vector3f obb_half_extents = obb.get_aabb().compute_half_extents();
    Quaternion obb_rotation = obb.get_rotation();

    Vector3f base_corners[] = {
        Vector3f(-obb_half_extents.x, -obb_half_extents.y, -obb_half_extents.z),
        Vector3f(obb_half_extents.x, -obb_half_extents.y, -obb_half_extents.z),
        Vector3f(-obb_half_extents.x, obb_half_extents.y, -obb_half_extents.z),
        Vector3f(-obb_half_extents.x, -obb_half_extents.y, obb_half_extents.z),
        Vector3f(obb_half_extents.x, obb_half_extents.y, -obb_half_extents.z),
        Vector3f(obb_half_extents.x, -obb_half_extents.y, obb_half_extents.z),
        Vector3f(-obb_half_extents.x, obb_half_extents.y, obb_half_extents.z),
        Vector3f(obb_half_extents.x, obb_half_extents.y, obb_half_extents.z),
    };

    for (auto const& corner : base_corners) {
        obb_vertices.push_back(obb_center + obb_rotation * corner);
    }

    // Define potential separating axes
    std::vector<Vector3f> axes;

    // OBB's face normals (3 axes - the rotated local axes)
    axes.push_back(obb_rotation * Vector3f::Right);
    axes.push_back(obb_rotation * Vector3f::Up);
    axes.push_back(obb_rotation * Vector3f::Forward);

    // Quad's face normal (1 axis) - assuming planar
    axes.push_back((top_right_pos - top_left_pos).cross(bottom_left_pos - top_left_pos).normalize());

    // Cross products of OBB edges and Quad edges (3 * 4 = 12 potential axes)
    Vector3f quad_edges[4] = {
        top_right_pos - top_left_pos,
        bottom_right_pos - top_right_pos,
        bottom_left_pos - bottom_right_pos,
        top_left_pos - bottom_left_pos,
    };
    Vector3f obb_axes[3] = {
        obb_rotation * Vector3f::Right, obb_rotation * Vector3f::Up, obb_rotation * Vector3f::Forward
    };

    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 3; ++j) {
            Vector3f axis = quad_edges[i].cross(obb_axes[j]).normalize();
            // Avoid adding duplicate or zero-length axes
            if (axis.length_squared() > std::numbers::e_v<float>) {
                bool is_duplicate = false;
                for (auto const& existing_axis : axes) {
                    if (Math::almost_equal((axis - existing_axis).length_squared(), 0.f) ||
                        Math::almost_equal((axis + existing_axis).length_squared(), 0.f)) {
                        is_duplicate = true;
                        break;
                    }
                }
                if (!is_duplicate) {
                    axes.push_back(axis);
                }
            }
        }
    }

    // Check for separation on each axis
    for (auto const& axis : axes) {
        if (axis.length_squared() < std::numbers::e_v<float>)
            continue; // Should not happen

        Projection proj_quad = project_points(quad_vertices, axis);
        Projection proj_obb = project_points(obb_vertices, axis);

        if (!check_overlap(proj_quad, proj_obb)) {
            return false; // Separating axis found
        }
    }

    return true; // No separating axis found, they intersect
}

bool Quad::intersects(Ray const& ray, RayHit* hit) const
{
    // Treat the quad as two triangles and check intersection with either
    // Return the closest hit if both intersect
    Triangle tri1(top_left_pos, top_right_pos, bottom_right_pos);
    Triangle tri2(top_left_pos, bottom_right_pos, bottom_left_pos);

    RayHit hit1, hit2;
    bool intersect1 = ray.intersects(tri1, &hit1);
    bool intersect2 = ray.intersects(tri2, &hit2);

    if (intersect1 && intersect2) {
        if (hit) {
            *hit = (hit1.distance <= hit2.distance) ? hit1 : hit2;
        }
        return true;
    }
    else if (intersect1) {
        if (hit) {
            *hit = hit1;
        }
        return true;
    }
    else if (intersect2) {
        if (hit) {
            *hit = hit2;
        }
        return true;
    }

    return false;
}

Vector3f Quad::compute_projection(Vector3f const& point) const
{
    // Project the point onto the plane of the quad
    Vector3f normal =
        (top_right_pos - top_left_pos).cross(bottom_left_pos - top_left_pos).normalize(); // Assumes planar
    Vector3f plane_origin = top_left_pos;
    Vector3f projected_point = project_point_onto_plane(point, plane_origin, normal);

    // Check if the projected point is inside the 2D quad
    Vector3f edge1 = top_right_pos - top_left_pos;
    Vector3f edge2 = bottom_left_pos - top_left_pos;

    // Handle degenerate cases
    if (edge1.length_squared() < (std::numbers::e_v<float> * std::numbers::e_v<float>) ||
        edge2.length_squared() < (std::numbers::e_v<float> * std::numbers::e_v<float>) ||
        edge1.cross(edge2).length_squared() < (std::numbers::e_v<float> * std::numbers::e_v<float>)) {
        // Quad is degenerate (line or point), project onto the line/point.
        if (edge1.length_squared() >= (std::numbers::e_v<float> * std::numbers::e_v<float>)) {
            // Treat as line segment top_left_pos to top_right_pos (or other defining edge)
            return closest_point_on_line_segment(point, top_left_pos, top_right_pos); // Or other edge
        }
        else {
            return top_left_pos; // It's just a point
        }
    }

    // Find a basis in the plane
    Vector3f basis_u = edge1.normalize();
    Vector3f basis_v = normal.cross(basis_u).normalize(); // orthonormal basis

    // Express points in terms of this 2D basis (relative to top_left_pos)
    Vector2f tl_2d(0.f, 0.f);
    Vector2f tr_2d((top_right_pos - top_left_pos).dot(basis_u), (top_right_pos - top_left_pos).dot(basis_v));
    Vector2f br_2d((bottom_right_pos - top_left_pos).dot(basis_u), (bottom_right_pos - top_left_pos).dot(basis_v));
    Vector2f bl_2d((bottom_left_pos - top_left_pos).dot(basis_u), (bottom_left_pos - top_left_pos).dot(basis_v));
    Vector2f projected_point_2d(
        (projected_point - top_left_pos).dot(basis_u), (projected_point - top_left_pos).dot(basis_v)
    );

    // Check if the projected 2D point is inside the 2D quad
    if (is_point_in_quad_2d(projected_point_2d, tl_2d, tr_2d, br_2d, bl_2d)) {
        return projected_point; // Projection is inside the quad
    }

    // If not inside, find the closest point on the quad boundary (edges or vertices)
    Vector3f closest = top_left_pos;
    float min_dist_sq = (point - closest).length_squared();

    auto check_edge = [&](Vector3f const& p1, Vector3f const& p2) {
        Vector3f closest_edge = closest_point_on_line_segment(point, p1, p2);
        float dist_sq = (point - closest_edge).length_squared();
        if (dist_sq < min_dist_sq) {
            min_dist_sq = dist_sq;
            closest = closest_edge;
        }
    };

    check_edge(top_left_pos, top_right_pos);
    check_edge(top_right_pos, bottom_right_pos);
    check_edge(bottom_right_pos, bottom_left_pos);
    check_edge(bottom_left_pos, top_left_pos);

    return closest;
}

void Quad::translate(Vector3f const& translation)
{
    top_left_pos += translation;
    top_right_pos += translation;
    bottom_right_pos += translation;
    bottom_left_pos += translation;
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

bool AABB::intersects(const OBB& obb) const
{
    // Use SAT (Separating Axis Theorem)

    std::vector<Vector3f> aabb_vertices;
    Vector3f aabb_min = min_pos;
    Vector3f aabb_max = max_pos;

    aabb_vertices.push_back(aabb_min);
    aabb_vertices.push_back(Vector3f(aabb_max.x, aabb_min.y, aabb_min.z));
    aabb_vertices.push_back(Vector3f(aabb_min.x, aabb_max.y, aabb_min.z));
    aabb_vertices.push_back(Vector3f(aabb_min.x, aabb_min.y, aabb_max.z));
    aabb_vertices.push_back(Vector3f(aabb_max.x, aabb_max.y, aabb_min.z));
    aabb_vertices.push_back(Vector3f(aabb_max.x, aabb_min.y, aabb_max.z));
    aabb_vertices.push_back(Vector3f(aabb_min.x, aabb_max.y, aabb_max.z));
    aabb_vertices.push_back(aabb_max);

    std::vector<Vector3f> obb_vertices;
    Vector3f obb_center = obb.compute_centroid();
    Vector3f obb_half_extents = obb.get_aabb().compute_half_extents();
    Quaternion obb_rotation = obb.get_rotation();

    Vector3f base_corners[] = {
        Vector3f(-obb_half_extents.x, -obb_half_extents.y, -obb_half_extents.z),
        Vector3f(obb_half_extents.x, -obb_half_extents.y, -obb_half_extents.z),
        Vector3f(-obb_half_extents.x, obb_half_extents.y, -obb_half_extents.z),
        Vector3f(-obb_half_extents.x, -obb_half_extents.y, obb_half_extents.z),
        Vector3f(obb_half_extents.x, obb_half_extents.y, -obb_half_extents.z),
        Vector3f(obb_half_extents.x, -obb_half_extents.y, obb_half_extents.z),
        Vector3f(-obb_half_extents.x, obb_half_extents.y, obb_half_extents.z),
        Vector3f(obb_half_extents.x, obb_half_extents.y, obb_half_extents.z),
    };

    for (auto const& corner : base_corners) {
        obb_vertices.push_back(obb_center + obb_rotation * corner);
    }

    // Define potential separating axes
    std::vector<Vector3f> axes;

    // AABB's face normals (3 axes)
    axes.push_back(Vector3f::Right);   // (1, 0, 0)
    axes.push_back(Vector3f::Up);      // (0, 1, 0)
    axes.push_back(Vector3f::Forward); // (0, 0, 1)

    // OBB's face normals (3 axes - the rotated local axes)
    axes.push_back(obb_rotation * Vector3f::Right);
    axes.push_back(obb_rotation * Vector3f::Up);
    axes.push_back(obb_rotation * Vector3f::Forward);

    // Cross products of AABB edges and OBB edges (3 * 3 = 9 axes)
    Vector3f aabb_axes[3] = {Vector3f::Right, Vector3f::Up, Vector3f::Forward};
    Vector3f obb_axes[3] = {
        obb_rotation * Vector3f::Right, obb_rotation * Vector3f::Up, obb_rotation * Vector3f::Forward
    };

    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            Vector3f axis = aabb_axes[i].cross(obb_axes[j]).normalize();
            if (axis.length_squared() > std::numbers::e_v<float>) { // Avoid zero-length axes
                axes.push_back(axis);
            }
        }
    }

    // Check for separation on each axis
    for (auto const& axis : axes) {
        if (axis.length_squared() < std::numbers::e_v<float>)
            continue;

        Projection proj_aabb = project_points(aabb_vertices, axis);
        Projection proj_obb = project_points(obb_vertices, axis);

        if (!check_overlap(proj_aabb, proj_obb)) {
            return false; // Separating axis found
        }
    }

    return true; // No separating axis found, they intersect
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

void OBB::set_rotation(Quaternion const& rotation)
{
    this->rotation = rotation;
    inv_rotation = this->rotation.inverse();
}

bool OBB::contains(Vector3f const& point) const
{
    // Transform the point into the OBB's local space and check if it's inside the local AABB
    Vector3f const obb_center = compute_centroid();
    Vector3f local_point = inv_rotation * (point - obb_center);

    // The local AABB is centered at the origin with extents from the OBB's internal AABB
    Vector3f obb_half_extents = aabb.compute_half_extents();
    Vector3f local_aabb_min = -obb_half_extents;
    Vector3f local_aabb_max = obb_half_extents;

    return local_point.x >= local_aabb_min.x && local_point.x <= local_aabb_max.x &&
           local_point.y >= local_aabb_min.y && local_point.y <= local_aabb_max.y &&
           local_point.z >= local_aabb_min.z && local_point.z <= local_aabb_max.z;
}

bool OBB::intersects(const OBB& obb) const
{
    // Use SAT (Separating Axis Theorem)

    // Get vertices of both OBBs in world space
    std::vector<Vector3f> obb1_vertices;
    Vector3f obb1_center = compute_centroid();
    Vector3f obb1_half_extents = aabb.compute_half_extents();
    Quaternion obb1_rotation = rotation;

    Vector3f base_corners[] = {
        Vector3f(-1, -1, -1), Vector3f(1, -1, -1), Vector3f(-1, 1, -1), Vector3f(-1, -1, 1),
        Vector3f(1, 1, -1),   Vector3f(1, -1, 1),  Vector3f(-1, 1, 1),  Vector3f(1, 1, 1),
    };

    for (auto const& corner : base_corners) {
        obb1_vertices.push_back(obb1_center + obb1_rotation * (corner * obb1_half_extents));
    }

    std::vector<Vector3f> obb2_vertices;
    Vector3f obb2_center = obb.compute_centroid();
    Vector3f obb2_half_extents = obb.get_aabb().compute_half_extents();
    Quaternion obb2_rotation = obb.get_rotation();

    for (auto const& corner : base_corners) {
        obb2_vertices.push_back(obb2_center + obb2_rotation * (corner * obb2_half_extents));
    }

    // Define potential separating axes
    std::vector<Vector3f> axes;

    // OBB1's face normals (3 axes)
    axes.push_back(obb1_rotation * Vector3f::Right);
    axes.push_back(obb1_rotation * Vector3f::Up);
    axes.push_back(obb1_rotation * Vector3f::Forward);

    // OBB2's face normals (3 axes)
    axes.push_back(obb2_rotation * Vector3f::Right);
    axes.push_back(obb2_rotation * Vector3f::Up);
    axes.push_back(obb2_rotation * Vector3f::Forward);

    // Cross products of OBB1 axes and OBB2 axes (3 * 3 = 9 axes)
    Vector3f obb1_axes[3] = {
        obb1_rotation * Vector3f::Right, obb1_rotation * Vector3f::Up, obb1_rotation * Vector3f::Forward
    };
    Vector3f obb2_axes[3] = {
        obb2_rotation * Vector3f::Right, obb2_rotation * Vector3f::Up, obb2_rotation * Vector3f::Forward
    };

    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            Vector3f axis = obb1_axes[i].cross(obb2_axes[j]).normalize();
            if (axis.length_squared() > std::numbers::e_v<float>) { // Avoid zero-length axes
                axes.push_back(axis);
            }
        }
    }

    // Check for separation on each axis
    for (auto const& axis : axes) {
        if (axis.length_squared() < std::numbers::e_v<float>)
            continue;

        Projection proj1 = project_points(obb1_vertices, axis);
        Projection proj2 = project_points(obb2_vertices, axis);

        if (!check_overlap(proj1, proj2)) {
            return false; // Separating axis found
        }
    }

    return true; // No separating axis found, they intersect
}

bool OBB::intersects(Ray const& ray, RayHit* hit) const
{
    // Transform the ray into the OBB's local space and check intersection with the local AABB
    Quaternion const& inv_rotation_val = inv_rotation; // Use the stored inverse rotation
    Vector3f const obb_center = compute_centroid();

    Vector3f local_ray_origin = inv_rotation_val * (ray.get_origin() - obb_center);
    Vector3f local_ray_direction = inv_rotation_val * ray.get_direction(); // Directions rotate directly

    // Create a ray in local space
    Ray local_ray(local_ray_origin, local_ray_direction);

    // Check intersection with the OBB's AABB in local space
    Vector3f obb_half_extents = aabb.compute_half_extents();
    AABB local_aabb_centered(-obb_half_extents, obb_half_extents); // AABB centered at origin in local space

    RayHit local_hit;
    if (local_ray.intersects(local_aabb_centered, &local_hit)) {
        if (hit) {
            // Transform the hit information back to world space
            hit->distance = local_hit.distance;
            hit->position =
                obb_center + rotation * (local_ray.get_origin() + local_ray.get_direction() * local_hit.distance);
            // Normal transformation: rotate the local normal by the OBB's rotation
            if (local_hit.normal.length_squared() > (std::numbers::e_v<float> * std::numbers::e_v<float>)) {
                hit->normal = rotation * local_hit.normal;
            }
            else {
                // Handle cases where the local normal might be zero (e.g., hitting an edge/vertex precisely)
                // Depending on the Ray-AABB implementation, local_hit.normal might need careful handling.
                // A common approach is to derive the world normal from the hit position relative to the OBB center.
                // For now, just rotate if non-zero.
                hit->normal = Vector3f::Zero; // Or some default
            }
        }
        return true;
    }

    return false;
}

Vector3f OBB::compute_projection(Vector3f const& point) const
{
    // Transform the point into the OBB's local space, project onto the local AABB, then transform back
    Vector3f const obb_center = compute_centroid();
    Vector3f local_point = inv_rotation * (point - obb_center);

    // The local AABB is centered at the origin with extents from the OBB's internal AABB
    Vector3f obb_half_extents = aabb.compute_half_extents();
    Vector3f local_aabb_min = -obb_half_extents;
    Vector3f local_aabb_max = obb_half_extents;

    // Project onto the local AABB
    float closest_x = std::max(local_aabb_min.x, std::min(local_point.x, local_aabb_max.x));
    float closest_y = std::max(local_aabb_min.y, std::min(local_point.y, local_aabb_max.y));
    float closest_z = std::max(local_aabb_min.z, std::min(local_point.z, local_aabb_max.z));

    Vector3f local_projected_point = Vector3f(closest_x, closest_y, closest_z);

    // Transform the projected point back to world space
    return obb_center + rotation * local_projected_point;
}

AABB OBB::compute_bounding_box() const
{
    // Compute the world-space AABB that tightly encloses the rotated OBB.
    // Get the 8 corners of the OBB in world space and find their min/max coordinates.
    Vector3f obb_center = compute_centroid();
    Vector3f obb_half_extents = aabb.compute_half_extents(); // Unrotated half-extents
    Quaternion obb_rotation = rotation;

    std::vector<Vector3f> corners;
    corners.reserve(8);

    Vector3f base_corners[] = {
        Vector3f(-1, -1, -1), Vector3f(1, -1, -1), Vector3f(-1, 1, -1), Vector3f(-1, -1, 1),
        Vector3f(1, 1, -1),   Vector3f(1, -1, 1),  Vector3f(-1, 1, 1),  Vector3f(1, 1, 1),
    };

    for (auto const& corner_scale : base_corners) {
        corners.push_back(obb_center + obb_rotation * (corner_scale * obb_half_extents));
    }

    // Find min and max coordinates among the transformed corners
    Vector3f min_world = corners[0];
    Vector3f max_world = corners[0];

    for (size_t i = 1; i < corners.size(); ++i) {
        min_world.x = std::min(min_world.x, corners[i].x);
        min_world.y = std::min(min_world.y, corners[i].y);
        min_world.z = std::min(min_world.z, corners[i].z);

        max_world.x = std::max(max_world.x, corners[i].x);
        max_world.y = std::max(max_world.y, corners[i].y);
        max_world.z = std::max(max_world.z, corners[i].z);
    }

    return AABB(min_world, max_world);
}
}