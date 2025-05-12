#pragma once

#include <component.hpp>
#include <utils/ray.hpp>

namespace xen {
class Line;
class Plane;
class Sphere;
class Triangle;
class Quad;
class AABB;
class OBB;

enum class ShapeType { LINE, PLANE, SPHERE, TRIANGLE, QUAD, AABB, OBB };

class Shape {
public:
    Shape(Shape const&) = default;
    Shape(Shape&&) noexcept = default;

    Shape& operator=(Shape const&) = default;
    Shape& operator=(Shape&&) noexcept = default;

    virtual ~Shape() = default;

    /// Gets the type of the shape.
    /// \return Shape's type.
    virtual ShapeType get_type() const = 0;

    /// Point containment check.
    /// \param point Point to be checked.
    /// \return True if the point is contained by the shape, false otherwise.
    virtual bool contains(Vector3f const& point) const = 0;

    /// Shape-line intersection check.
    /// \param line Line to check if there is an intersection with.
    /// \return True if both shapes intersect each other, false otherwise.
    virtual bool intersects(Line const& line) const = 0;

    /// Shape-plane intersection check.
    /// \param plane Plane to check if there is an intersection with.
    /// \return True if both shapes intersect each other, false otherwise.
    virtual bool intersects(Plane const& plane) const = 0;

    /// Shape-sphere intersection check.
    /// \param sphere Sphere to check if there is an intersection with.
    /// \return True if both shapes intersect each other, false otherwise.
    virtual bool intersects(Sphere const& sphere) const = 0;

    /// Shape-triangle intersection check.
    /// \param triangle Triangle to check if there is an intersection with.
    /// \return True if both shapes intersect each other, false otherwise.
    virtual bool intersects(Triangle const& triangle) const = 0;

    /// Shape-quad intersection check.
    /// \param quad Quad to check if there is an intersection with.
    /// \return True if both shapes intersect each other, false otherwise.
    virtual bool intersects(Quad const& quad) const = 0;

    /// Shape-AABB intersection check.
    /// \param aabb AABB to check if there is an intersection with.
    /// \return True if both shapes intersect each other, false otherwise.
    virtual bool intersects(const AABB& aabb) const = 0;

    /// Shape-OBB intersection check.
    /// \param obb OBB to check if there is an intersection with.
    /// \return True if both shapes intersect each other, false otherwise.
    virtual bool intersects(const OBB& obb) const = 0;

    /// Ray-shape intersection check.
    /// \param ray Ray to check if there is an intersection with.
    /// \param hit Optional ray intersection's information to recover (nullptr if unneeded).
    /// \return True if the ray intersects the shape, false otherwise.
    virtual bool intersects(Ray const& ray, RayHit* hit) const = 0;

    /// Translates the shape by the given vector.
    /// \param displacement Displacement to be translated by.
    virtual void translate(Vector3f const& displacement) = 0;

    /// Computes the projection of a point (closest point) onto the shape.
    /// \param point Point to compute the projection from.
    /// \return Point projected onto the shape.
    virtual Vector3f compute_projection(Vector3f const& point) const = 0;

    /// Computes the shape's centroid.
    /// \return Computed centroid.
    virtual Vector3f compute_centroid() const = 0;

    /// Computes the shape's bounding box.
    /// \return Computed bounding box.
    virtual AABB conpute_bounding_box() const = 0;

protected:
    Shape() = default;
};

/// Line segment defined by its two extremities' positions.
class Line final : public Shape {
public:
    Line(Vector3f const& begin_pos, Vector3f const& end_pos) : begin_pos{begin_pos}, end_pos{end_pos} {}

    ShapeType get_type() const override { return ShapeType::LINE; }

    Vector3f const& get_begin_pos() const { return begin_pos; }

    Vector3f const& get_end_pos() const { return end_pos; }

    /// Point containment check.
    /// \param point Point to be checked.
    /// \return True if the point is located on the line, false otherwise.
    bool contains(Vector3f const& point) const override { return compute_projection(point) == point; }

    /// Line-line intersection check.
    /// \param line Line to check if there is an intersection with.
    /// \return True if both lines intersect each other, false otherwise.
    bool intersects(Line const& line) const override;

    /// Line-plane intersection check.
    /// \param plane Plane to check if there is an intersection with.
    /// \return True if both shapes intersect each other, false otherwise.
    bool intersects(Plane const& plane) const override;

    /// Line-sphere intersection check.
    /// \param sphere Sphere to check if there is an intersection with.
    /// \return True if both shapes intersect each other, false otherwise.
    bool intersects(Sphere const& sphere) const override;

