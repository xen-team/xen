#include <script/lua_wrapper.hpp>
#include <utils/shape.hpp>
#include <utils/type_utils.hpp>

#define SOL_ALL_SAFETIES_ON 1
#include "sol/sol.hpp"

namespace xen {

using namespace TypeUtils;

void LuaWrapper::register_shape_types()
{
    sol::state& state = get_state();

    {
        sol::usertype<AABB> aabb = state.new_usertype<AABB>(
            "AABB", sol::constructors<AABB(Vector3f const&, Vector3f const&)>(), sol::base_classes, sol::bases<Shape>()
        );
        aabb["get_min_position"] = &AABB::get_min_position;
        aabb["get_max_position"] = &AABB::get_max_position;
        aabb["compute_half_extents"] = &AABB::compute_half_extents;
        aabb["contains"] = &AABB::contains;
    }

    {
        sol::usertype<Line> line = state.new_usertype<Line>(
            "Line", sol::constructors<Line(Vector3f const&, Vector3f const&)>(), sol::base_classes, sol::bases<Shape>()
        );
        line["get_begin_pos"] = &Line::get_begin_pos;
        line["get_end_pos"] = &Line::get_end_pos;
        line["length_squared"] = &Line::length_squared;
        line["length"] = &Line::length;
    }

    {
        sol::usertype<OBB> obb = state.new_usertype<OBB>(
            "OBB",
            sol::constructors<
                OBB(Vector3f const&, Vector3f const&), OBB(Vector3f const&, Vector3f const&, Quaternion const&),
                OBB(AABB const&), OBB(AABB const&, Quaternion const&)>(),
            sol::base_classes, sol::bases<Shape>()
        );
        obb["get_min_position"] = &OBB::get_min_position;
        obb["get_max_position"] = &OBB::get_max_position;
        obb["get_rotation"] = &OBB::get_rotation;
        obb["compute_half_extents"] = &OBB::compute_half_extents;
    }

    {
        sol::usertype<Plane> plane = state.new_usertype<Plane>(
            "Plane",
            sol::constructors<
                Plane(float), Plane(float, Vector3f const&), Plane(Vector3f const&),
                Plane(Vector3f const&, Vector3f const&), Plane(Vector3f const&, Vector3f const&, Vector3f const&)>(),
            sol::base_classes, sol::bases<Shape>()
        );
        plane["get_distance"] = &Plane::get_distance;
        plane["get_normal"] = &Plane::get_normal;
    }

    {
        sol::usertype<Quad> quad = state.new_usertype<Quad>(
            "Quad", sol::constructors<Quad(Vector3f const&, Vector3f const&, Vector3f const&, Vector3f const&)>(),
            sol::base_classes, sol::bases<Shape>()
        );
        quad["get_top_left_pos"] = &Quad::get_top_left_pos;
        quad["get_top_right_pos"] = &Quad::get_top_right_pos;
        quad["get_bottom_right_pos"] = &Quad::get_bottom_right_pos;
        quad["get_bottom_left_pos"] = &Quad::get_bottom_left_pos;
    }

    {
        sol::usertype<Shape> shape = state.new_usertype<Shape>("Shape", sol::no_constructor);
        shape["get_type"] = &Shape::get_type;
        shape["contains"] = &Shape::contains;
        shape["intersects"] = sol::overload(
            PickOverload<AABB const&>(&Shape::intersects), PickOverload<Line const&>(&Shape::intersects),
            PickOverload<OBB const&>(&Shape::intersects), PickOverload<Plane const&>(&Shape::intersects),
            PickOverload<Quad const&>(&Shape::intersects), PickOverload<Sphere const&>(&Shape::intersects),
            PickOverload<Triangle const&>(&Shape::intersects), PickOverload<Ray const&, RayHit*>(&Shape::intersects)
        );
        shape["translate"] = &Shape::translate;
        shape["compute_projection"] = &Shape::compute_projection;
        shape["compute_centroid"] = &Shape::compute_centroid;
        shape["compute_bounding_box"] = &Shape::compute_bounding_box;
    }

    {
        sol::usertype<Sphere> sphere = state.new_usertype<Sphere>(
            "Sphere", sol::constructors<Sphere(Vector3f const&, float)>(), sol::base_classes, sol::bases<Shape>()
        );
        sphere["get_center"] = &Sphere::get_center;
        sphere["get_radius"] = &Sphere::get_radius;
    }

    {
        sol::usertype<Triangle> triangle = state.new_usertype<Triangle>(
            "Triangle", sol::constructors<Triangle(Vector3f const&, Vector3f const&, Vector3f const&)>(),
            sol::base_classes, sol::bases<Shape>()
        );
        triangle["get_first_pos"] = &Triangle::get_first_pos;
        triangle["get_second_pos"] = &Triangle::get_second_pos;
        triangle["get_third_pos"] = &Triangle::get_third_pos;
        triangle["compute_normal"] = &Triangle::compute_normal;
        triangle["is_counter_clockwise"] = &Triangle::is_counter_clockwise;
        triangle["make_counter_clockwise"] = &Triangle::make_counter_clockwise;
    }

    state.new_enum<ShapeType>(
        "ShapeType", {{"AABB", ShapeType::AABB},
                      {"LINE", ShapeType::LINE},
                      {"OBB", ShapeType::OBB},
                      {"PLANE", ShapeType::PLANE},
                      {"QUAD", ShapeType::QUAD},
                      {"SPHERE", ShapeType::SPHERE},
                      {"TRIANGLE", ShapeType::TRIANGLE}}
    );
}
}