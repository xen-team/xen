#pragma once

#include "math_types.hpp"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace xen {
template <typename T>
void to_json(json& j, Vector2<T> const& v)
{
    j = json{{"x", v.x}, {"y", v.y}};
}

template <typename T>
void from_json(json const& j, Vector2<T>& v)
{
    j.at("x").get_to(v.x);
    j.at("y").get_to(v.y);
}

template <typename T>
void to_json(json& j, Vector3<T> const& v)
{
    j = json{{"x", v.x}, {"y", v.y}, {"z", v.z}};
}

template <typename T>
void from_json(json const& j, Vector3<T>& v)
{
    j.at("x").get_to(v.x);
    j.at("y").get_to(v.y);
    j.at("z").get_to(v.z);
}

template <typename T>
void to_json(json& j, Vector4<T> const& v)
{
    j = json{{"x", v.x}, {"y", v.y}, {"z", v.z}, {"w", v.w}};
}

template <typename T>
void from_json(json const& j, Vector4<T>& v)
{
    j.at("x").get_to(v.x);
    j.at("y").get_to(v.y);
    j.at("z").get_to(v.z);
    j.at("w").get_to(v.w);
}

inline void to_json(json& j, Matrix2 const& v)
{
    for (size_t i = 0; i < Matrix2::COUNT; i++) {
        j = json{{"x" + std::to_string(i), v[i].x}, {"y" + std::to_string(i), v[i].y}};
    }
}

inline void from_json(json const& j, Matrix2& v)
{
    for (size_t i = 0; i < Matrix2::COUNT; i++) {
        j.at("x" + std::to_string(i)).get_to(v[i].x);
        j.at("y" + std::to_string(i)).get_to(v[i].y);
    }
}

inline void to_json(json& j, Matrix3 const& v)
{
    for (size_t i = 0; i < Matrix3::COUNT; i++) {
        j = json{
            {"x" + std::to_string(i), v[i].x}, {"y" + std::to_string(i), v[i].y}, {"z" + std::to_string(i), v[i].z}
        };
    }
}

inline void from_json(json const& j, Matrix3& v)
{
    for (size_t i = 0; i < Matrix3::COUNT; i++) {
        j.at("x" + std::to_string(i)).get_to(v[i].x);
        j.at("y" + std::to_string(i)).get_to(v[i].y);
        j.at("z" + std::to_string(i)).get_to(v[i].z);
    }
}

inline void to_json(json& j, Matrix4 const& v)
{
    for (size_t i = 0; i < Matrix4::COUNT; i++) {
        j = json{
            {"x" + std::to_string(i), v[i].x},
            {"y" + std::to_string(i), v[i].y},
            {"z" + std::to_string(i), v[i].z},
            {"w" + std::to_string(i), v[i].w}
        };
    }
}

inline void from_json(json const& j, Matrix4& v)
{
    for (size_t i = 0; i < Matrix4::COUNT; i++) {
        j.at("x" + std::to_string(i)).get_to(v[i].x);
        j.at("y" + std::to_string(i)).get_to(v[i].y);
        j.at("z" + std::to_string(i)).get_to(v[i].z);
        j.at("w" + std::to_string(i)).get_to(v[i].w);
    }
}

inline void to_json(json& j, Quaternion const& v)
{
    j = json{{"x", v.x}, {"y", v.y}, {"z", v.z}, {"w", v.w}};
}

inline void from_json(json const& j, Quaternion& v)
{
    j.at("x").get_to(v.x);
    j.at("y").get_to(v.y);
    j.at("z").get_to(v.z);
    j.at("w").get_to(v.w);
}
}