    /// Line-triangle intersection check.
    /// \param triangle Triangle to check if there is an intersection with.
    /// \return True if both shapes intersect each other, false otherwise.
    bool intersects(Triangle const& triangle) const override;

    /// Line-quad intersection check.
    /// \param quad Quad to check if there is an intersection with.
    /// \return True if both shapes intersect each other, false otherwise.
    bool intersects(Quad const& quad) const override;

    /// Line-AABB intersection check.
    /// \param aabb AABB to check if there is an intersection with.
    /// \return True if both shapes intersect each other, false otherwise.
    bool intersects(const AABB& aabb) const override;

    /// Line-OBB intersection check.
    /// \param obb OBB to check if there is an intersection with.
    /// \return True if both shapes intersect each other, false otherwise.
    bool intersects(const OBB& obb) const override;

    /// Ray-line intersection check.
    /// \param ray Ray to check if there is an intersection with.
    /// \param hit Optional ray intersection's information to recover (nullptr if unneeded).
    /// \return True if the ray intersects the line, false otherwise.
    bool intersects(Ray const&, RayHit*) const override { throw std::runtime_error("Error: Not implemented yet."); }

    /// Translates the line by the given vector.
    /// \param displacement Displacement to be translated by.
    void translate(Vector3f const& displacement) override;

    /// Computes the projection of a point (closest point) onto the line.
    /// The projected point is necessarily located on the line.
    /// \param point Point to compute the projection from.
    /// \return Point projected onto the line.
    Vector3f compute_projection(Vector3f const& point) const override;

    /// Computes the line's centroid, which is the point lying directly between the two extremities.
    /// \return Computed centroid.
    Vector3f compute_centroid() const override { return (begin_pos + end_pos) * 0.5f; }

    AABB conpute_bounding_box() const override;

    /// Line squared length computation.
    /// To be preferred over length() for faster operations.
    /// \return Line's squared length.
    float length_squared() const { return (end_pos - begin_pos).length_squared(); }

    /// Line length computation.
    /// To be used if the actual length is needed; otherwise, prefer length_squared().
    /// \return Line's length.
    float length() const { return (end_pos - begin_pos).length(); }

    /// Checks if the current line is equal to another given one.
    /// Uses a near-equality check to take floating-point errors into account.
    /// \param line Line to be compared with.
    /// \return True if the lines are nearly equal to each other, false otherwise.
    constexpr bool operator==(Line const& line) const
    {
        return (begin_pos == line.begin_pos && end_pos == line.end_pos);
    }

    /// Checks if the current line is different from another given one.
    /// Uses a near-equality check to take floating-point errors into account.
    /// \param line Line to be compared with.
    /// \return True if the lines are different from each other, false otherwise.
    constexpr bool operator!=(Line const& line) const { return !(*this == line); }

private:
    Vector3f begin_pos{};
    Vector3f end_pos{};
};

/// Plane defined by a distance from [ 0; 0; 0 ] and a normal.
class Plane final : public Shape {
public:
    explicit Plane(float distance, Vector3f const& normal = Vector3f::Up) : distance{distance}, normal{normal} {}

    explicit Plane(Vector3f const& position, Vector3f const& normal = Vector3f::Up) :
        distance{position.length()}, normal{normal}
    {
    }
    Plane(Vector3f const& first_point, Vector3f const& second_point, Vector3f const& third_point) :
        distance{((first_point + second_point + third_point) / 3.f).length()},
        normal{(second_point - first_point).cross(third_point - first_point).normalize()}
    {
    }

    ShapeType get_type() const override { return ShapeType::PLANE; }

    float get_distance() const { return distance; }

    Vector3f const& get_normal() const { return normal; }

    /// Point containment check.
    /// \param point Point to be checked.
    /// \return True if the point is located on the plane, false otherwise.
    bool contains(Vector3f const& point) const override
    {
        return Math::almost_equal(normal.dot(point) - distance, 0.f);
    }

    /// Plane-line intersection check.
    /// \param line Line to check if there is an intersection with.
    /// \return True if both shapes intersect each other, false otherwise.
    bool intersects(Line const& line) const override { return line.intersects(*this); }

    /// Plane-plane intersection check.
    /// \param plane Plane to check if there is an intersection with.
    /// \return True if both planes intersect each other, false otherwise.
    bool intersects(Plane const& plane) const override;

    /// Plane-sphere intersection check.
    /// \param sphere Sphere to check if there is an intersection with.
    /// \return True if both shapes intersect each other, false otherwise.
    bool intersects(Sphere const& sphere) const override;

