#include "ray.hpp"

#include <utils/shape.hpp>

#include <algorithm>
#include <cmath>
#include <limits>

// Assuming Math, Vector3f, Quaternion, and the Shape classes are defined and included

// Assuming Vector2f is defined elsewhere or is a local helper struct as in the previous shape.cpp
// struct Vector2f { float x, y; ... };
// And assuming std::numbers::e_v<float> and (std::numbers::e_v<float> * std::numbers::e_v<float>) are defined
// constants. And helper functions like is_point_in_quad_2d, closest_point_on_line_segment etc. from shape.cpp

namespace {
// Helper function from shape.cpp (assuming it's available via include or copied)
// Helper for 2D cross product used in point-in-triangle test
// float sign(const Vector2f& p1, const Vector2f& p2, const Vector2f& p3) { return (p1.x - p3.x) * (p2.y - p3.y) -
// (p2.x - p3.x) * (p1.y - p3.y); } Helper for 2D point-in-triangle test using barycentric coordinates bool
// is_point_in_triangle_2d(const Vector2f& pt, const Vector2f& v1, const Vector2f& v2, const Vector2f& v3); Helper
// for 2D point-in-quad test bool is_point_in_quad_2d(const Vector2f& pt, const Vector2f& tl, const Vector2f& tr,
// const Vector2f& br, const Vector2f& bl);

constexpr bool solve_quadratic(float a, float b, float c, float& first_hit_distance, float& second_hit_distance)
{
    float const discriminant = b * b - 4.f * a * c;

    if (discriminant < 0.f) {
        return false;
    }
    else if (discriminant > 0.f) {
        float const q = -0.5f * ((b > 0) ? b + std::sqrt(discriminant) : b - std::sqrt(discriminant));

        // Avoid division by zero if 'a' is very small, though for ray-sphere 'a' should be direction.dot(direction)
        // which is 1 for normalized ray
        if (std::abs(a) < std::numbers::e_v<float>)
            return false;
        first_hit_distance = q / a;
        // Avoid division by zero if 'q' is very small
        if (std::abs(q) < std::numbers::e_v<float>)
            return false;
        second_hit_distance = c / q;
    }
    else { // discriminant == 0
        if (std::abs(a) < std::numbers::e_v<float>)
            return false;
        float const hit_distance = -0.5f * b / a;

        first_hit_distance = hit_distance;
        second_hit_distance = hit_distance;
    }

    if (first_hit_distance > second_hit_distance) {
        std::swap(first_hit_distance, second_hit_distance);
    }

    return true;
}
} // namespace

