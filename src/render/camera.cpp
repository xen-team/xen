#include "camera.hpp"

#include <math/transform/transform.hpp>

#include <tracy/Tracy.hpp>

namespace xen {
Camera::Camera(
    Vector2ui const& frame_size, Vector3f offset_from_parent, Radiansf fov, float near, float far,
    ProjectionType projection_type
) :
    frame_ratio{static_cast<float>(frame_size.x) / static_cast<float>(frame_size.y)}, fov{fov}, near{near}, far{far},
    projection_type{projection_type}, offset_from_parent{std::move(offset_from_parent)}
{
    ZoneScopedN("Camera::Camera");

    compute_projection();
    compute_inverse_projection();
}

void Camera::set_fov(Radiansf fov)
{
    ZoneScopedN("Camera::set_fov");

    this->fov = fov;

    if (projection_type == ProjectionType::PERSPECTIVE) {
        compute_projection();
        compute_inverse_projection();
    }
}

void Camera::set_orthographic_bound(float bound)
{
    ZoneScopedN("Camera::set_orthographic_bound");

    ortho_bound = bound;

    if (projection_type == ProjectionType::ORTHOGRAPHIC) {
        compute_projection();
        compute_inverse_projection();
    }
}

void Camera::set_projection_type(ProjectionType projection_type)
{
    ZoneScopedN("Camera::set_projection_type");

    if (this->projection_type == projection_type)
        return; // No need to recompute the projection matrix

    this->projection_type = projection_type;

    compute_projection();
    compute_inverse_projection();
}

Matrix4 const& Camera::compute_view(Transform const& camera_transform)
{
    ZoneScopedN("Camera::compute_view");

    auto camera_transform_with_offset = camera_transform;
    camera_transform_with_offset.set_position(camera_transform.get_position() + offset_from_parent);

    view = camera_transform_with_offset.get_rotation().inverse().to_rotation_matrix() *
           camera_transform_with_offset.compute_translation(true);

    return view;
}

Matrix4 const& Camera::compute_look_at(Vector3f const& position)
{
    ZoneScopedN("Camera::compute_look_at");

    Vector3f const zAxis = (position - target).normalize();
    Vector3f const xAxis = up_axis.cross(zAxis).normalize();
    Vector3f const yAxis = zAxis.cross(xAxis);

    view = Matrix4(
        {xAxis.x, xAxis.y, xAxis.z, -xAxis.dot(position), yAxis.x, yAxis.y, yAxis.z, -yAxis.dot(position), zAxis.x,
         zAxis.y, zAxis.z, -zAxis.dot(position), 0.f, 0.f, 0.f, 1.f}
    );

    return view;
}

Matrix4 const& Camera::compute_inverse_view()
{
    ZoneScopedN("Camera::compute_inverse_view");

    inverse_view = view.inverse();
    return inverse_view;
}

Matrix4 const& Camera::compute_perspective()
{
    ZoneScopedN("Camera::compute_perspective");

    float const halfFovTangent = std::tan(fov.value * 0.5f);
    float const fovRatio = frame_ratio * halfFovTangent;
    float const planeMult = far * near;
    float const invDist = 1.f / (far - near);

    projection = Matrix4(
        {1.f / fovRatio, 0.f, 0.f, 0.f, 0.f, 1.f / halfFovTangent, 0.f, 0.f, 0.f, 0.f, -(far + near) * invDist,
         -2.f * planeMult * invDist, 0.f, 0.f, -1.f, 0.f}
    );

    return projection;
}

Matrix4 const&
Camera::compute_orthographic(float min_x, float max_x, float min_y, float max_y, float min_z, float max_z)
{
    ZoneScopedN("Camera::compute_orthographic");

    float const invDistX = 1.f / (max_x - min_x);
    float const invDistY = 1.f / (max_y - min_y);
    float const invDistZ = 1.f / (max_z - min_z);

    projection = Matrix4(
        {2.f * invDistX, 0.f, 0.f, -(max_x + min_x) * invDistX, 0.f, 2.f * invDistY, 0.f, -(max_y + min_y) * invDistY,
         0.f, 0.f, -2.f * invDistZ, -(max_z + min_z) * invDistZ, 0.f, 0.f, 0.f, 1.f}
    );

    return projection;
}

Matrix4 const& Camera::compute_orthographic()
{
    float const orthoRatio = ortho_bound * frame_ratio;
    return compute_orthographic(-orthoRatio, orthoRatio, -ortho_bound, ortho_bound, -far, far);
}

Matrix4 const& Camera::compute_projection()
{
    ZoneScopedN("Camera::compute_projection");

    if (projection_type == ProjectionType::ORTHOGRAPHIC)
        return compute_orthographic();

    return compute_perspective();
}

Matrix4 const& Camera::compute_inverse_projection()
{
    ZoneScopedN("Camera::compute_inverse_projection");

    inverse_projection = projection.inverse();
    return inverse_projection;
}

void Camera::resize_viewport(Vector2ui const& frame_size)
{
    ZoneScopedN("Camera::resize_viewport");

    float const newRatio = static_cast<float>(frame_size.x) / static_cast<float>(frame_size.y);

    if (newRatio == frame_ratio)
        return; // No need to recompute the projection matrix

    frame_ratio = newRatio;

    compute_projection();
    compute_inverse_projection();
}
}