    /// Plane-triangle intersection check.
    /// \param triangle Triangle to check if there is an intersection with.
    /// \return True if both shapes intersect each other, false otherwise.
    bool intersects(Triangle const& triangle) const override;

    /// Plane-quad intersection check.
    /// \param quad Quad to check if there is an intersection with.
    /// \return True if both shapes intersect each other, false otherwise.
    bool intersects(Quad const& quad) const override;

    /// Plane-AABB intersection check.
    /// \param aabb AABB to check if there is an intersection with.
    /// \return True if both shapes intersect each other, false otherwise.
    bool intersects(const AABB& aabb) const override;

    /// Plane-OBB intersection check.
    /// \param obb OBB to check if there is an intersection with.
    /// \return True if both shapes intersect each other, false otherwise.
    bool intersects(const OBB& obb) const override;

    /// Ray-plane intersection check.
    /// \param ray Ray to check if there is an intersection with.
    /// \param hit Optional ray intersection's information to recover (nullptr if unneeded).
    /// \return True if the ray intersects the plane, false otherwise.
    bool intersects(Ray const& ray, RayHit* hit) const override { return ray.intersects(*this, hit); }

    /// Translates the plane by the given vector.
    /// \param displacement Displacement to be translated by.
    void translate(Vector3f const& displacement) override { distance = normal.dot(compute_centroid() + displacement); }

    /// Computes the projection of a point (closest point) onto the plane.
    /// The projected point is necessarily located on the plane.
    /// \param point Point to compute the projection from.
    /// \return Point projected onto the plane.
    Vector3f compute_projection(Vector3f const& point) const override
    {
        return point - normal * (normal.dot(point) - distance);
    }

    /// Computes the plane's centroid, which is the point lying onto the plane at its distance from the center in its
    /// normal direction.
    /// \return Computed centroid.
    Vector3f compute_centroid() const override { return normal * distance; }

    AABB conpute_bounding_box() const override;

    /// Checks if the current plane is equal to another given one.
    /// Uses a near-equality check to take floating-point errors into account.
    /// \param plane Plane to be compared with.
    /// \return True if the planes are nearly equal to each other, false otherwise.
    constexpr bool operator==(Plane const& plane) const
    {
        return (Math::almost_equal(distance, plane.distance) && normal == plane.normal);
    }

    /// Checks if the current plane is different from another given one.
    /// Uses a near-equality check to take floating-point errors into account.
    /// \param plane Plane to be compared with.
    /// \return True if the planes are different from each other, false otherwise.
    constexpr bool operator!=(Plane const& plane) const { return !(*this == plane); }

private:
    float distance{};
    Vector3f normal{};
};

/// Sphere defined by its center position and a radius.
class Sphere final : public Shape {
public:
    Sphere(Vector3f const& center_pos, float radius) : center_pos{center_pos}, radius{radius} {}

    ShapeType get_type() const override { return ShapeType::SPHERE; }

    Vector3f const& get_center() const { return center_pos; }

    float get_radius() const { return radius; }

    /// Point containment check.
    /// \param point Point to be checked.
    /// \return True if the point is contained in the sphere, false otherwise.
    bool contains(Vector3f const& point) const override;

    /// Sphere-line intersection check.
    /// \param line Line to check if there is an intersection with.
    /// \return True if both shapes intersect each other, false otherwise.
    bool intersects(Line const& line) const override { return line.intersects(*this); }

    /// Sphere-plane intersection check.
    /// \param plane Plane to check if there is an intersection with.
    /// \return True if both shapes intersect each other, false otherwise.
    bool intersects(Plane const& plane) const override { return plane.intersects(*this); }

    /// Sphere-sphere intersection check.
    /// \param sphere Sphere to check if there is an intersection with.
    /// \return True if both spheres intersect each other, false otherwise.
    bool intersects(Sphere const& sphere) const override;

    /// Sphere-triangle intersection check.
    /// \param triangle Triangle to check if there is an intersection with.
    /// \return True if both shapes intersect each other, false otherwise.
    bool intersects(Triangle const& triangle) const override;

    /// Sphere-quad intersection check.
    /// \param quad Quad to check if there is an intersection with.
    /// \return True if both shapes intersect each other, false otherwise.
    bool intersects(Quad const& quad) const override;

    /// Sphere-AABB intersection check.
    /// \param aabb AABB to check if there is an intersection with.
    /// \return True if both shapes intersect each other, false otherwise.
    bool intersects(const AABB& aabb) const override;

