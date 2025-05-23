#include "ray.hpp"

namespace xen {
void Ray::update(Vector3f const& current_pos, Vector2f const& mouse_pos, Matrix4 const& view, Matrix4 const& projection)
{
    origin = current_pos;

    if (use_mouse) {
        update_normalized_device_coords(mouse_pos.x, mouse_pos.y);
    }
    else {
        normalized_coords = screen_start;
    }

    this->view = view;
    this->projection = projection;
    clip_coords = {normalized_coords.x, normalized_coords.y, -1.0f};

    update_eye_coords();
    update_world_coords();
}

constexpr Vector3f Ray::point_on_ray(float const distance) const
{
    Vector3f const vector = distance * current_ray;
    return origin + vector;
}

constexpr Vector3f Ray::to_screen_space(Vector3f const& position) const
{
    Vector4f coords(position);
    coords = view.transform(coords);
    coords = projection.transform(coords);

    if (coords.w < 0.0f) {
        return {};
    }

    return {(coords.x / coords.w + 1.0f) / 2.0f, 1.0f - ((coords.y / coords.w + 1.0f) / 2.0f), coords.z};
}

void Ray::update_normalized_device_coords(float const mouse_x, float const mouse_y)
{
    normalized_coords.x = (2.0f * mouse_x) - 1.0f;
    normalized_coords.y = (2.0f * mouse_y) - 1.0f;
}

void Ray::update_eye_coords()
{
    inverted_projection = projection.inverse();
    eye_coords = inverted_projection.transform(clip_coords);
    eye_coords = {eye_coords.x, eye_coords.y, -1.0f, 0.0f};
}

void Ray::update_world_coords()
{
    inverted_view = view.inverse();
    ray_world = inverted_view.transform(eye_coords);
    current_ray = {ray_world};
}
}