#pragma once

namespace xen {
class Line;
class Plane;
class Sphere;
class Triangle;
class Quad;
class AABB;
class OBB;

/// Ray hit used to get information from a ray intersection.
struct RayHit {
    Vector3f position{};
    Vector3f normal{};
    float distance = std::numeric_limits<float>::max();
};

/// Ray defined by an origin and a normalized direction.
class Ray {
public:
    constexpr Ray(Vector3f const& origin, Vector3f const& direction) :
        origin{origin}, direction{direction},
        inverse_direction(
            (direction.x != 0.f ? 1.f / direction.x : std::numeric_limits<float>::infinity()),
            (direction.y != 0.f ? 1.f / direction.y : std::numeric_limits<float>::infinity()),
            (direction.z != 0.f ? 1.f / direction.z : std::numeric_limits<float>::infinity())
        )
    {
    }
    constexpr Ray(Ray const&) = default;
    constexpr Ray(Ray&&) noexcept = default;
    constexpr Ray& operator=(Ray const&) = default;
    constexpr Ray& operator=(Ray&&) noexcept = default;

    constexpr Vector3f const& get_origin() const { return origin; }

    constexpr Vector3f const& get_direction() const { return direction; }

    constexpr Vector3f const& get_inverse_direction() const { return inverse_direction; }

    /// Ray-point intersection check.
    /// \param point Point to check if there is an intersection with.
    /// \param hit Ray intersection's information to recover.
    /// \return True if the ray intersects the point, false otherwise.
    bool intersects(Vector3f const& point, RayHit* hit = nullptr) const;

    /// Ray-line intersection check.
    /// \param line Line to check if there is an intersection with.
    /// \param hit Ray intersection's information to recover.
    /// \return True if the ray intersects the line, false otherwise.
    bool intersects(Line const& line, RayHit* hit = nullptr) const;

    /// Ray-plane intersection check.
    /// \param plane Plane to check if there is an intersection with.
    /// \param hit Ray intersection's information to recover.
    /// \return True if the ray intersects the plane, false otherwise.
    bool intersects(Plane const& plane, RayHit* hit = nullptr) const;

    /// Ray-sphere intersection check.
    /// The intersection is checked by calculating a quadratic equation to determine the hits distances.
    /// \param sphere Sphere to check if there is an intersection with.
    /// \param hit Ray intersection's information to recover.
    /// \return True if the ray intersects the sphere, false otherwise.
    bool intersects(Sphere const& sphere, RayHit* hit = nullptr) const;

    /// Ray-triangle intersection check.
    /// The intersection is checked by calculating the barycentic coordinates at the intersection point.
    /// \param triangle Triangle to check if there is an intersection with.
    /// \param hit Ray intersection's information to recover.
    /// \note The hit normal will always be oriented towards the ray.
    /// \return True if the ray intersects the triangle, false otherwise.
    bool intersects(Triangle const& triangle, RayHit* hit = nullptr) const;

    /// Ray-quad intersection check.
    /// \param quad Quad to check if there is an intersection with.
    /// \param hit Ray intersection's information to recover.
    /// \return True if the ray intersects the quad, false otherwise.
    bool intersects(Quad const& quad, RayHit* hit = nullptr) const;

    /// Ray-AABB intersection check.
    /// \param aabb AABB to check if there is an intersection with.
    /// \param hit Ray intersection's information to recover.
    /// \note If returns true with a negative hit distance, the ray is located inside the box & the hit position is the
    /// intersection point found behind the ray.
    /// \return True if the ray intersects the AABB, false otherwise.
    bool intersects(const AABB& aabb, RayHit* hit = nullptr) const;

    /// Ray-OBB intersection check.
    /// \param obb OBB to check if there is an intersection with.
    /// \param hit Ray intersection's information to recover.
    /// \return True if the ray intersects the OBB, false otherwise.
    bool intersects(const OBB& obb, RayHit* hit = nullptr) const;

    /// Computes the projection of a point (closest point) onto the ray.
    /// The projected point is necessarily located between the ray's origin and towards infinity in the ray's direction.
    /// \param point Point to compute the projection from.
    /// \return Point projected onto the ray.
    Vector3f compute_projection(Vector3f const& point) const;

private:
    Vector3f origin{};
    Vector3f direction{};
    Vector3f inverse_direction{};
};
}