    /// Sphere-OBB intersection check.
    /// \param obb OBB to check if there is an intersection with.
    /// \return True if both shapes intersect each other, false otherwise.
    bool intersects(const OBB& obb) const override;

    /// Ray-sphere intersection check.
    /// \param ray Ray to check if there is an intersection with.
    /// \param hit Optional ray intersection's information to recover (nullptr if unneeded).
    /// \return True if the ray intersects the sphere, false otherwise.
    bool intersects(Ray const& ray, RayHit* hit) const override { return ray.intersects(*this, hit); }

    /// Translates the sphere by the given vector.
    /// \param displacement Displacement to be translated by.
    void translate(Vector3f const& displacement) override { center_pos += displacement; }

    /// Computes the projection of a point (closest point) onto the sphere.
    /// The projected point may be inside the sphere itself or on its surface.
    /// \param point Point to compute the projection from.
    /// \return Point projected onto/into the sphere.
    Vector3f compute_projection(Vector3f const& point) const override
    {
        return (point - center_pos).normalize() * radius + center_pos;
    }

    /// Computes the sphere's centroid, which is its center. Strictly equivalent to get_center_pos().
    /// \return Computed centroid.
    Vector3f compute_centroid() const override { return center_pos; }

    AABB conpute_bounding_box() const override;

    /// Checks if the current sphere is equal to another given one.
    /// Uses a near-equality check to take floating-point errors into account.
    /// \param sphere Sphere to be compared with.
    /// \return True if the spheres are nearly equal to each other, false otherwise.
    constexpr bool operator==(Sphere const& sphere) const
    {
        return (center_pos == sphere.center_pos && Math::almost_equal(radius, sphere.radius));
    }

    /// Checks if the current sphere is different from another given one.
    /// Uses a near-equality check to take floating-point errors into account.
    /// \param sphere Sphere to be compared with.
    /// \return True if the spheres are different from each other, false otherwise.
    constexpr bool operator!=(Sphere const& sphere) const { return !(*this == sphere); }

private:
    Vector3f center_pos{};
    float radius{};
};

/// Triangle defined by its three vertices' positions, presumably in counter-clockwise order.
class Triangle final : public Shape {
public:
    Triangle(Vector3f const& first_pos, Vector3f const& second_pos, Vector3f const& third_pos) :
        first_pos{first_pos}, second_pos{second_pos}, third_pos{third_pos}
    {
    }

    ShapeType get_type() const override { return ShapeType::TRIANGLE; }

    Vector3f const& get_first_pos() const { return first_pos; }

    Vector3f const& get_second_pos() const { return second_pos; }

    Vector3f const& get_third_pos() const { return third_pos; }

    /// Point containment check.
    /// \param point Point to be checked.
    /// \return True if the point is located on the triangle, false otherwise.
    bool contains(Vector3f const& point) const override { return (compute_projection(point) == point); }

    /// Triangle-line intersection check.
    /// \param line Line to check if there is an intersection with.
    /// \return True if both shapes intersect each other, false otherwise.
    bool intersects(Line const& line) const override { return line.intersects(*this); }

    /// Triangle-plane intersection check.
    /// \param plane Plane to check if there is an intersection with.
    /// \return True if both shapes intersect each other, false otherwise.
    bool intersects(Plane const& plane) const override { return plane.intersects(*this); }

    /// Triangle-sphere intersection check.
    /// \param sphere Sphere to check if there is an intersection with.
    /// \return True if both shapes intersect each other, false otherwise.
    bool intersects(Sphere const& sphere) const override { return sphere.intersects(*this); }

    /// Triangle-triangle intersection check.
    /// \param triangle Triangle to check if there is an intersection with.
    /// \return True if both triangles intersect each other, false otherwise.
    bool intersects(Triangle const& triangle) const override;

    /// Triangle-quad intersection check.
    /// \param quad Quad to check if there is an intersection with.
    /// \return True if both shapes intersect each other, false otherwise.
    bool intersects(Quad const& quad) const override;

    /// Triangle-AABB intersection check.
    /// \param aabb AABB to check if there is an intersection with.
    /// \return True if both shapes intersect each other, false otherwise.
    bool intersects(const AABB& aabb) const override;

    /// Triangle-OBB intersection check.
    /// \param obb OBB to check if there is an intersection with.
    /// \return True if both shapes intersect each other, false otherwise.
    bool intersects(const OBB& obb) const override;

