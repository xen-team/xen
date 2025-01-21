#pragma once
#include "frustum.hpp"
#include "model.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <memory>

class Entity {
public:
	std::string name;
    Transform transform;

    std::vector<std::unique_ptr<Entity>> children;
    Entity* parent = nullptr;

	Model* pModel = nullptr;
	Shader* shader = nullptr;

	std::unique_ptr<AABB> boundingVolume;

	Entity(Model& model, Shader& nshader) : pModel{ &model }, shader{ &nshader }
	{
		static int id = 0;
		id++;
		name = "entity#" + std::to_string(id);
		boundingVolume = std::make_unique<AABB>(generateAABB(model));

		//boundingVolume = std::make_unique<Sphere>(generateSphereBV(model));
	}

	AABB getGlobalAABB()
	{
		//Get global scale thanks to our transform
		const glm::vec3 globalCenter{ transform.getModelMatrix() * glm::vec4(boundingVolume->center, 1.f) };

		// Scaled orientation
		const glm::vec3 right = transform.getRight() * boundingVolume->extents.x;
		const glm::vec3 up = transform.getUp() * boundingVolume->extents.y;
		const glm::vec3 forward = transform.getForward() * boundingVolume->extents.z;

		const float newIi = std::abs(glm::dot(glm::vec3{ 1.f, 0.f, 0.f }, right)) +
			std::abs(glm::dot(glm::vec3{ 1.f, 0.f, 0.f }, up)) +
			std::abs(glm::dot(glm::vec3{ 1.f, 0.f, 0.f }, forward));

		const float newIj = std::abs(glm::dot(glm::vec3{ 0.f, 1.f, 0.f }, right)) +
			std::abs(glm::dot(glm::vec3{ 0.f, 1.f, 0.f }, up)) +
			std::abs(glm::dot(glm::vec3{ 0.f, 1.f, 0.f }, forward));

		const float newIk = std::abs(glm::dot(glm::vec3{ 0.f, 0.f, 1.f }, right)) +
			std::abs(glm::dot(glm::vec3{ 0.f, 0.f, 1.f }, up)) +
			std::abs(glm::dot(glm::vec3{ 0.f, 0.f, 1.f }, forward));

		return AABB(globalCenter, newIi, newIj, newIk);
	}

    template<typename... Args>
    void add_child(Args&... args)
    {
        children.emplace_back(std::make_unique<Entity>(args...));
        children.back()->parent = this;
    }

    template<typename... Args>
    void add_child(Args&&... args)
    {
        children.emplace_back(std::make_unique<Entity>(std::forward<Args>(args)...));
        children.back()->parent = this;
    }
        
    void update_tree()
    {
        if (transform.is_dirty())
        {
            force_update_tree();
            return;
        }

        for (auto&& child : children)
        {
            child->update_tree();
        }
    }

    void force_update_tree()
    {
        if (parent)
            transform.computeModelMatrix(parent->transform.getModelMatrix());
        else
            transform.computeModelMatrix();

        for (auto&& child : children)
        {
            child->force_update_tree();
        }
    }

	void draw(Frustum const& frustum) {
		if (boundingVolume->isOnFrustum(frustum, transform))
		{
			shader->use();
			shader->setMat4("model", transform.getModelMatrix());
			pModel->Draw(*shader);
		}
	}

	void draw_tree(Frustum const& frustum)
	{
		draw(frustum);

		for (auto&& child : children)
		{
			child->draw(frustum);
		}
	}
}; 