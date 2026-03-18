#include "Entity.h"

Entity::Entity(std::shared_ptr<Model> model, std::shared_ptr<Material> material)
	: m_model(model), m_material(material)
{
	m_modelMatrix = glm::mat4(1.0f);
	m_position = glm::vec3(0.0f);
	m_rotation = glm::vec3(0.0f);
	m_scale = glm::vec3(1.0f);
}

Entity::~Entity()
{

}

glm::mat4 Entity::getModelMatrix()
{
	if (m_modified) {
		m_modelMatrix = glm::mat4(1.0f);
		m_modelMatrix = glm::translate(m_modelMatrix, m_position);
		m_modelMatrix = glm::rotate(m_modelMatrix, glm::radians(m_rotation.x), { 1, 0, 0 });
		m_modelMatrix = glm::rotate(m_modelMatrix, glm::radians(m_rotation.y), { 0, 1, 0 });
		m_modelMatrix = glm::rotate(m_modelMatrix, glm::radians(m_rotation.z), { 0, 0, 1 });
		m_modelMatrix = glm::scale(m_modelMatrix, m_scale);
		m_modified = false;
	}
	return m_modelMatrix;
}

void Entity::drawMain(VkCommandBuffer cmd, uint32_t currentFrame)
{
	glm::mat4 modelMat = getModelMatrix();
	m_material->bind(cmd, currentFrame);
	VkPipelineLayout pipelineLayout = m_material->getPipeline()->getPipelineLayout().getHandle();
	vkCmdPushConstants(cmd, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &modelMat);
	m_model->bind(cmd);
	m_model->draw(cmd);
}

void Entity::drawforShadow(VkCommandBuffer cmd, VkPipelineLayout shadowPipelineLayout)
{
	glm::mat4 modelMat = getModelMatrix();
	vkCmdPushConstants(cmd, shadowPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &modelMat);
	m_model->bind(cmd);
	m_model->draw(cmd);
}