    /// Ray-triangle intersection check.
    /// \param ray Ray to check if there is an intersection with.
    /// \param hit Optional ray intersection's information to recover (nullptr if unneeded).
    /// \return True if the ray intersects the triangle, false otherwise.
    bool intersects(Ray const& ray, RayHit* hit) const override { return ray.intersects(*this, hit); }

    /// Translates the triangle by the given vector.
    /// \param displacement Displacement to be translated by.
    void translate(Vector3f const& displacement) override;

    /// Computes the projection of a point (closest point) onto the triangle.
    /// The projected point is necessarily located on the triangle's surface.
    /// \param point Point to compute the projection from.
    /// \return Point projected onto the triangle.
    Vector3f compute_projection(Vector3f const& point) const override;

    /// Computes the triangle's centroid, which is the point lying directly between its three points.
    /// \return Computed centroid.
    Vector3f compute_centroid() const override { return (first_pos + second_pos + third_pos) / 3.f; }

    AABB conpute_bounding_box() const override;

    /// Computes the triangle's normal from its points.
    /// \return Computed normal.
    Vector3f compute_normal() const;

    /// Checks if the triangle's points are defined in a counter-clockwise manner around a normal.
    /// \param normal Normal from which to determinate the ordering.
    /// \return True if the triangle is counter-clockwise, false otherwise.
    bool is_counter_clockwise(Vector3f const& normal) const { return (normal.dot(compute_normal()) > 0.f); }

    /// Makes the triangle counter-clockwise around a normal. Does nothing if it already is.
    /// \param normal Normal from which to determinate the ordering.
    void make_counter_clockwise(Vector3f const& normal);

    /// Checks if the current triangle is equal to another given one.
    /// Uses a near-equality check to take floating-point errors into account.
    /// \param triangle Triangle to be compared with.
    /// \return True if the triangles are nearly equal to each other, false otherwise.
    constexpr bool operator==(Triangle const& triangle) const
    {
        return (
            first_pos == triangle.first_pos && second_pos == triangle.second_pos && third_pos == triangle.third_pos
        );
    }

    /// Checks if the current triangle is different from another given one.
    /// Uses a near-equality check to take floating-point errors into account.
    /// \param triangle Triangle to be compared with.
    /// \return True if the triangles are different from each other, false otherwise.
    constexpr bool operator!=(Triangle const& triangle) const { return !(*this == triangle); }

private:
    Vector3f first_pos{};
    Vector3f second_pos{};
    Vector3f third_pos{};
};

/// Quad defined by its four vertices' positions, presumably in counter-clockwise order.
class Quad final : public Shape {
public:
    Quad(
        Vector3f const& left_top_pos, Vector3f const& right_top_pos, Vector3f const& right_bottom_pos,
        Vector3f const& left_bottom_pos
    ) :
        top_left_pos{left_top_pos}, top_right_pos{right_top_pos}, bottom_right_pos{right_bottom_pos},
        bottom_left_pos{left_bottom_pos}
    {
    }

    ShapeType get_type() const override { return ShapeType::QUAD; }

    Vector3f const& get_top_left_pos() const { return top_left_pos; }

    Vector3f const& get_top_right_pos() const { return top_right_pos; }

    Vector3f const& get_bottom_right_pos() const { return bottom_right_pos; }

    Vector3f const& get_bottom_left_pos() const { return bottom_left_pos; }

    /// Point containment check.
    /// \param point Point to be checked.
    /// \return True if the point is located on the quad, false otherwise.
    bool contains(Vector3f const&) const override { throw std::runtime_error("Error: Not implemented yet."); }

    /// Quad-line intersection check.
    /// \param line Line to check if there is an intersection with.
    /// \return True if both shapes intersect each other, false otherwise.
    bool intersects(Line const& line) const override { return line.intersects(*this); }

    /// Quad-plane intersection check.
    /// \param plane Plane to check if there is an intersection with.
    /// \return True if both shapes intersect each other, false otherwise.
    bool intersects(Plane const& plane) const override { return plane.intersects(*this); }

    /// Quad-sphere intersection check.
    /// \param sphere Sphere to check if there is an intersection with.
    /// \return True if both shapes intersect each other, false otherwise.
    bool intersects(Sphere const& sphere) const override { return sphere.intersects(*this); }

    /// Quad-triangle intersection check.
    /// \param triangle Triangle to check if there is an intersection with.
    /// \return True if both shapes intersect each other, false otherwise.
    bool intersects(Triangle const& triangle) const override { return triangle.intersects(*this); }

    /// Quad-quad intersection check.
    /// \param quad Quad to check if there is an intersection with.
    /// \return True if both quads intersect each other, false otherwise.
    bool intersects(Quad const& quad) const override;

