#include <math/angle.hpp>
#include <math/perlin_noise.hpp>
#include <math/transform/transform.hpp>
#include <script/lua_wrapper.hpp>
#include <utils/type_utils.hpp>

#define SOL_ALL_SAFETIES_ON 1
#include "sol/sol.hpp"

namespace xen {

using namespace TypeUtils;

void LuaWrapper::register_math_types()
{
    sol::state& state = get_state();

    {
        {
            sol::usertype<Degreesf> degreesf =
                state.new_usertype<Degreesf>("Degreesf", sol::constructors<Degreesf(float), Degreesf(Radiansf)>());
            degreesf["value"] = sol::property(&Degreesf::value);

            degreesf.set_function(sol::meta_function::unary_minus, [](Degreesf const& deg) { return -deg; });
            degreesf.set_function(sol::meta_function::addition, &Degreesf::operator+ <float>);
            degreesf.set_function(sol::meta_function::subtraction, &Degreesf::operator- <float>);
            degreesf.set_function(sol::meta_function::multiplication, &Degreesf::operator* <float>);
            degreesf.set_function(sol::meta_function::division, &Degreesf::operator/ <float>);
        }

        {
            sol::usertype<Radiansf> radiansf =
                state.new_usertype<Radiansf>("Radiansf", sol::constructors<Radiansf(float), Radiansf(Degreesf)>());
            radiansf["value"] = &Radiansf::value;
            radiansf.set_function(sol::meta_function::unary_minus, [](Radiansf const& rad) { return -rad; });
            radiansf.set_function(sol::meta_function::addition, &Radiansf::operator+ <float>);
            radiansf.set_function(sol::meta_function::subtraction, &Radiansf::operator- <float>);
            radiansf.set_function(sol::meta_function::multiplication, &Radiansf::operator* <float>);
            radiansf.set_function(sol::meta_function::division, &Radiansf::operator/ <float>);
        }
    }

    {
        sol::table constants = state["Constant"].get_or_create<sol::table>();
        constants["Pi"] = std::numbers::pi_v<float>;
    }

    {
        sol::table math = state["Math"].get_or_create<sol::table>();
        math["randomf"] = &Math::random<float>;
        math["randomd"] = &Math::random<double>;
        math["randomi"] = &Math::random<int>;
        math["random_normalf"] = &Math::random_normal<float>;
        math["random_normald"] = &Math::random_normal<double>;
        math["random_logf"] = &Math::random_log<float>;
        math["random_logd"] = &Math::random_log<double>;
        math["random_logi"] = &Math::random_log<int>;
        math["deg_to_rad"] = &Math::deg_to_rad<float>;
        math["rad_to_deg"] = &Math::rad_to_deg<float>;
        math["deg_normal"] = &Math::deg_normal<float>;
        math["deg_normal"] = &Math::deg_normal<float>;
        math["rad_normal"] = &Math::rad_normal<float>;
        math["almost_equal"] = &Math::almost_equal<float>;
        math["smooth_damp"] = &Math::smooth_damp<float>;
        math["smooth_step"] = &Math::smooth_step<float>;
        math["lerp"] = &Math::lerp<float>;
        math["deadband"] = &Math::deadband<float>;
        math["cos_from_sin"] = &Math::cos_from_sin<float>;
    }

    {
        sol::table perlin_noise = state["PerlinNoise"].get_or_create<sol::table>();
        perlin_noise["compute_1d"] = &PerlinNoise::compute_1d;
        perlin_noise["compute_2d"] = &PerlinNoise::compute_2d;
        perlin_noise["compute_3d"] = &PerlinNoise::compute_3d;
    }

    {
        sol::usertype<Quaternion> quaternion = state.new_usertype<Quaternion>(
            "Quaternion",
            sol::constructors<
                Quaternion(float, float, float, float), Quaternion(Vector3f const&, Vector3f const&, Vector3f const&),
                Quaternion(Vector3f const&, Radiansf), Quaternion(Vector3f const&, Degreesf)>()
        );
        quaternion["x"] = &Quaternion::x;
        quaternion["y"] = &Quaternion::y;
        quaternion["z"] = &Quaternion::z;
        quaternion["w"] = &Quaternion::w;

        sol::table vector_preset = state["Quaternion"].get_or_create<sol::table>();
        quaternion["Identity"] = sol::var(&Quaternion::Identity);
        quaternion["Zero"] = sol::var(&Quaternion::Zero);
        quaternion["One"] = sol::var(&Quaternion::One);
        quaternion["PositiveInfinity"] = sol::var(&Quaternion::PositiveInfinity);
        quaternion["NegativeInfinity"] = sol::var(&Quaternion::NegativeInfinity);

        quaternion["is_identity"] = &Quaternion::is_identity;
        quaternion["is_pure"] = &Quaternion::is_pure;
        quaternion["is_normalized"] = &Quaternion::is_normalized;
        quaternion["rotation_axis"] = &Quaternion::rotation_axis;
        quaternion["inverse"] = &Quaternion::inverse;
        quaternion["conjugate"] = &Quaternion::conjugate;
        quaternion["rotate_point"] = sol::overload(
            PickOverload<Vector3f const&>(&Quaternion::rotate_point),
            PickOverload<Vector3f const&, Vector3f const&>(&Quaternion::rotate_point)
        );

        quaternion["look_at"] = &Quaternion::look_at;
        quaternion["multiply_inverse"] = &Quaternion::multiply_inverse;
        quaternion["dot"] = &Quaternion::dot;
        quaternion["slerp"] = &Quaternion::slerp;
        quaternion["normalize"] = &Quaternion::normalize;
        quaternion["length"] = &Quaternion::length;
        quaternion["length_squared"] = &Quaternion::length_squared;
        quaternion["max_component"] = &Quaternion::max_component;
        quaternion["min_component"] = &Quaternion::min_component;
        quaternion["normalize"] = &Quaternion::normalize;
        quaternion["to_matrix"] = &Quaternion::to_matrix;
        quaternion["to_rotation_matrix"] = &Quaternion::to_rotation_matrix;
        quaternion["from_rotation_matrix"] = &Quaternion::from_rotation_matrix;
        quaternion["to_euler"] = &Quaternion::to_euler;

        quaternion.set_function(
            sol::meta_function::multiplication,
            sol::overload(
                [](Quaternion const& quat, Quaternion const& other_quat) { return quat * other_quat; },
                [](Quaternion const& quat, float const& data) { return quat * data; },
                [](Quaternion const& quat, Vector3f const& data) { return quat * data; }
            )
        );

        // quaternion.set_function(sol::meta_function::unary_minus, &Quaternion::subtract);
        // quaternion.set_function(sol::meta_function::addition, &Quaternion::add);
        // quaternion.set_function(sol::meta_function::subtraction, &Quaternion::subtract);
        // quaternion.set_function(sol::meta_function::multiplication, &Quaternion::multiply);
        // quaternion.set_function(sol::meta_function::division, &Quaternion::divide);
        quaternion.set_function(
            sol::meta_function::index, sol::overload(
                                           PickNonConstOverload<uint32_t>(&Quaternion::operator[]),
                                           PickConstOverload<uint32_t>(&Quaternion::operator[])
                                       )
        );
    }

    {
        sol::usertype<Transform> transform = state.new_usertype<Transform>(
            "Transform",
            sol::constructors<
                Transform(), Transform(Vector3f const&), Transform(Vector3f const&, Quaternion const&),
                Transform(Vector3f const&, Quaternion const&, Vector3f const&)>(),
            sol::base_classes, sol::bases<Component>()
        );
        transform["position"] =
            sol::property(&Transform::get_position, PickOverload<Vector3f const&>(&Transform::set_position));
        transform["rotation"] =
            sol::property(&Transform::get_rotation, PickOverload<Quaternion const&>(&Transform::set_rotation));
        transform["scaling"] =
            sol::property(&Transform::get_scale, PickOverload<Vector3f const&>(&Transform::set_scale));

        transform["move"] = &Transform::move;
        transform["translate"] = &Transform::translate;
        transform["rotate"] = sol::overload(
            PickOverload<Quaternion const&>(&Transform::rotate), PickOverload<Radiansf, Radiansf>(&Transform::rotate),
            PickOverload<Radiansf, Radiansf, Radiansf>(&Transform::rotate)
        );
        transform["scale"] = &Transform::scale;
        transform["compute_translation"] = sol::overload(
            [](Transform& t) { return t.compute_translation(); }, PickOverload<bool>(&Transform::compute_translation)
        );
        transform["compute_transform"] = &Transform::compute_transform;
    }
}
}
