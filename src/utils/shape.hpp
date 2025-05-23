#pragma once

#include <component.hpp>
#include <utility>
#include <utility>
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
    virtual ShapeType get_type() const = 0;
    virtual bool contains(Vector3f const& point) const = 0;
    virtual bool intersects(Line const& line) const = 0;
    virtual bool intersects(Plane const& plane) const = 0;
    virtual bool intersects(Sphere const& sphere) const = 0;
    virtual bool intersects(Triangle const& triangle) const = 0;
    virtual bool intersects(Quad const& quad) const = 0;
    virtual bool intersects(const AABB& aabb) const = 0;
    virtual bool intersects(const OBB& obb) const = 0;
    virtual bool intersects(Ray const& ray, RayHit* hit) const = 0;
    virtual void translate(Vector3f const& displacement) = 0;
    virtual Vector3f compute_projection(Vector3f const& point) const = 0;
    virtual Vector3f compute_centroid() const = 0;
    virtual AABB compute_bounding_box() const = 0;

protected:
    Shape() = default;
};

class Line final : public Shape {
public:
    Line(Vector3f const& begin_pos, Vector3f const& end_pos) : begin_pos{begin_pos}, end_pos{end_pos} {}
    ShapeType get_type() const override { return ShapeType::LINE; }
    Vector3f const& get_begin_pos() const { return begin_pos; }
    Vector3f const& get_end_pos() const { return end_pos; }
    bool contains(Vector3f const& point) const override { return compute_projection(point) == point; }
    bool intersects(Line const& line) const override;
    bool intersects(Plane const& plane) const override;
    bool intersects(Sphere const& sphere) const override;
    bool intersects(Triangle const& triangle) const override;
    bool intersects(Quad const& quad) const override;
    bool intersects(const AABB& aabb) const override;
    bool intersects(const OBB& obb) const override;
    bool intersects(Ray const&, RayHit*) const override;
    void translate(Vector3f const& displacement) override;
    Vector3f compute_projection(Vector3f const& point) const override;
    Vector3f compute_centroid() const override { return (begin_pos + end_pos) * 0.5f; }
    AABB compute_bounding_box() const override;
    float length_squared() const { return (end_pos - begin_pos).length_squared(); }
    float length() const { return (end_pos - begin_pos).length(); }
    constexpr bool operator==(Line const& line) const
    {
        return (begin_pos == line.begin_pos && end_pos == line.end_pos);
    }
    constexpr bool operator!=(Line const& line) const { return !(*this == line); }

private:
    Vector3f begin_pos{};
    Vector3f end_pos{};
};

class Plane final : public Shape {
public:
    explicit Plane(float distance, Vector3f normal = Vector3f::Up) : distance{distance}, normal{std::move(normal)} {}
    explicit Plane(Vector3f const& position, Vector3f normal = Vector3f::Up) :
        distance{position.length()}, normal{std::move(normal)}
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
    bool contains(Vector3f const& point) const override
    {
        return Math::almost_equal(normal.dot(point) - distance, 0.f);
    }
    bool intersects(Line const& line) const override { return line.intersects(*this); }
    bool intersects(Plane const& plane) const override;
    bool intersects(Sphere const& sphere) const override;
    bool intersects(Triangle const& triangle) const override;
    bool intersects(Quad const& quad) const override;
    bool intersects(const AABB& aabb) const override;
    bool intersects(const OBB& obb) const override;
    bool intersects(Ray const& ray, RayHit* hit) const override { return ray.intersects(*this, hit); }
    void translate(Vector3f const& displacement) override { distance = normal.dot(compute_centroid() + displacement); }
    Vector3f compute_projection(Vector3f const& point) const override
    {
        return point - normal * (normal.dot(point) - distance);
    }
    Vector3f compute_centroid() const override { return normal * distance; }
    AABB compute_bounding_box() const override;
    constexpr bool operator==(Plane const& plane) const
    {
        return (Math::almost_equal(distance, plane.distance) && normal == plane.normal);
    }
    constexpr bool operator!=(Plane const& plane) const { return !(*this == plane); }

private:
    float distance{};
    Vector3f normal{};
};