    /// Quad-AABB intersection check.
    /// \param aabb AABB to check if there is an intersection with.
    /// \return True if both shapes intersect each other, false otherwise.
    bool intersects(const AABB& aabb) const override;

    /// Quad-OBB intersection check.
    /// \param obb OBB to check if there is an intersection with.
    /// \return True if both shapes intersect each other, false otherwise.
    bool intersects(const OBB& obb) const override;

    /// Ray-quad intersection check.
    /// \param ray Ray to check if there is an intersection with.
    /// \param hit Optional ray intersection's information to recover (nullptr if unneeded).
    /// \return True if the ray intersects the quad, false otherwise.
    bool intersects(Ray const&, RayHit*) const override { throw std::runtime_error("Error: Not implemented yet."); }

    /// Translates the quad by the given vector.
    /// \param displacement Displacement to be translated by.
    void translate(Vector3f const& displacement) override;

    /// Computes the projection of a point (closest point) onto the quad.
    /// The projected point is necessarily located on the quad's surface.
    /// \param point Point to compute the projection from.
    /// \return Point projected onto the quad.
    Vector3f compute_projection(Vector3f const& point) const override;

    /// Computes the quad's centroid, which is the point lying directly between its four points.
    /// \return Computed centroid.
    Vector3f compute_centroid() const override
    {
        return (top_left_pos + top_right_pos + bottom_right_pos + bottom_left_pos) * 0.25f;
    }

    AABB conpute_bounding_box() const override;

    /// Checks if the current quad is equal to another given one.
    /// Uses a near-equality check to take floating-point errors into account.
    /// \param quad Quad to be compared with.
    /// \return True if the quads are nearly equal to each other, false otherwise.
    constexpr bool operator==(Quad const& quad) const
    {
        return (
            top_left_pos == quad.top_left_pos && top_right_pos == quad.top_right_pos &&
            bottom_right_pos == quad.bottom_right_pos && bottom_left_pos == quad.bottom_left_pos
        );
    }

    /// Checks if the current quad is different from another given one.
    /// Uses a near-equality check to take floating-point errors into account.
    /// \param quad Quad to be compared with.
    /// \return True if the quads are different from each other, false otherwise.
    constexpr bool operator!=(Quad const& quad) const { return !(*this == quad); }

private:
    Vector3f top_left_pos{};
    Vector3f top_right_pos{};
    Vector3f bottom_right_pos{};
    Vector3f bottom_left_pos{};
};

/// Axis-aligned bounding box defined by its minimal and maximal vertices' positions.
///
///          _______________________
///         /|                    /|
///        / |                   / |
///       |---------------------| < max_pos
///       |  |                  |  |
///       |  |                  |  |
///       |  |                  |  |
///       |  |                  |  |
///       | /-------------------|-/
///       |/ ^ min_pos           |/
///       -----------------------
///
/// The min position designates the point in [ -X; -Y; -Z ], and the max the point in [ +X; +Y; +Z ].
class AABB final : public Shape {
public:
    AABB(Vector3f const& min_pos, Vector3f const& max_pos) : min_pos{min_pos}, max_pos{max_pos} {}

    ShapeType get_type() const override { return ShapeType::AABB; }

    Vector3f const& get_min_position() const { return min_pos; }

    Vector3f const& get_max_position() const { return max_pos; }

    /// Point containment check.
    /// \param point Point to be checked.
    /// \return True if the point is contained in the AABB, false otherwise.
    bool contains(Vector3f const& point) const override;

    /// AABB-line intersection check.
    /// \param line Line to check if there is an intersection with.
    /// \return True if both shapes intersect each other, false otherwise.
    bool intersects(Line const& line) const override { return line.intersects(*this); }

    /// AABB-plane intersection check.
    /// \param plane Plane to check if there is an intersection with.
    /// \return True if both shapes intersect each other, false otherwise.
    bool intersects(Plane const& plane) const override { return plane.intersects(*this); }

    /// AABB-sphere intersection check.
    /// \param sphere Sphere to check if there is an intersection with.
    /// \return True if both shapes intersect each other, false otherwise.
    bool intersects(Sphere const& sphere) const override { return sphere.intersects(*this); }

    /// AABB-triangle intersection check.
    /// \param triangle Triangle to check if there is an intersection with.
    /// \return True if both shapes intersect each other, false otherwise.
    bool intersects(Triangle const& triangle) const override { return triangle.intersects(*this); }