namespace xen {

// Assuming the Ray class definition and other includes from ray.hpp are here

// Assuming helper functions from shape.cpp (like project_point_onto_plane, is_point_in_triangle_2d, etc.)
// are available or included. If not, they need to be added here or included from a common header.

// Minimal definition for Vector2f if not provided by includes
// Adjust based on your actual Vector2f implementation
// struct Vector2f {
//     float x, y;
//     // Add necessary constructors and operators (like dot, arithmetic) if used by helpers
// };

// Helper functions that might be needed locally if not available via includes
// float sign(const Vector2f& p1, const Vector2f& p2, const Vector2f& p3);
// bool is_point_in_triangle_2d(const Vector2f& pt, const Vector2f& v1, const Vector2f& v2, const Vector2f& v3);
// bool is_point_in_quad_2d(const Vector2f& pt, const Vector2f& tl, const Vector2f& tr, const Vector2f& br, const
// Vector2f& bl); Vector3f project_point_onto_plane(Vector3f const& point, Vector3f const& plane_origin, Vector3f const&
// plane_normal);

bool Ray::intersects(Vector3f const& point, RayHit* hit) const
{
    // Check if the point lies on the infinite line defined by the ray.
    Vector3f const ray_to_point = point - origin;

    // Check if the point is "behind" the ray's origin.
    // The dot product of the ray direction and the vector from origin to point
    // must be non-negative.
    if (ray_to_point.dot(direction) < -std::numbers::e_v<float>) {
        return false;
    }

    // Check if the point lies on the line by checking if the vector from origin to point
    // is parallel to the ray direction. The cross product should be zero.
    if (ray_to_point.cross(direction).length_squared() > (std::numbers::e_v<float> * std::numbers::e_v<float>)) {
        return false;
    }

    // If the point is on the line and not behind the origin, it intersects.
    if (hit) {
        hit->position = point;
        // Normal for point intersection is not well-defined, maybe direction to point?
        // Or maybe zero/context dependent. Let's use normalized vector from origin if point is not origin.
        if ((point - origin).length_squared() > (std::numbers::e_v<float> * std::numbers::e_v<float>)) {
            hit->normal = (point - origin).normalize(); // Points away from origin
        }
        else {
            hit->normal = Vector3f::Zero; // Or some default
        }

        hit->distance = ray_to_point.length();
    }

    return true;
}

bool Ray::intersects(Line const& line, RayHit* hit) const
{
    // Ray-Line Segment intersection.
    // Ray: P = origin + t * direction (t >= 0)
    // Line Segment: Q = line.begin_pos + u * (line.end_pos - line.begin_pos) (0 <= u <= 1)
    // Find t and u such that P = Q.

    Vector3f const p1 = origin;
    Vector3f const d1 = direction; // Ray direction (normalized)

    Vector3f const p2 = line.get_begin_pos();
    Vector3f const d2 = line.get_end_pos() - line.get_begin_pos(); // Line segment direction

    Vector3f const r = p1 - p2; // Vector from segment start to ray origin

    float const a = d1.dot(d1); // Should be 1 if direction is normalized
    float const b = d1.dot(d2);
    float const c = d2.dot(d2); // Squared length of segment vector
    float const e = r.dot(d1);
    float const f = r.dot(d2);

    float const denom = a * c - b * b; // Denominator of the system solution

    // If denom is close to zero, the ray and the line are parallel or collinear.
    if (Math::almost_equal(denom, 0.f)) {
        // Check for collinearity (r is parallel to d1 or d2)
        // If collinear, check for overlap of the ray [origin, inf) and segment [begin_pos, end_pos].
        if (r.cross(d1).length_squared() < (std::numbers::e_v<float> * std::numbers::e_v<float>)) { // Collinear
            // Project segment endpoints onto the ray's line.
            float t_begin = (line.get_begin_pos() - p1).dot(d1); // Parameter for segment start on ray's line
            float t_end = (line.get_end_pos() - p1).dot(d1);     // Parameter for segment end on ray's line
            if (t_begin > t_end)
                std::swap(t_begin, t_end); // Ensure t_begin is the smaller parameter

            // Check for overlap between the ray's parameter range [0, inf) and the segment's parameter range [t_begin,
            // t_end].
            float overlap_min = std::max(0.f, t_begin);
            float overlap_max = t_end; // Ray extends to infinity, segment ends at t_end

            if (overlap_min <= overlap_max + std::numbers::e_v<float>) {
                // There is an overlap. The closest intersection distance is overlap_min.
                // Check if the point at overlap_min is actually on the segment (it should be by definition of overlap).
                Vector3f intersection_point = p1 + overlap_min * d1;
                // Need to check if this point is within the original segment bounds [begin_pos, end_pos]
                // This is implicitly handled by the overlap calculation if the collinearity check is robust.

                if (hit) {
                    hit->distance = overlap_min;
                    hit->position = intersection_point;
                    // Normal is not well-defined for ray-line intersection.
                    // hit->normal = ???; // Maybe Vector3f::Zero or context dependent
                }
                return true;
            }
        }
        return false; // Parallel and not collinear, or collinear but no overlap
    }

    // Lines are not parallel. Solve for parameters t and u.
    // Using Cramer's rule or similar algebraic manipulation:
    // t = (f * b - e * c) / denom
    // u = (a * f - e * b) / denom // Note: My previous derivation had u = (a * f - b * e) / denom, double checking...
    // Let's use the system:
    // d1.x * t - d2.x * u = -r.x
    // d1.y * t - d2.y * u = -r.y
    // d1.z * t - d2.z * u = -r.z
    // Select two non-parallel equations to solve. A more robust method uses dot products:
    // d1 . (R = u * D1 - t * D2) => d1 . R = u * (d1 . D1) - t * (d1 . D2) => e = u * a - t * b
    // d2 . (R = u * D1 - t * D2) => d2 . R = u * (d2 . D1) - t * (d2 . D2) => -f = u * b - t * c  => f = t * c - u * b
    // System:
    // u * a - t * b = e
    // -u * b + t * c = f
    // Solve for t and u:
    // Multiply first by c, second by b:
    // u * ac - t * bc = ec
    // -u * b^2 + t * bc = fb
    // Add them: u * (ac - b^2) = ec + fb => u * denom = ec + fb => u = (ec + fb) / denom
    // Multiply first by b, second by a:
    // u * ab - t * b^2 = eb
    // -u * ab + t * ac = fa
    // Add them: t * (ac - b^2) = eb + fa => t * denom = eb + fa => t = (eb + fa) / denom
    // My derivation for t was (b*f - c*e) / denom.
    // (eb + fa) / denom = (e*b + f*a) / denom. This seems different.
    // Let's re-check the standard formula for closest points on two lines in 3D.
    // The parameters s and t for closest points on P1 + s*D1 and P2 + t*D2 are:
    // s = ( (P2-P1).D1 * D2.D2 - (P2-P1).D2 * D1.D2 ) / (D1.D1 * D2.D2 - (D1.D2)^2)
    // t = ( (P2-P1).D1 * D1.D2 - (P2-P1).D2 * D1.D1 ) / (D1.D1 * D2.D2 - (D1.D2)^2)
    // In our case, Ray: P1=origin, D1=direction. Segment: P2=line.begin_pos, D2=line.end_pos - line.begin_pos.
    // r = origin - line.begin_pos => P2 - P1 = line.begin_pos - origin = -r.
    // Our parameters are t (for ray) and u (for segment).
    // t (ray) = ( (-r).direction * (line.end_pos - line.begin_pos).length_squared() - (-r).(line.end_pos -
    // line.begin_pos) * direction.(line.end_pos - line.begin_pos) ) / (direction.length_squared() * (line.end_pos -
    // line.begin_pos).length_squared() - (direction.(line.end_pos - line.begin_pos))^2) t = ( (-e) * c - (-f) * b ) /
    // (a * c - b^2) = (-ec + fb) / denom = (fb - ec) / denom. Yes, my previous formula for t was correct. u (segment) =
    // ( (-r).direction * direction.(line.end_pos - line.begin_pos) - (-r).(line.end_pos - line.begin_pos) *
    // direction.length_squared() ) / (direction.length_squared() * (line.end_pos - line.begin_pos).length_squared() -
    // (direction.(line.end_pos - line.begin_pos))^2) u = ( (-e) * b - (-f) * a ) / denom = (-eb + fa) / denom = (fa -
    // eb) / denom. Yes, my previous formula for u was correct.

    float const t = (f * b - e * c) / denom; // parameter for ray (p1 + t * d1)
    float const u = (f * a - e * b) / denom; // parameter for segment (p2 + u * d2)

    // Check if the intersection point (where the infinite lines intersect) lies within the
    // constraints of the ray (t >= 0) and the segment (0 <= u <= 1).
    if (t >= 0.f - std::numbers::e_v<float> && u >= 0.f - std::numbers::e_v<float> &&
        u <= 1.f + std::numbers::e_v<float>) {
        // Check if the intersection point is actually close on both lines (due to floating point)
        Vector3f const intersection_point_ray = p1 + t * d1;
        Vector3f const intersection_point_segment = p2 + u * d2;

        if ((intersection_point_ray - intersection_point_segment).length_squared() <
            (std::numbers::e_v<float> * std::numbers::e_v<float>)) {
            if (hit) {
                hit->distance = t; // Distance along the ray
                hit->position = intersection_point_ray;
                // Normal is not well-defined for ray-line intersection.
                // hit->normal = ???; // Maybe Vector3f::Zero or context dependent
            }
            return true;
        }
    }

    return false;
}

bool Ray::intersects(Plane const& plane, RayHit* hit) const
{
    // The intersection logic was already implemented in the provided ray.cpp
    // Ensure this implementation is correct and respects hit->distance > 0 or >= 0 depending on convention.
    // The current implementation checks hit_distance <= 0.f and returns false, meaning only hits in front of the ray
    // origin are considered.

    float const direction_dot_normal = direction.dot(plane.get_normal());

    // If the ray is parallel to the plane (dot product is close to zero)
    if (Math::almost_equal(std::abs(direction_dot_normal), 0.f)) {
        // Check if the ray origin lies on the plane (within tolerance).
        // If origin is on the plane and parallel, the ray is coplanar and infinite intersection.
        // If origin is not on the plane and parallel, no intersection.
        float const origin_distance_to_plane = origin.dot(plane.get_normal()) - plane.get_distance();
        return Math::almost_equal(origin_distance_to_plane, 0.f); // Coplanar
    }

    // Calculate the distance (parameter t) along the ray to the intersection point.
    // Intersection point P = origin + t * direction
    // P lies on the plane: plane.get_normal() . P = plane.get_distance()
    // plane.get_normal() . (origin + t * direction) = plane.get_distance()
    // plane.get_normal() . origin + t * (plane.get_normal() . direction) = plane.get_distance()
    // t * (plane.get_normal() . direction) = plane.get_distance() - plane.get_normal() . origin
    // t = (plane.get_distance() - plane.get_normal() . origin) / (plane.get_normal() . direction)
    float const hit_distance = (plane.get_distance() - origin.dot(plane.get_normal())) / direction_dot_normal;

    // The original code checked direction_angle >= 0.f and returned false, which means
    // it only hits planes where the normal points *towards* the ray origin (dot product is negative).
    // And it checked hit_distance <= 0.f and returned false, meaning only hits in front are valid.
    // This is a valid convention. Let's stick to that.

    // If the hit distance is negative, the intersection point is behind the ray origin.
    if (hit_distance <
        -std::numbers::e_v<float>) { // Allow small negative distance due to float precision if origin is on plane
        return false;
    }

    // If the dot product is positive, the plane normal points away from the ray origin along the ray direction.
    // This means the plane is "facing away" from the ray, or the ray starts behind the front face.
    // If hit_distance >= 0 and direction_dot_normal > 0, the ray hits the back face or is inside and exiting.
    // The original code returns false if direction_angle >= 0.f (which is direction.dot(normal)).
    // Let's preserve that behavior: only hit the "front" side of the plane where normal points against ray direction.
    if (direction_dot_normal >
        std::numbers::e_v<float>) { // Ray direction aligns with normal (hitting back face or inside exiting)
        // However, if hit_distance is 0 (origin is on the plane), it should intersect regardless of normal direction.
        if (hit_distance > std::numbers::e_v<float>) { // Only discard if strictly in front and facing away
            return false;
        }
    }

    if (hit) {
        hit->position = origin + direction * hit_distance;
        hit->normal = plane.get_normal(); // Normal of the plane
        hit->distance = hit_distance;
    }

    return true;
}

bool Ray::intersects(Sphere const& sphere, RayHit* hit) const
{
    // This implementation was already provided and seems correct for finding the first intersection
    // in front of the ray origin using the quadratic formula.

    Vector3f const origin_to_center = origin - sphere.get_center();

    // Quadratic equation components: at^2 + bt + c = 0
    // a = direction.dot(direction) (1 if normalized)
    // b = 2 * direction.dot(origin - center) = 2 * direction.dot(origin_to_center)
    // c = (origin - center).dot(origin - center) - radius^2 = origin_to_center.length_squared() - radius^2

    float const a = direction.dot(direction); // Should be 1.0
    float const b = 2.f * direction.dot(origin_to_center);
    float const c = origin_to_center.length_squared() - sphere.get_radius() * sphere.get_radius();

    float first_hit_distance{};
    float second_hit_distance{};

    if (!solve_quadratic(a, b, c, first_hit_distance, second_hit_distance)) {
        return false; // No real roots, no intersection
    }

    // If both distances are negative, both intersection points are behind the ray origin.
    if (second_hit_distance < -std::numbers::e_v<float>) { // Check the larger distance first
        return false;
    }

    // The first hit we care about must be in front of the ray origin or at the origin (distance >= 0).
    float final_hit_distance = first_hit_distance;

    if (final_hit_distance < -std::numbers::e_v<float>) { // If first hit is behind, use the second hit
        final_hit_distance = second_hit_distance;
    }

    // If the final chosen hit distance is still negative (or very close to zero with epsilon issues),
    // it means the closest valid intersection is behind the ray origin.
    if (final_hit_distance < -std::numbers::e_v<float>) { // Use strict comparison after checks? Or >= 0?
        // Let's require hit distance to be >= 0 for a valid intersection in front or at origin.
        // If hit_distance is negative, the intersection is behind. The Ray-Sphere test typically
        // returns the *first* hit along the ray. If the ray starts inside the sphere,
        // first_hit_distance will be negative (distance to entry point behind origin)
        // and second_hit_distance will be positive (distance to exit point in front).
        // So, if first_hit_distance is negative, we should use second_hit_distance if it's >= 0.
        if (first_hit_distance < 0.f - std::numbers::e_v<float>) { // Ray starts inside or sphere is behind
            final_hit_distance = second_hit_distance;
            if (final_hit_distance < 0.f - std::numbers::e_v<float>) { // If both are behind
                return false;
            }
        }
    }

    // Final check for valid hit distance
    if (final_hit_distance < -std::numbers::e_v<float>) { // Should ideally be >= 0 or just > epsilon for strict hits
        return false;                                     // Intersection is behind the ray origin
    }

    if (hit) {
        Vector3f const hit_pos = origin + direction * final_hit_distance;

        hit->position = hit_pos;
        hit->normal = (hit_pos - sphere.get_center()).normalize(); // Normal points outwards from sphere center
        hit->distance = final_hit_distance;
    }

    return true;
}

bool Ray::intersects(Triangle const& triangle, RayHit* hit) const
{
    // This implementation uses the Moller-Trumbore algorithm, which is standard and correct.
    // It calculates the barycentric coordinates and the distance t along the ray.
    // It checks if t >= 0 and if barycentric coords are within [0, 1] and sum <= 1.

    Vector3f const edge1 = triangle.get_second_pos() - triangle.get_first_pos();
    Vector3f const edge2 = triangle.get_third_pos() - triangle.get_first_pos();
    Vector3f const p_vec = direction.cross(edge2); // RayDir x Edge2

    // Determinant: dot(Edge1, p_vec). If determinant is close to zero, ray is parallel to triangle plane.
    float const determinant = edge1.dot(p_vec);

    // Use a tolerance for the determinant check. If near zero, no intersection or ray is in plane.
    if (std::abs(determinant) < std::numbers::e_v<float>) {
        return false; // Ray is parallel to the triangle plane
    }

    float const inverse_determinant = 1.f / determinant;

    // Calculate distance from v1 to ray origin
    Vector3f const t_vec = origin - triangle.get_first_pos();

    // Calculate barycentric coordinate u (often alpha) and test bounds [0, 1]
    float const u = t_vec.dot(p_vec) * inverse_determinant;

    if (u < 0.f - std::numbers::e_v<float> ||
        u > 1.f + std::numbers::e_v<float>) { // Use epsilon for boundary inclusion
        return false;
    }

    // Prepare for calculating barycentric coordinate v (often beta)
    Vector3f const q_vec = t_vec.cross(edge1); // t_vec x Edge1

    // Calculate barycentric coordinate v (often beta) and test bounds [0, 1] and u + v <= 1
    float const v = direction.dot(q_vec) * inverse_determinant;

    if (v < 0.f - std::numbers::e_v<float> ||
        u + v > 1.f + std::numbers::e_v<float>) { // Use epsilon for boundary inclusion
        return false;
    }

    // Calculate the distance t along the ray
    float const hit_distance = edge2.dot(q_vec) * inverse_determinant;

    // Check if the intersection point is in front of the ray origin
    if (hit_distance < -std::numbers::e_v<float>) { // Allow small negative distance if origin is on triangle
        return false;                               // Intersection is behind the ray origin
    }

    if (hit) {
        hit->position = origin + direction * hit_distance;
        // Normal should face the ray origin side. Moller-Trumbore doesn't inherently give this.
        // The cross product edge1 x edge2 gives a normal. If dot(direction, normal) is positive,
        // the ray hits the back face, so flip the normal.
        Vector3f triangle_normal = edge1.cross(edge2); // Unnormalized normal
        if (triangle_normal.length_squared() < (std::numbers::e_v<float> * std::numbers::e_v<float>)) {
            hit->normal = Vector3f::Zero; // Degenerate triangle
        }
        else {
            triangle_normal = triangle_normal.normalize();
            if (direction.dot(triangle_normal) > 0) {
                hit->normal = -triangle_normal; // Ray hits back face, flip normal
            }
            else {
                hit->normal = triangle_normal; // Ray hits front face
            }
        }

        hit->distance = hit_distance;
    }

    return true;
}

bool Ray::intersects(Quad const& quad, RayHit* hit) const
{
    // Treat the quad as two triangles and check intersection with either ray-triangle.
    // Return the closest hit if both intersect.
    // Assumes the quad is planar and convex and the triangulation is valid.
    Triangle tri1(quad.get_top_left_pos(), quad.get_top_right_pos(), quad.get_bottom_right_pos());
    Triangle tri2(quad.get_top_left_pos(), quad.get_bottom_right_pos(), quad.get_bottom_left_pos());

    RayHit hit1, hit2;
    bool intersect1 = intersects(tri1, &hit1); // Use Ray::intersects(Triangle)
    bool intersect2 = intersects(tri2, &hit2); // Use Ray::intersects(Triangle)

    if (intersect1 && intersect2) {
        // Return the hit with the minimum distance (closest to ray origin)
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

    return false; // No intersection with either triangle
}

bool Ray::intersects(const AABB& aabb, RayHit* hit) const
{
    // This implementation using the branchless method was already provided and seems correct.
    // It handles cases where the ray origin is inside the AABB, resulting in a non-positive hit distance.

    Vector3f const min_t = (aabb.get_min_position() - origin) * inverse_direction;
    Vector3f const max_t = (aabb.get_max_position() - origin) * inverse_direction;

    float const min_t_x = std::min(min_t.x, max_t.x);
    float const max_t_x = std::max(min_t.x, max_t.x);

    float const min_t_y = std::min(min_t.y, max_t.y);
    float const max_t_y = std::max(min_t.y, max_t.y);

    float const min_t_z = std::min(min_t.z, max_t.z);
    float const max_t_z = std::max(min_t.z, max_t.z);

    // The closest intersection distance is the maximum of the minimums.
    float const t_enter = std::max({min_t_x, min_t_y, min_t_z});
    // The furthest intersection distance is the minimum of the maximums.
    float const t_exit = std::min({max_t_x, max_t_y, max_t_z});

    // If t_enter > t_exit, the ray misses the AABB.
    // If t_exit < 0, the AABB is entirely behind the ray origin.
    // A valid intersection occurs if t_enter <= t_exit AND t_exit >= 0.
    // If t_enter < 0, the ray origin is inside the AABB. The first intersection point in front
    // is at t_exit (if t_exit >= 0).
    // If t_enter >= 0, the first intersection point is at t_enter.

    if (t_enter > t_exit + std::numbers::e_v<float> ||
        t_exit < -std::numbers::e_v<float>) { // Use epsilon for robustness
        return false;                         // No intersection or AABB is behind ray
    }

    // Determine the actual hit distance. It's the point where the ray enters the AABB (t_enter),
    // but only if that point is not behind the ray origin.
    float hit_distance = t_enter;

    // If t_enter is negative, the ray origin is inside the AABB. The hit distance should be 0 or the distance to the
    // exit point. The convention noted in the comment is "If returns true with a negative hit distance, the ray is
    // located inside". This means we should return t_enter even if negative, to indicate the ray started inside. Let's
    // follow that convention. The closest intersection point *along the infinite line* is at t_enter. If t_enter < 0,
    // the origin is inside. The first point hit *by the ray* is either the origin (distance 0) or the exit point
    // (distance t_exit). The note says "hit position is the intersection point found behind the ray" for negative
    // distance. This implies t_enter is the distance. So, the hit distance is simply t_enter. The RayHit stores this
    // parameter t.

    // If the ray starts inside (t_enter < 0), the actual hit point *on the boundary* in front is at t_exit.
    // However, the hit distance returned usually represents the parameter 't' along the ray for the first point of
    // intersection. If the ray origin is inside, t=0 is inside. The first intersection with the *surface* is the exit
    // point. Some conventions return the distance to the exit point when inside. Some return the distance to the entry
    // point (negative). The comment suggests the latter. Let's return t_enter.

    if (hit) {
        // Use t_enter for the hit distance as per the comment's implied convention
        hit->distance = t_enter;
        hit->position = origin + direction * hit->distance; // Position at t_enter

        // Compute normal based on which face was hit at t_enter.
        // Find the axis plane (x, y, or z) corresponding to the max(min_t_x, min_t_y, min_t_z).
        // This indicates which plane was entered first.
        Vector3f normal = Vector3f::Zero;
        if (Math::almost_equal(t_enter, min_t_x)) {
            normal = (direction.x < 0) ? Vector3f::Right : -Vector3f::Right; // Normal points opposite to ray direction
        }
        else if (Math::almost_equal(t_enter, min_t_y)) {
            normal = (direction.y < 0) ? Vector3f::Up : -Vector3f::Up; // Normal points opposite to ray direction
        }
        else if (Math::almost_equal(t_enter, min_t_z)) {
            normal =
                (direction.z < 0) ? Vector3f::Forward : -Vector3f::Forward; // Normal points opposite to ray direction
        }
        // Handle edge/corner cases where t_enter is equal for multiple axes. Normal might be sum or undefined.
        // The John Novak method used in the original code:
        // `Vector3f const hit_direction = (hit->position - aabb.compute_centroid()) / aabb.compute_half_extents();`
        // `hit->normal = Vector3f(std::trunc(hit_direction.x), std::trunc(hit_direction.y),
        // std::trunc(hit_direction.z)).normalize();` This calculates a vector from the center to the hit point, scaled
        // by inverse half-extents, and then truncates the components to get (-1, 0, or 1) to identify the face normal.
        // This is a clever way to get the face normal. Let's use that method for the normal.

        Vector3f const aabb_center = aabb.compute_centroid();
        Vector3f const aabb_half_extents = aabb.compute_half_extents(); // Needs to be non-zero
        Vector3f const hit_relative_to_center = hit->position - aabb_center;

        // Avoid division by zero if half_extents are zero
        Vector3f scaled_hit_direction = Vector3f::Zero;
        if (std::abs(aabb_half_extents.x) > std::numbers::e_v<float>)
            scaled_hit_direction.x = hit_relative_to_center.x / aabb_half_extents.x;
        if (std::abs(aabb_half_extents.y) > std::numbers::e_v<float>)
            scaled_hit_direction.y = hit_relative_to_center.y / aabb_half_extents.y;
        if (std::abs(aabb_half_extents.z) > std::numbers::e_v<float>)
            scaled_hit_direction.z = hit_relative_to_center.z / aabb_half_extents.z;

        hit->normal = Vector3f(
            std::trunc(scaled_hit_direction.x), std::trunc(scaled_hit_direction.y), std::trunc(scaled_hit_direction.z)
        );
        // Normal should point outwards. The trunc method gets (-1, 0, 1). Normalize might not be needed if it's already
        // axis aligned. Let's just use the trunc vector directly as the normal direction.
        if (hit->normal.length_squared() > (std::numbers::e_v<float> * std::numbers::e_v<float>)) {
            hit->normal = hit->normal.normalize();
        }
        else {
            hit->normal = Vector3f::Zero; // Degenerate case
        }
    }

    return true;
}

bool Ray::intersects(const OBB& obb, RayHit* hit) const
{
    // Transform the ray into the OBB's local space and check intersection with the local AABB.
    Quaternion const& inv_rotation = obb.get_rotation().inverse(); // Assumes inv_rotation is correctly managed in OBB

    Vector3f const obb_center = obb.compute_centroid();

    // Transform ray origin and direction into OBB local space
    Vector3f local_ray_origin = inv_rotation * (origin - obb_center);
    Vector3f local_ray_direction = inv_rotation * direction; // Directions rotate directly

    // Create a ray in local space (direction should remain normalized if original direction was)
    Ray local_ray(local_ray_origin, local_ray_direction);

    // Check intersection with the OBB's AABB in local space.
    // The AABB stored in OBB represents the bounds *before* rotation, relative to the OBB's centroid.
    // So the local AABB for intersection testing should be centered at the origin with extents from the OBB's internal
    // AABB.
    Vector3f obb_half_extents = obb.get_aabb().compute_half_extents(); // Use internal AABB's half extents
    AABB local_aabb_centered(-obb_half_extents, obb_half_extents);     // AABB centered at origin in local space

    RayHit local_hit;
    // Use the Ray::intersects(AABB, RayHit*) method
    if (local_ray.intersects(local_aabb_centered, &local_hit)) {
        if (hit) {
            // Transform the hit information back to world space
            hit->distance = local_hit.distance; // Distance along the original ray is the same

            // Position needs to be transformed back from local space to world space
            // local_hit.position is in local space. Add OBB center and apply OBB rotation.
            hit->position = obb_center + obb.get_rotation() * local_hit.position;

            // Normal transformation: rotate the local normal by the OBB's rotation.
            // Local normal from AABB intersection is axis-aligned (-1,0,0), (0,1,0), etc.
            if (local_hit.normal.length_squared() > (std::numbers::e_v<float> * std::numbers::e_v<float>)) {
                hit->normal = obb.get_rotation() * local_hit.normal;
            }
            else {
                hit->normal = Vector3f::Zero; // Handle degenerate local normal
            }
        }
        return true;
    }

    return false; // No intersection in local space
}

Vector3f Ray::compute_projection(Vector3f const& point) const
{
    // This was already provided and seems correct for projecting onto the ray starting from origin.
    float const point_dist = direction.dot(point - origin);
    // Project only if the point is in front of the ray origin (dot product >= 0).
    // If point_dist < 0, the projection onto the infinite line is behind the origin,
    // but the projection onto the ray (which starts at origin) is the origin itself.
    return (origin + direction * std::max(point_dist, 0.f));
}
} // namespace xen