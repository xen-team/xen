#pragma once

#include <utility>

#include "math/matrix4.hpp"
#include "math/vector2.hpp"
#include "math/vector3.hpp"
#include "math/vector4.hpp"

namespace xen {
class XEN_API Ray {
private:
    Matrix4 projection, view;

    Vector2f normalized_coords;
    Vector4f clip_coords;
    Vector4f eye_coords;

    Matrix4 inverted_projection, inverted_view;
    Vector4f ray_world;

    Vector3f origin;
    Vector3f current_ray;

    bool use_mouse;
    Vector2f screen_start;

public:
    constexpr Ray(bool use_mouse, Vector2f screen_start) :
        use_mouse{use_mouse}, screen_start{std::move(screen_start)} {};

    void update(Vector3f const& current_pos, Vector2f const& mouse_pos, Matrix4 const& view, Matrix4 const& projection);

    [[nodiscard]] constexpr Vector3f point_on_ray(float distance) const;

    [[nodiscard]] constexpr Vector3f to_screen_space(Vector3f const& position) const;

    [[nodiscard]] constexpr bool is_use_mouse() const { return use_mouse; }
    void set_use_mouse(bool use_mouse) { this->use_mouse = use_mouse; }

    [[nodiscard]] constexpr Vector2f const& get_screen_start() const { return screen_start; }
    void set_screen_start(Vector2f const& screen_start) { this->screen_start = screen_start; }

    [[nodiscard]] constexpr Vector3f const& get_origin() const { return origin; }
    [[nodiscard]] constexpr Vector3f const& get_current_ray() const { return current_ray; }

    template <class Archive>
    void save(Archive& ar) const
    {
        ar(projection, view, normalized_coords, clip_coords, eye_coords, inverted_projection, inverted_view, ray_world,
           origin, current_ray, use_mouse, screen_start);
    }
    template <class Archive>
    void load(Archive& ar)
    {
        ar(projection, view, normalized_coords, clip_coords, eye_coords, inverted_projection, inverted_view, ray_world,
           origin, current_ray, use_mouse, screen_start);
    }

private:
    void update_normalized_device_coords(float mouse_x, float mouse_y);
    void update_eye_coords();
    void update_world_coords();
};
}