    /// AABB-quad intersection check.
    /// \param quad Quad to check if there is an intersection with.
    /// \return True if both shapes intersect each other, false otherwise.
    bool intersects(Quad const& quad) const override { return quad.intersects(*this); }

    /// AABB-AABB intersection check.
    /// \param aabb AABB to check if there is an intersection with.
    /// \return True if both AABBs intersect each other, false otherwise.
    bool intersects(const AABB& aabb) const override;

    /// AABB-OBB intersection check.
    /// \param obb OBB to check if there is an intersection with.
    /// \return True if both shapes intersect each other, false otherwise.
    bool intersects(const OBB& obb) const override;

    /// Ray-AABB intersection check.
    /// \param ray Ray to check if there is an intersection with.
    /// \param hit Optional ray intersection's information to recover (nullptr if unneeded).
    /// \return True if the ray intersects the AABB, false otherwise.
    bool intersects(Ray const& ray, RayHit* hit) const override { return ray.intersects(*this, hit); }

    /// Translates the AABB by the given vector.
    /// \param displacement Displacement to be translated by.
    void translate(Vector3f const& displacement) override;

    /// Computes the projection of a point (closest point) onto the AABB.
    /// The projected point may be inside the AABB itself or on its surface.
    /// \param point Point to compute the projection from.
    /// \return Point projected onto the shape.
    Vector3f compute_projection(Vector3f const& point) const override;

    /// Computes the AABB's centroid, which is the point lying directly between its two extremities.
    /// \return Computed centroid.
    Vector3f compute_centroid() const override { return (max_pos + min_pos) * 0.5f; }

    AABB conpute_bounding_box() const override { return *this; }

    /// Computes the half extents of the box, starting from its centroid.
    ///
    ///          _______________________
    ///         /|          ^         /|
    ///        / |          |        / |
    ///       |---------------------|  |
    ///       |  |          |       |  |
    ///       |  |          --------|->|
    ///       |  |         /        |  |
    ///       |  |        v         |  |
    ///       | /-------------------|-/
    ///       |/                    |/
    ///       -----------------------
    /// \return AABB's half extents.
    Vector3f compute_half_extents() const { return (max_pos - min_pos) * 0.5f; }

    /// Checks if the current AABB is equal to another given one.
    /// Uses a near-equality check to take floating-point errors into account.
    /// \param aabb AABB to be compared with.
    /// \return True if the AABBs are nearly equal to each other, false otherwise.
    constexpr bool operator==(const AABB& aabb) const { return (min_pos == aabb.min_pos && max_pos == aabb.max_pos); }

    /// Checks if the current AABB is different from another given one.
    /// Uses a near-equality check to take floating-point errors into account.
    /// \param aabb AABB to be compared with.
    /// \return True if the AABBs are different from each other, false otherwise.
    constexpr bool operator!=(const AABB& aabb) const { return !(*this == aabb); }

private:
    Vector3f min_pos{};
    Vector3f max_pos{};
};

/// Oriented bounding box defined by its minimal and maximal vertices' positions, as well as a rotation.
///
///          _______________________
///         /|                    /|
///        / |                   / |
///       |---------------------| < max_pos
///       |  |                  |  |
///       |  |                  |  |
///       |  |                  |  |
///       |  |                  |  |
///       | /-------------------|-/
///       |/ ^ min_pos           |/
///       -----------------------
///
/// The min position designates the point in [ -X; -Y; -Z ], and the max the point in [ +X; +Y; +Z ].
///
/// Beyond that, an OBB differs from an AABB in that it contains a rotation giving its orientation.
///
///                        /\-----------------|
///                      /    \               / |
///                    /        \           /     |
///                  /            \       /         |
///                /                \   /             |
///              /                    \                 |
///            /                     /  \-----------------|
///          /                     /    /                 /
///        /                     /    /                 /
///        \--------------------|   /                 /
///          \                   \/                 /
///            \                /  \              /
///              \            /      \          /
///                \        /          \      /
///                  \    /              \  /
///                    \/-----------------/
///
class OBB final : public Shape {
public:
    OBB(Vector3f const& min_pos, Vector3f const& max_pos) : aabb(min_pos, max_pos) {}

    OBB(Vector3f const& min_pos, Vector3f const& max_pos, Quaternion const& rotation) :
        aabb(min_pos, max_pos), rotation{rotation}
    {
    }

    explicit OBB(const AABB& aabb) : aabb{aabb} {}

    OBB(const AABB& aabb, Quaternion const& rotation) : aabb{aabb}, rotation{rotation} {}

