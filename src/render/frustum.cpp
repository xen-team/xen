#include "frustum.hpp"

Frustum createFrustumFromCamera(const PerspectiveCamera& cam)
{
	Frustum     frustum;
	const float halfVSide = cam.far * tanf(cam.fov * .5f);
	const float halfHSide = halfVSide * cam.aspect_ratio;
	const glm::vec3 frontMultFar = cam.far * cam.direction;

	frustum.nearFace = { cam.pos + cam.near * cam.direction, cam.direction };
	frustum.farFace = { cam.pos + frontMultFar, -cam.direction };
	frustum.rightFace = { cam.pos, glm::cross(frontMultFar - cam.right() * halfHSide, cam.up) };
	frustum.leftFace = { cam.pos, glm::cross(cam.up, frontMultFar + cam.right() * halfHSide) };
	frustum.topFace = { cam.pos, glm::cross(cam.right(), frontMultFar - cam.up * halfVSide) };
	frustum.bottomFace = { cam.pos, glm::cross(frontMultFar + cam.up * halfVSide, cam.right()) };
	return frustum;
}

AABB generateAABB(const Model& model)
{
	glm::vec3 minAABB = glm::vec3(std::numeric_limits<float>::max());
	glm::vec3 maxAABB = glm::vec3(std::numeric_limits<float>::min());
	for (auto&& mesh : model.meshes)
	{
		for (auto&& vertex : mesh.vertices)
		{
			minAABB.x = std::min(minAABB.x, vertex.Position.x);
			minAABB.y = std::min(minAABB.y, vertex.Position.y);
			minAABB.z = std::min(minAABB.z, vertex.Position.z);

			maxAABB.x = std::max(maxAABB.x, vertex.Position.x);
			maxAABB.y = std::max(maxAABB.y, vertex.Position.y);
			maxAABB.z = std::max(maxAABB.z, vertex.Position.z);
		}
	}
	return AABB(minAABB, maxAABB);
}

Sphere generateSphereBV(const Model& model)
{
	glm::vec3 minAABB = glm::vec3(std::numeric_limits<float>::max());
	glm::vec3 maxAABB = glm::vec3(std::numeric_limits<float>::min());
	for (auto&& mesh : model.meshes)
	{
		for (auto&& vertex : mesh.vertices)
		{
			minAABB.x = std::min(minAABB.x, vertex.Position.x);
			minAABB.y = std::min(minAABB.y, vertex.Position.y);
			minAABB.z = std::min(minAABB.z, vertex.Position.z);

			maxAABB.x = std::max(maxAABB.x, vertex.Position.x);
			maxAABB.y = std::max(maxAABB.y, vertex.Position.y);
			maxAABB.z = std::max(maxAABB.z, vertex.Position.z);
		}
	}

	return Sphere((maxAABB + minAABB) * 0.5f, glm::length(minAABB - maxAABB));
}