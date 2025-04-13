#pragma once

#include <math/math_serialization.hpp>
#include <scene/component.hpp>
#include <nlohmann/json.hpp>

namespace xen {
struct TransformComponent : public Component {
    inline static bool const registered = register_component<TransformComponent>();

public:
    Vector3f position = Vector3f(1.f);
    Vector3f rotation = Vector3f(1.f);
    Vector3f scale = Vector3f(1.f);

    TransformComponent() = default;

    TransformComponent(
        Vector3f const& position, Vector3f const& rotation = Vector3f(1.f), Vector3f const& scale = Vector3f(1.f)
    ) : position{position}, rotation{rotation}, scale{scale} {};

    // ~TransformComponent();
    // public:
    //     [[nodiscard]] constexpr Matrix4 get_model_matrix() const;

    //     constexpr void set_model_matrix(Matrix4 model);

    [[nodiscard]] constexpr Vector3f right() const { return rotation * Vector3f(1.f, 0.f, 0.f); }

    [[nodiscard]] constexpr Vector3f forward() const { return rotation * Vector3f(0.f, 0.f, 1.f); }

    [[nodiscard]] constexpr Vector3f up() const { return rotation * Vector3f(0.f, 1.f, 0.f); }

    [[nodiscard]] constexpr Matrix4 transform() const
    {
        return Matrix4(1.f).translate(position) * Quaternion(rotation).to_matrix() * Matrix4(1.f).scale(scale);
    }

    //     [[nodiscard]] constexpr Vector3f get_euler_rotation() const;

    //     constexpr void set_euler_rotation(Vector3f const& euler_rotation);

    constexpr void look_at(Vector3f const& at)
    {
        Matrix4 view_matrix = Matrix4::look_at(Vector3f(1.f), at, Vector3f::up);
        Quaternion quaternion = from_rotation_matrix(view_matrix);
        rotation = quaternion.to_euler();
    }

    template <class Archive>
    void save(Archive& ar) const
    {
        ar(position, rotation, scale);
    }
    template <class Archive>
    void load(Archive& ar)
    {
        ar(position, rotation, scale);
    }
    // void save(nlohmann::json& j)
    // {
    //     save_type(j);

    //     nlohmann::to_json(j, this->position);
    //     // save_value(j, "port", get_port());
    //     // save_value(j, "axis", axis);
    // }
    // void load(nlohmann::json const& j)
    // {
    //     load_base(j);
    //     set_port(get_value<JoystickPort>(j, "port"));
    //     load_value(j, "axis", axis);
    // }
};

inline void to_json(nlohmann::json& j, TransformComponent const& p)
{
    to_json(j["position"], p.position);
    to_json(j["rotation"], p.rotation);
    to_json(j["scale"], p.scale);
}

inline void from_json(nlohmann::json const& j, TransformComponent& p)
{
    from_json(j["position"], p.position);
    from_json(j["rotation"], p.rotation);
    from_json(j["scale"], p.scale);
}
}