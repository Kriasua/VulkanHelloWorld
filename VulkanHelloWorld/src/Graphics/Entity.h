#pragma once
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Model.h"
#include "Material.h"

class Entity
{
public:
	Entity(std::shared_ptr<Model> model, std::shared_ptr<Material> material);
	~Entity();

	void setPosition(glm::vec3 position) { m_position = position; m_modified = true; }
	void setRotation(glm::vec3 rotation) { m_rotation = rotation; m_modified = true;}
	void setScale(glm::vec3 scale) { m_scale = scale; m_modified = true;}

	glm::mat4 getModelMatrix();
	void draw(VkCommandBuffer cmd, uint32_t currentFrame);

private:
	std::shared_ptr<Model> m_model;
	std::shared_ptr<Material> m_material;

	glm::vec3 m_position;
	glm::vec3 m_rotation;
	glm::vec3 m_scale;

	glm::mat4 m_modelMatrix;
	bool m_modified = true;
};
