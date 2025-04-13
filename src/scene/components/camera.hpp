#pragma once

#include <scene/component.hpp>
#include <scene/components/transform.hpp>
#include <math/math.hpp>
#include <physics/frustum.hpp>
#include <physics/ray.hpp>
#include <system/window.hpp>

namespace xen {
enum class CameraType : uint8_t { PERSPECTIVE };

struct XEN_API CameraComponent : public Component {
    inline static bool const registered = register_component<CameraComponent>();

protected:
    Frustum view_frustum;
    Ray view_ray{false, {0.5f, 0.5f}};

    Matrix4 projection;
    Matrix4 view;

    TransformComponent transform;
    Vector3f velocity;

    CameraType type = CameraType::PERSPECTIVE;

    float fov = Math::deg_to_rad(45.f);
    float near = 0.1f;
    float far = 1000.f;

public:
    CameraComponent() = default;

    virtual ~CameraComponent() = default;

    void start() override {}
    void update() override {}

    [[nodiscard]] constexpr float get_near_plane() const { return near; }
    void set_near_plane(float near) { this->near = near; }

    [[nodiscard]] constexpr float get_far_plane() const { return far; }
    void set_far_plane(float far) { this->far = far; }

    [[nodiscard]] constexpr float get_fov() const { return fov; }
    void set_fov(float fov) { this->fov = fov; }

    void set_position(Vector3f const& position) { transform.position = position; }
    void set_rotation(Vector3f const& rotation) { transform.rotation = rotation; }
    void set_scale(Vector3f const& scale) { transform.scale = scale; }

    [[nodiscard]] constexpr TransformComponent const& get_transform() const { return transform; }
    [[nodiscard]] constexpr Vector3f const& get_position() const { return transform.position; }
    [[nodiscard]] constexpr Vector3f const& get_rotation() const { return transform.rotation; }
    [[nodiscard]] constexpr Vector3f const& get_scale() const { return transform.scale; }

    [[nodiscard]] constexpr Vector3f const& get_velocity() const { return velocity; }

    [[nodiscard]] constexpr Matrix4 const& get_view() const { return view; }

    [[nodiscard]] constexpr Matrix4 const& get_projection() const { return projection; }

    [[nodiscard]] constexpr Frustum const& get_view_frustum() const { return view_frustum; }

    [[nodiscard]] constexpr Ray const& get_view_ray() const { return view_ray; }

    void correct_camera_to_cubemap_face(uint face)
    {
        switch (face) {
        case 0:
            transform.look_at(Vector3f::left);
            break;
        case 1:
            transform.look_at(Vector3f::right);
            break;
        case 2:
            transform.look_at(Vector3f::up);
            break;
        case 3:
            transform.look_at(Vector3f::down);
            break;
        case 4:
            transform.look_at(Vector3f::front);
            break;
        case 5:
            transform.look_at(Vector3f::back);
            break;
        }
    }

    template <class Archive>
    void save(Archive& ar) const
    {
        // ar(far);
    }
    template <class Archive>
    void load(Archive& ar)
    {
        // ar(far);
    }
};
}