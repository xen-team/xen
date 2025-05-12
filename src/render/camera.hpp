#pragma once

#include <component.hpp>
#include <math/angle.hpp>

namespace xen {
class Transform;

enum class CameraType : uint8_t { FREE_FLY = 0, LOOK_AT };

enum class ProjectionType : uint8_t { PERSPECTIVE = 0, ORTHOGRAPHIC };

/// Camera simulating a point of view for a scene to be rendered from.
class Camera final : public Component {
public:
    Camera() = default;
    Camera(
        Vector2ui const& frame_size, Radiansf fov = Degreesf(45.f), float near = 0.1f, float far = 1000.f,
        ProjectionType projection_type = ProjectionType::PERSPECTIVE
    );

    Radiansf get_fov() const { return fov; }
    float get_orthographic_bound() const { return ortho_bound; }
    CameraType get_camera_type() const { return camera_type; }
    Matrix4 const& get_view() const { return view; }
    Matrix4 const& get_inverse_view() const { return inverse_view; }
    Matrix4 const& get_projection() const { return projection; }
    Matrix4 const& get_inverse_projection() const { return inverse_projection; }

    void set_fov(Radiansf fov);
    void set_orthographic_bound(float bound);
    void set_camera_type(CameraType camera_type) { this->camera_type = camera_type; }
    void set_projection_type(ProjectionType projection_type);
    void set_target(Vector3f const& target) { this->target = target; }

    /// Computes the standard "free fly" view matrix.
    /// \param camera_transform Transform component of the camera.
    /// \return Reference to the computed view matrix.
    Matrix4 const& compute_view(Transform const& camera_transform);

    /// Computes the "look at" view matrix.
    /// \param position Position of the camera.
    /// \return Reference to the computed view matrix.
    Matrix4 const& compute_look_at(Vector3f const& position);

    /// Computes the inverse view matrix.
    /// \return Reference to the computed inverse view matrix.
    Matrix4 const& compute_inverse_view();

    /// Computes the perspective projection matrix.
    /// \return Reference to the computed perspective projection matrix.
    Matrix4 const& compute_perspective();

    /// Computes the orthographic projection matrix.
    /// \param min_x Minimum X limit of the projection frustum.
    /// \param max_x Maximum X limit of the projection frustum.
    /// \param min_y Minimum Y limit of the projection frustum.
    /// \param max_y Maximum Y limit of the projection frustum.
    /// \param min_z Minimum Z limit of the projection frustum.
    /// \param max_z Maximum Z limit of the projection frustum.
    /// \return Reference to the computed orthographic projection matrix.
    Matrix4 const& compute_orthographic(float min_x, float max_x, float min_y, float max_y, float min_z, float max_z);

    /// Computes the orthographic projection matrix using the camera's orthographic bounds.
    /// \see set_orthographic_bound()
    /// \return Reference to the computed orthographic projection matrix.
    Matrix4 const& compute_orthographic();

    /// Computes the projection matrix.
    /// Depending on the projection type, either perspective or orthographic will be computed.
    /// \return Reference to the computed projection matrix.
    Matrix4 const& compute_projection();

    /// Computes the inverse projection matrix.
    /// Depending on the projection type, either perspective or orthographic will be computed.
    /// \return Reference to the computed inverse projection matrix.
    Matrix4 const& compute_inverse_projection();

    /// Resizes the viewport.
    /// Resizing the viewport recomputes the projection matrix.
    /// \param frameWidth Viewport width.
    /// \param frameHeight Viewport height.
    void resize_viewport(Vector2ui const& frame_size);

    /// Unprojects to world space the given 3D point in homogeneous coordinates.
    /// \param point Point to unproject.
    /// \return Given point in world space.
    constexpr Vector3f unproject(Vector4f const& point) const
    {
        auto const view_mat = inverse_projection * point;
        Vector4f view_space_point{view_mat[0][0], view_mat[0][1], view_mat[0][2], view_mat[0][3]};
        view_space_point /= view_space_point.w;

        auto const result_mat = inverse_view * view_space_point;
        return Vector3f(result_mat[0][0], result_mat[0][1], result_mat[0][2]);
    }

    /// Unprojects to world space the given 3D point.
    /// \param point Point to unproject.
    /// \return Given point in world space.
    constexpr Vector3f unproject(Vector3f const& point) const { return unproject(Vector4f(point, 1.f)); }

    /// Unprojects to world space the given 2D coordinates.
    /// \param point Point to unproject. Its values are supposed to be between -1 & 1.
    /// \return Given point in world space.
    constexpr Vector3f unproject(Vector2f const& point) const { return unproject(Vector3f(point, 0.f)); }

private:
    float frame_ratio = 1.f;
    Radiansf fov = Degreesf(45.f);
    float near = 0.1f;
    float far = 1000.f;
    float ortho_bound = 1.f;

    CameraType camera_type = CameraType::FREE_FLY;
    ProjectionType projection_type = ProjectionType::PERSPECTIVE;

    Vector3f target = Vector3f(0.f);
    Vector3f up_axis = Vector3f::Up;

    Matrix4 view = Matrix4::Identity;
    Matrix4 inverse_view = Matrix4::Identity;
    Matrix4 projection = Matrix4::Identity;
    Matrix4 inverse_projection = Matrix4::Identity;
};
}