class Sphere final : public Shape {
public:
    Sphere(Vector3f const& center_pos, float radius) : center_pos{center_pos}, radius{radius} {}
    ShapeType get_type() const override { return ShapeType::SPHERE; }
    Vector3f const& get_center() const { return center_pos; }
    float get_radius() const { return radius; }
    bool contains(Vector3f const& point) const override;
    bool intersects(Line const& line) const override { return line.intersects(*this); }
    bool intersects(Plane const& plane) const override { return plane.intersects(*this); }
    bool intersects(Sphere const& sphere) const override;
    bool intersects(Triangle const& triangle) const override;
    bool intersects(Quad const& quad) const override;
    bool intersects(const AABB& aabb) const override;
    bool intersects(const OBB& obb) const override;
    bool intersects(Ray const& ray, RayHit* hit) const override { return ray.intersects(*this, hit); }
    void translate(Vector3f const& displacement) override { center_pos += displacement; }
    Vector3f compute_projection(Vector3f const& point) const override
    {
        return (point - center_pos).normalize() * radius + center_pos;
    }
    Vector3f compute_centroid() const override { return center_pos; }
    AABB compute_bounding_box() const override;
    constexpr bool operator==(Sphere const& sphere) const
    {
        return (center_pos == sphere.center_pos && Math::almost_equal(radius, sphere.radius));
    }
    constexpr bool operator!=(Sphere const& sphere) const { return !(*this == sphere); }

private:
    Vector3f center_pos{};
    float radius{};
};

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
    bool contains(Vector3f const& point) const override { return (compute_projection(point) == point); }
    bool intersects(Line const& line) const override { return line.intersects(*this); }
    bool intersects(Plane const& plane) const override { return plane.intersects(*this); }
    bool intersects(Sphere const& sphere) const override { return sphere.intersects(*this); }
    bool intersects(Triangle const& triangle) const override;
    bool intersects(Quad const& quad) const override;
    bool intersects(const AABB& aabb) const override;
    bool intersects(const OBB& obb) const override;
    bool intersects(Ray const& ray, RayHit* hit) const override { return ray.intersects(*this, hit); }
    void translate(Vector3f const& displacement) override;
    Vector3f compute_projection(Vector3f const& point) const override;
    Vector3f compute_centroid() const override { return (first_pos + second_pos + third_pos) / 3.f; }
    AABB compute_bounding_box() const override;
    Vector3f compute_normal() const;
    bool is_counter_clockwise(Vector3f const& normal) const { return (normal.dot(compute_normal()) > 0.f); }
    void make_counter_clockwise(Vector3f const& normal);
    constexpr bool operator==(Triangle const& triangle) const
    {
        return (
            first_pos == triangle.first_pos && second_pos == triangle.second_pos && third_pos == triangle.third_pos
        );
    }
    constexpr bool operator!=(Triangle const& triangle) const { return !(*this == triangle); }

private:
    Vector3f first_pos{};
    Vector3f second_pos{};
    Vector3f third_pos{};
};

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
    bool contains(Vector3f const&) const override;
    bool intersects(Line const& line) const override { return line.intersects(*this); }
    bool intersects(Plane const& plane) const override { return plane.intersects(*this); }
    bool intersects(Sphere const& sphere) const override { return sphere.intersects(*this); }
    bool intersects(Triangle const& triangle) const override { return triangle.intersects(*this); }
    bool intersects(Quad const& quad) const override;
    bool intersects(const AABB& aabb) const override;
    bool intersects(const OBB& obb) const override;
    bool intersects(Ray const&, RayHit*) const override;
    void translate(Vector3f const& displacement) override;
    Vector3f compute_projection(Vector3f const& point) const override;
    Vector3f compute_centroid() const override
    {
        return (top_left_pos + top_right_pos + bottom_right_pos + bottom_left_pos) * 0.25f;
    }
    AABB compute_bounding_box() const override;
    constexpr bool operator==(Quad const& quad) const
    {
        return (
            top_left_pos == quad.top_left_pos && top_right_pos == quad.top_right_pos &&
            bottom_right_pos == quad.bottom_right_pos && bottom_left_pos == quad.bottom_left_pos
        );
    }
    constexpr bool operator!=(Quad const& quad) const { return !(*this == quad); }