    ShapeType get_type() const override { return ShapeType::OBB; }

    Vector3f const& get_min_position() const { return aabb.get_min_position(); }

    Vector3f const& get_max_position() const { return aabb.get_max_position(); }

    Quaternion const& get_rotation() const { return rotation; }

    void set_rotation(Quaternion const& rotation);

    /// Point containment check.
    /// \param point Point to be checked.
    /// \return True if the point is contained in the OBB, false otherwise.
    bool contains(Vector3f const& point) const override;

    /// OBB-line intersection check.
    /// \param line Line to check if there is an intersection with.
    /// \return True if both shapes intersect each other, false otherwise.
    bool intersects(Line const& line) const override { return line.intersects(*this); }

    /// OBB-plane intersection check.
    /// \param plane Plane to check if there is an intersection with.
    /// \return True if both shapes intersect each other, false otherwise.
    bool intersects(Plane const& plane) const override { return plane.intersects(*this); }

    /// OBB-sphere intersection check.
    /// \param sphere Sphere to check if there is an intersection with.
    /// \return True if both shapes intersect each other, false otherwise.
    bool intersects(Sphere const& sphere) const override { return sphere.intersects(*this); }

    /// OBB-triangle intersection check.
    /// \param triangle Triangle to check if there is an intersection with.
    /// \return True if both shapes intersect each other, false otherwise.
    bool intersects(Triangle const& triangle) const override { return triangle.intersects(*this); }

    /// OBB-quad intersection check.
    /// \param quad Quad to check if there is an intersection with.
    /// \return True if both shapes intersect each other, false otherwise.
    bool intersects(Quad const& quad) const override { return quad.intersects(*this); }

    /// OBB-AABB intersection check.
    /// \param aabb AABB to check if there is an intersection with.
    /// \return True if both shapes intersect each other, false otherwise.
    bool intersects(const AABB& aabb) const override { return aabb.intersects(*this); }

    /// OBB-OBB intersection check.
    /// \param obb OBB to check if there is an intersection with.
    /// \return True if both OBBs intersect each other, false otherwise.
    bool intersects(const OBB& obb) const override;

    /// Ray-OBB intersection check.
    /// \param ray Ray to check if there is an intersection with.
    /// \param hit Optional ray intersection's information to recover (nullptr if unneeded).
    /// \return True if the ray intersects the OBB, false otherwise.
    bool intersects(Ray const&, RayHit*) const override { throw std::runtime_error("Error: Not implemented yet."); }

    /// Translates the OBB by the given vector.
    /// \param displacement Displacement to be translated by.
    void translate(Vector3f const& displacement) override { aabb.translate(displacement); }

    /// Computes the projection of a point (closest point) onto the OBB.
    /// The projected point may be inside the AABB itself or on its surface.
    /// \param point Point to compute the projection from.
    /// \return Point projected onto the shape.
    Vector3f compute_projection(Vector3f const& point) const override;

    /// Computes the OBB's centroid, which is the point lying directly between its two extremities.
    /// \return Computed centroid.
    Vector3f compute_centroid() const override { return aabb.compute_centroid(); }

    AABB conpute_bounding_box() const override;

    /// Computes the half extents of the box, starting from its centroid.
    /// These half extents are oriented according to the box's rotation.
    ///
    ///          _______________________
    ///         /|          ^         /|
    ///        / |          |        / |
    ///       |---------------------|  |
    ///       |  |          |       |  |
    ///       |  |          --------|->|
    ///       |  |         /        |  |
    ///       |  |        v         |  |
    ///       | /-------------------|-/
    ///       |/                    |/
    ///       -----------------------
    /// \return OBB's half extents.
    Vector3f compute_half_extents() const { return aabb.compute_half_extents() * rotation; }

    /// Checks if the current OBB is equal to another given one.
    /// Uses a near-equality check to take floating-point errors into account.
    /// \param obb OBB to be compared with.
    /// \return True if the OBBs are nearly equal to each other, false otherwise.
    constexpr bool operator==(const OBB& obb) const { return (aabb == obb.aabb && rotation == obb.rotation); }

    /// Checks if the current OBB is different from another given one.
    /// Uses a near-equality check to take floating-point errors into account.
    /// \param obb OBB to be compared with.
    /// \return True if the OBBs are different from each other, false otherwise.
    constexpr bool operator!=(const OBB& obb) const { return !(*this == obb); }

private:
    AABB aabb;
    Quaternion rotation = Quaternion::Identity;
    Quaternion inv_rotation = Quaternion::Identity;
};
}