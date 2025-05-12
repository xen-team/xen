#pragma once

#include <component.hpp>
#include <math/angle.hpp>

namespace xen {
/// Transform class which handles 3D transformations (translation/rotation/scale).
class Transform final : public Component {
public:
    constexpr explicit Transform(
        Vector3f const& position = Vector3f(0.f), Quaternion const& rotation = Quaternion::Identity,
        Vector3f const& scale = Vector3f(1.f)
    ) : position_{position}, rotation_{rotation}, scale_{scale}
    {
    }

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
        Quaternion const z_quat(Vector3f::Front, z_angle);
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

private:
    Vector3f position_;
    Quaternion rotation_ = Quaternion::Identity;
    Vector3f scale_ = Vector3f(1.f);
    bool updated_ = true;
};
}