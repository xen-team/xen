// #include "transform.hpp"

// // #define GLM_ENABLE_EXPERIMENTAL

// // #include <glm/glm.hpp>
// // #include <glm/gtx/quaternion.hpp>
// // #include <glm/gtc/matrix_transform.hpp>
// // #include <glm/gtx/matrix_decompose.hpp>

// namespace xen {
// // constexpr glm::mat4 TransformComponent::get_model_matrix() const
// // {
// //     glm::mat4 const trans = glm::translate(glm::mat4(1.f), translation);
// //     glm::mat4 const rot = glm::toMat4(rotation);
// //     glm::mat4 const scl = glm::scale(glm::mat4(1.f), scale);

// //     return trans * rot * scl;
// // }

// // constexpr void TransformComponent::set_model_matrix(glm::mat4 model)
// // {
// //     glm::vec3 skew;
// //     glm::vec4 perspective;
// //     glm::decompose(model, scale, rotation, translation, skew, perspective);
// // }

// constexpr Vector3f Transform::right() const
// {
//     return rotation * Vector3f(1.f, 0.f, 0.f);
// }

// constexpr Vector3f Transform::forward() const
// {
//     return rotation * Vector3f(0.f, 0.f, 1.f);
// }

// constexpr Vector3f Transform::up() const
// {
//     return rotation * Vector3f(0.f, 1.f, 0.f);
// }

// // constexpr glm::vec3 TransformComponent::get_euler_rotation() const
// // {
// //     // PROFILE();

// //     return glm::degrees(glm::eulerAngles(rotation));
// // }

// // constexpr void TransformComponent::set_euler_rotation(glm::vec3 eulerRotation)
// // {
// //     // PROFILE();

// //     rotation = glm::radians(eulerRotation);
// // }

// constexpr void Transform::look_at(Vector3f const& at)
// {
//     Matrix4 view_matrix = Matrix4::look_at(position, at, Vector3f(0.0f, 1.0f, 0.0f));
//     Quaternion quaternion = from_rotation_matrix(view_matrix);
//     rotation = quaternion.to_euler();
// }
// }