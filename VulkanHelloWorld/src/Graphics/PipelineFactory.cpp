#include "PipelineFactory.h"
#include "Shader.h"
#include "../Vertex.h"
#include <stdexcept>

std::shared_ptr<Pipeline> PipelineFactory::createStandardPipeline(Devices& device, VkRenderPass renderPass, VkExtent2D extent, VkDescriptorSetLayout descripLayout)
{
	/////////////////////////////////////////////////////////////////////////////////////
		////////////////////// 图形管线可编程阶段的配置(shaders) /////////////////////////////
		/////////////////////////////////////////////////////////////////////////////////////

	Shader vertShader(device.getLogicalDevice(), "shader/vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
	Shader fragShader(device.getLogicalDevice(), "shader/frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
	std::vector<VkPipelineShaderStageCreateInfo> shaderStages = { vertShader.getStageInfo(), fragShader.getStageInfo() };

	/////////////////////////////////////////////////////////////////////////////////////
	////////////////////// 图形管线其他阶段的配置 (固定功能阶段) /////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////


	VertexLayout layout;
	layout.push<glm::vec3>();//位置
	layout.push<glm::vec3>();//颜色
	layout.push<glm::vec2>();//UV
	layout.push<glm::vec3>();//法线

	PipelineBuilder builder;
	builder.shaderStages.push_back(vertShader.getStageInfo());
	builder.shaderStages.push_back(fragShader.getStageInfo());
	builder.setVertexInput(layout.getBindingDescription(), layout.getAttributeDescriptions());
	builder.viewport = { 0.0f,0.0f,(float)extent.width ,(float)extent.height ,0.0f,1.0f };
	builder.scissor = { {0,0}, extent };
	builder.enableDepthTest();

	std::vector<VkDescriptorSetLayout> layouts = { descripLayout };
	auto pipelineLayout = std::make_unique<PipelineLayout>(device.getLogicalDevice(), layouts);
	builder.setPipelineLayout(pipelineLayout->getHandle());

	VkPipeline rawPipeline = builder.build(device.getLogicalDevice(), renderPass);
	if (rawPipeline == VK_NULL_HANDLE) {
		throw std::runtime_error("Failed to create graphics pipeline!");
	}
	std::shared_ptr<Pipeline> pipeline = std::make_shared<Pipeline>(device.getLogicalDevice(), rawPipeline);
	pipeline->setPipelineLayout(std::move(pipelineLayout));
	pipeline->setDescriptorSetLayout(descripLayout);
	return pipeline;
}

