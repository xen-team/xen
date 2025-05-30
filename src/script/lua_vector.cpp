#include <script/lua_wrapper.hpp>
#include <utils/type_utils.hpp>

#define SOL_ALL_SAFETIES_ON 1
#include "sol/sol.hpp"

namespace xen {

using namespace TypeUtils;

namespace {
template <typename DataType>
constexpr void register_vector2(sol::state& state, std::string_view name)
{
    {
        using VectorDataType = DataType;
        using VectorType = Vector2<VectorDataType>;

        sol::usertype<VectorType> vector = state.new_usertype<VectorType>(
            name, sol::constructors<
                      VectorType(), VectorType(VectorDataType), VectorType(VectorDataType, VectorDataType),
                      VectorType(Vector3<VectorDataType> const&)>()
        );

        vector["x"] = sol::property(&VectorType::x);
        vector["y"] = sol::property(&VectorType::y);

        vector["add"] = &VectorType::template add<VectorDataType>;
        vector["subtract"] = &VectorType::template subtract<VectorDataType>;
        vector["multiply"] = &VectorType::template multiply<VectorDataType>;
        vector["divide"] = &VectorType::template divide<VectorDataType>;
        vector["angle"] = &VectorType::template angle<VectorDataType>;
        vector["dot"] = &VectorType::template dot<VectorDataType>;

        vector["lerp"] = &VectorType::template lerp<VectorDataType>;

        vector["scale"] = &VectorType::template scale<VectorDataType>;

        vector["rotate"] =
            sol::overload(&VectorType::template rotate<float>, &VectorType::template rotate<float, VectorType>);
        vector["aspect_ratio"] = &VectorType::aspect_ratio;
        vector["normalize"] = &VectorType::normalize;
        vector["length"] = &VectorType::length;
        vector["length_squared"] = &VectorType::length_squared;
        vector["abs"] = &VectorType::abs;

        vector["min"] = sol::overload(
            [](VectorType const& vec) { return vec.min(); },
            PickOverload<VectorType const&>(&VectorType::template min<VectorDataType>)
        );

        vector["min"] = sol::overload(
            [](VectorType const& vec) { return vec.min(); },
            PickOverload<VectorType const&>(&VectorType::template min<VectorDataType>)
        );

        vector["distance"] = &VectorType::template distance<VectorDataType>;
        vector["distance_squared"] = &VectorType::template distance_squared<VectorDataType>;
        vector["distance_vector"] = &VectorType::template distance_vector<VectorDataType>;
        vector["in_triangle"] = &VectorType::template in_triangle<VectorDataType>;

        vector["cartesian_to_polar"] = &VectorType::cartesian_to_polar;
        vector["polar_to_cartesian"] = &VectorType::polar_to_cartesian;
        vector["hash"] = &VectorType::hash;

        if constexpr (std::is_same_v<VectorDataType, float> || std::is_same_v<VectorDataType, int>) {
            sol::table vector_preset = state[name].get_or_create<sol::table>();
            vector_preset["Zero"] = sol::var(&VectorType::Zero);
            vector_preset["One"] = sol::var(&VectorType::One);
            vector_preset["Infinity"] = sol::var(&VectorType::Infinity);
            vector_preset["Left"] = sol::var(&VectorType::Left);
            vector_preset["Right"] = sol::var(&VectorType::Right);
            vector_preset["Up"] = sol::var(&VectorType::Up);
            vector_preset["Down"] = sol::var(&VectorType::Down);
        }

        vector.set_function(sol::meta_function::unary_minus, &VectorType::template subtract<VectorDataType>);
        vector.set_function(sol::meta_function::addition, &VectorType::template add<VectorDataType>);
        vector.set_function(sol::meta_function::subtraction, &VectorType::template subtract<VectorDataType>);
        vector.set_function(
            sol::meta_function::multiplication,
            sol::overload(
                [](VectorType const& vec, VectorDataType const& data) { return vec * data; },
                [](VectorType const& vec, VectorType const& other_vec) { return vec * other_vec; }
            )
        );
        vector.set_function(sol::meta_function::division, &VectorType::template divide<VectorDataType>);
        vector.set_function(
            sol::meta_function::index, sol::overload(
                                           PickNonConstOverload<uint32_t>(&VectorType::operator[]),
                                           PickConstOverload<uint32_t>(&VectorType::operator[])
                                       )
        );
    }
}

template <typename DataType>
constexpr void register_vector3(sol::state& state, std::string_view name)
{
    {
        using VectorDataType = DataType;
        using VectorType = Vector3<VectorDataType>;

        sol::usertype<VectorType> vector = state.new_usertype<VectorType>(
            name,
            sol::constructors<
                VectorType(), VectorType(VectorDataType), VectorType(VectorDataType, VectorDataType, VectorDataType)>()
        );

        vector["x"] = sol::property(&VectorType::x);
        vector["y"] = sol::property(&VectorType::y);
        vector["z"] = sol::property(&VectorType::z);

        vector["add"] = &VectorType::template add<VectorDataType>;
        vector["subtract"] = &VectorType::template subtract<VectorDataType>;
        vector["multiply"] = &VectorType::template multiply<VectorDataType>;
        vector["divide"] = &VectorType::template divide<VectorDataType>;
        vector["angle"] = &VectorType::template angle<VectorDataType>;
        vector["dot"] = &VectorType::template dot<VectorDataType>;
        vector["cross"] = &VectorType::template cross<VectorDataType>;
        vector["lerp"] = &VectorType::template lerp<VectorDataType, float>;
        vector["scale"] = &VectorType::template scale<float>;

        vector["normalize"] = &VectorType::normalize;
        vector["length"] = &VectorType::length;
        vector["length_squared"] = &VectorType::length_squared;
        vector["abs"] = &VectorType::abs;

        vector["min"] = sol::overload(
            [](VectorType const& vec) { return vec.min(); },
            PickOverload<VectorType const&>(&VectorType::template min<VectorDataType>)
        );

        vector["min"] = sol::overload(
            [](VectorType const& vec) { return vec.min(); },
            PickOverload<VectorType const&>(&VectorType::template min<VectorDataType>)
        );

        vector["distance"] = &VectorType::template distance<VectorDataType>;
        vector["distance_squared"] = &VectorType::template distance_squared<VectorDataType>;
        vector["distance_vector"] = &VectorType::template distance_vector<VectorDataType>;
        vector["smooth_damp"] = &VectorType::template smooth_damp<VectorDataType, VectorDataType>;

        vector["cartesian_to_polar"] = &VectorType::cartesian_to_polar;
        vector["polar_to_cartesian"] = &VectorType::polar_to_cartesian;
        vector["hash"] = &VectorType::hash;

        if constexpr (std::is_same_v<VectorDataType, float> || std::is_same_v<VectorDataType, int>) {
            sol::table vector_preset = state[name].get_or_create<sol::table>();
            vector_preset["Zero"] = sol::var(&VectorType::Zero);
            vector_preset["One"] = sol::var(&VectorType::One);
            vector_preset["Infinity"] = sol::var(&VectorType::Infinity);
            vector_preset["Left"] = sol::var(&VectorType::Left);
            vector_preset["Right"] = sol::var(&VectorType::Right);
            vector_preset["Up"] = sol::var(&VectorType::Up);
            vector_preset["Down"] = sol::var(&VectorType::Down);
            vector_preset["Front"] = sol::var(&VectorType::Forward);
            vector_preset["Back"] = sol::var(&VectorType::Back);
        }

        vector.set_function(sol::meta_function::unary_minus, &VectorType::template subtract<VectorDataType>);
        vector.set_function(sol::meta_function::addition, &VectorType::template add<VectorDataType>);
        vector.set_function(sol::meta_function::subtraction, &VectorType::template subtract<VectorDataType>);
        vector.set_function(
            sol::meta_function::multiplication,
            sol::overload(
                [](VectorType const& vec, VectorDataType const& data) { return vec * data; },
                [](VectorType const& vec, VectorType const& other_vec) { return vec * other_vec; }
            )
        );
        vector.set_function(sol::meta_function::division, &VectorType::template divide<VectorDataType>);
        vector.set_function(
            sol::meta_function::index, sol::overload(
                                           PickNonConstOverload<uint32_t>(&VectorType::operator[]),
                                           PickConstOverload<uint32_t>(&VectorType::operator[])
                                       )
        );
    }
}

template <typename DataType>
constexpr void register_vector4(sol::state& state, std::string_view name)
{
    {
        using VectorDataType = DataType;
        using VectorType = Vector4<VectorDataType>;

        sol::usertype<VectorType> vector = state.new_usertype<VectorType>(
            name, sol::constructors<
                      VectorType(), VectorType(VectorDataType),
                      VectorType(VectorDataType, VectorDataType, VectorDataType, VectorDataType)>()
        );

        vector["x"] = sol::property(&VectorType::x);
        vector["y"] = sol::property(&VectorType::y);
        vector["z"] = sol::property(&VectorType::z);
        vector["w"] = sol::property(&VectorType::w);

        vector["add"] = &VectorType::template add<VectorDataType>;
        vector["subtract"] = &VectorType::template subtract<VectorDataType>;
        vector["multiply"] = &VectorType::template multiply<VectorDataType>;
        vector["divide"] = &VectorType::template divide<VectorDataType>;

        vector["angle"] = &VectorType::template angle<VectorDataType>;
        vector["dot"] = &VectorType::template dot<VectorDataType>;
        vector["lerp"] = &VectorType::template lerp<VectorDataType, float>;
        vector["scale"] = &VectorType::template scale<float>;

        vector["normalize"] = &VectorType::normalize;
        vector["length"] = &VectorType::length;
        vector["length_squared"] = &VectorType::length_squared;
        vector["abs"] = &VectorType::abs;

        vector["min"] = sol::overload(
            [](VectorType const& vec) { return vec.min(); },
            PickOverload<VectorType const&>(&VectorType::template min<VectorDataType>)
        );

        vector["min"] = sol::overload(
            [](VectorType const& vec) { return vec.min(); },
            PickOverload<VectorType const&>(&VectorType::template min<VectorDataType>)
        );

        vector["distance"] = &VectorType::template distance<VectorDataType>;
        vector["distance_squared"] = &VectorType::template distance_squared<VectorDataType>;
        vector["distance_vector"] = &VectorType::template distance_vector<VectorDataType>;
        vector["hash"] = &VectorType::hash;

        if constexpr (std::is_same_v<VectorDataType, float> || std::is_same_v<VectorDataType, int>) {
            sol::table vector_preset = state[name].get_or_create<sol::table>();
            vector_preset["Zero"] = sol::var(&VectorType::Zero);
            vector_preset["One"] = sol::var(&VectorType::One);
            vector_preset["Infinity"] = sol::var(&VectorType::Infinity);
        }

        vector.set_function(sol::meta_function::unary_minus, &VectorType::template subtract<VectorDataType>);
        vector.set_function(sol::meta_function::addition, &VectorType::template add<VectorDataType>);
        vector.set_function(sol::meta_function::subtraction, &VectorType::template subtract<VectorDataType>);
        vector.set_function(
            sol::meta_function::multiplication,
            sol::overload(
                [](VectorType const& vec, VectorDataType const& data) { return vec * data; },
                [](VectorType const& vec, VectorType const& other_vec) { return vec * other_vec; }
            )
        );
        vector.set_function(sol::meta_function::division, &VectorType::template divide<VectorDataType>);
        vector.set_function(
            sol::meta_function::index, sol::overload(
                                           PickNonConstOverload<uint32_t>(&VectorType::operator[]),
                                           PickConstOverload<uint32_t>(&VectorType::operator[])
                                       )
        );
    }
}
}

void LuaWrapper::register_vector_types()
{
    sol::state& state = get_state();

    register_vector2<int>(state, "Vector2i");
    register_vector2<uint>(state, "Vector2ui");
    register_vector2<float>(state, "Vector2f");
    register_vector2<uint8_t>(state, "Vector2ub");

    register_vector3<int>(state, "Vector3i");
    register_vector3<uint>(state, "Vector3ui");
    register_vector3<float>(state, "Vector3f");
    register_vector3<uint8_t>(state, "Vector3ub");

    register_vector4<int>(state, "Vector4i");
    register_vector4<uint>(state, "Vector4ui");
    register_vector4<float>(state, "Vector4f");
    register_vector4<uint8_t>(state, "Vector4ub");

    // Attempting to hash vectors can give an error: 'integer value will be misrepresented in lua'
    // This could be solved by defining SOL_ALL_INTEGER_VALUES_FIT to 1 (see
    // https://github.com/ThePhD/sol2/issues/1178#issuecomment-833923813),
    //   but this gives signed integers thus can (and does) produce negative values
    // Unsigned 64-bit integers seem to be supported in Lua 5.3 and above, so there may (will) be a better solution
    // Manually hashing a vector should not be useful on Lua's side anyway
}
}
