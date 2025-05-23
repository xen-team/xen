#pragma once

#include <component.hpp>
#include <math/angle.hpp>
#include <utility>

namespace xen {
/// Transform class which handles 3D transformations (translation/rotation/scale).
class Transform final : public Component {
public:
    constexpr Transform(
        Vector3f position = Vector3f(0.f), Quaternion rotation = Quaternion::Identity, Vector3f scale = Vector3f(1.f)
    ) : position_{std::move(position)}, rotation_{rotation}, scale_{std::move(scale)}
    {
    }

    constexpr Transform(Transform const& other) = default;
    constexpr Transform(Transform&& other) noexcept = default;

    constexpr Transform& operator=(Transform const& other) = default;
    constexpr Transform& operator=(Transform&& other) = default;

    [[nodiscard]] constexpr Vector3f const& get_position() const { return position_; }
    [[nodiscard]] constexpr Quaternion const& get_rotation() const { return rotation_; }
    [[nodiscard]] constexpr Vector3f const& get_scale() const { return scale_; }
    [[nodiscard]] constexpr bool has_updated() const { return updated_; }

    constexpr void set_position(Vector3f const& position)
    {
        position_ = position;
        updated_ = true;
    }

    constexpr void set_rotation(Quaternion const& rotation)
    {
        rotation_ = rotation;
        updated_ = true;
    }

    constexpr void set_scale(Vector3f const& scale)
    {
        scale_ = scale;
        updated_ = true;
    }

    constexpr void set_updated(bool updated) { updated_ = updated; }

    /// Moves by the given values in relative coordinates (takes rotation into account).
    /// \param displacement Displacement to be moved by.
    constexpr void move(Vector3f const& position) { translate(rotation_ * position); }

    /// Translates by the given values in absolute coordinates (does not take rotation into account).
    /// \param displacement Displacement to be translated by.
    constexpr void translate(Vector3f const& position)
    {
        position_ += position;
        updated_ = true;
    }

    /// Rotates by the given quaternion.
    /// \param rotation Rotation quaternion to rotate by.
    constexpr void rotate(Quaternion const& rotation)
    {
        rotation_ *= rotation;
        updated_ = true;
    }

    constexpr void rotate(Radiansf x_angle, Radiansf y_angle)
    {
        Quaternion const x_quat(Vector3f::Right, x_angle);
        Quaternion const y_quat(Vector3f::Up, y_angle);
        rotation_ = y_quat * rotation_ * x_quat;

        updated_ = true;
    }

    constexpr void rotate(Radiansf x_angle, Radiansf y_angle, Radiansf z_angle)
    {
        Quaternion const x_quat(Vector3f::Right, x_angle);
        Quaternion const y_quat(Vector3f::Right, y_angle);
        Quaternion const z_quat(Vector3f::Forward, z_angle);
        rotation_ *= z_quat * x_quat * y_quat;
        // rotation_ = rotation_.normalize();

        updated_ = true;
    }

    /// Scales by the given values.
    /// The scaling is a coefficient: scaling by a value of 2 doubles the size, while a value of 0.5 shrinks it by half.
    /// \param values Values to be scaled by.
    constexpr void scale(Vector3f const& scale)
    {
        scale_ *= scale;
        updated_ = true;
    }

    /// Computes the translation matrix (identity matrix with the translation in the last row).
    /// \param reverse_translation True if the translation should be reversed (negated), false otherwise.
    /// \return Translation matrix.
    [[nodiscard]] constexpr Matrix4 compute_translation(bool reverse_translation = false) const
    {
        Vector3f const translation = (reverse_translation ? -position_ : position_);

        Matrix4 const translation_matrix(
            {1.f, 0.f, 0.f, translation.x, 0.f, 1.f, 0.f, translation.y, 0.f, 0.f, 1.f, translation.z, 0.f, 0.f, 0.f,
             1.f}
        );

        return translation_matrix;
    }

    /// Computes the transformation matrix.
    /// This matrix combines all three features: translation, rotation & scale.
    /// \return Transformation matrix.
    [[nodiscard]] constexpr Matrix4 compute_transform() const
    {
        Matrix4 const scale_mat(
            {scale_.x, 0.f, 0.f, 0.f, 0.f, scale_.y, 0.f, 0.f, 0.f, 0.f, scale_.z, 0.f, 0.f, 0.f, 0.f, 1.f}
        );

        return compute_translation() * rotation_.to_rotation_matrix() * scale_mat;
    }

    /// Combines two transformations: parent_transform * local_transform.
    /// Applies the local transform relative to the parent's coordinate system.
    /// \param parent The parent's global transform.
    /// \param local The local transform relative to the parent.
    /// \return The resulting combined transform (child's global transform).
    [[nodiscard]] constexpr Transform operator*(Transform const& local) const
    {
        Vector3f combined_scale = get_scale() * local.get_scale();
        Quaternion combined_rotation = get_rotation() * local.get_rotation();
        Vector3f scaled_local_position = local.get_position() * get_scale();

        Vector3f rotated_scaled_local_position = get_rotation() * scaled_local_position;

        Vector3f combined_position = get_position() + rotated_scaled_local_position;

        return Transform(combined_position, combined_rotation, combined_scale);
    }

private:
    Vector3f position_;
    Quaternion rotation_ = Quaternion::Identity;
    Vector3f scale_ = Vector3f(1.f);
    bool updated_ = true;
};
}