private:
    Vector3f top_left_pos{};
    Vector3f top_right_pos{};
    Vector3f bottom_right_pos{};
    Vector3f bottom_left_pos{};
};
class AABB final : public Shape {
public:
    AABB(Vector3f const& min_pos, Vector3f const& max_pos) : min_pos{min_pos}, max_pos{max_pos} {}
    ShapeType get_type() const override { return ShapeType::AABB; }
    Vector3f const& get_min_position() const { return min_pos; }
    Vector3f const& get_max_position() const { return max_pos; }
    bool contains(Vector3f const& point) const override;
    bool intersects(Line const& line) const override { return line.intersects(*this); }
    bool intersects(Plane const& plane) const override { return plane.intersects(*this); }
    bool intersects(Sphere const& sphere) const override { return sphere.intersects(*this); }
    bool intersects(Triangle const& triangle) const override { return triangle.intersects(*this); }
    bool intersects(Quad const& quad) const override { return quad.intersects(*this); }
    bool intersects(const AABB& aabb) const override;
    bool intersects(const OBB& obb) const override;
    bool intersects(Ray const& ray, RayHit* hit) const override { return ray.intersects(*this, hit); }
    void translate(Vector3f const& displacement) override;
    Vector3f compute_projection(Vector3f const& point) const override;
    Vector3f compute_centroid() const override { return (max_pos + min_pos) * 0.5f; }
    AABB compute_bounding_box() const override { return *this; }
    Vector3f compute_half_extents() const { return (max_pos - min_pos) * 0.5f; }
    constexpr bool operator==(const AABB& aabb) const { return (min_pos == aabb.min_pos && max_pos == aabb.max_pos); }
    constexpr bool operator!=(const AABB& aabb) const { return !(*this == aabb); }

private:
    Vector3f min_pos{};
    Vector3f max_pos{};
};
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
    AABB const& get_aabb() const { return aabb; }
    void set_rotation(Quaternion const& rotation);
    bool contains(Vector3f const& point) const override;
    bool intersects(Line const& line) const override { return line.intersects(*this); }
    bool intersects(Plane const& plane) const override { return plane.intersects(*this); }
    bool intersects(Sphere const& sphere) const override { return sphere.intersects(*this); }
    bool intersects(Triangle const& triangle) const override { return triangle.intersects(*this); }
    bool intersects(Quad const& quad) const override { return quad.intersects(*this); }
    bool intersects(const AABB& aabb) const override { return aabb.intersects(*this); }
    bool intersects(const OBB& obb) const override;
    bool intersects(Ray const&, RayHit*) const override;
    void translate(Vector3f const& displacement) override { aabb.translate(displacement); }
    Vector3f compute_projection(Vector3f const& point) const override;
    Vector3f compute_centroid() const override { return aabb.compute_centroid(); }
    AABB compute_bounding_box() const override;
    Vector3f compute_half_extents() const { return aabb.compute_half_extents() * rotation; }
    constexpr bool operator==(const OBB& obb) const { return (aabb == obb.aabb && rotation == obb.rotation); }
    constexpr bool operator!=(const OBB& obb) const { return !(*this == obb); }

private:
    AABB aabb;
    Quaternion rotation = Quaternion::Identity;
    Quaternion inv_rotation = Quaternion::Identity;
};
}