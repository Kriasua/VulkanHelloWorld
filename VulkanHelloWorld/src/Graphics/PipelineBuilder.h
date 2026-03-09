#pragma once
#include<vector>
#include <vulkan/vulkan.h>

class PipelineBuilder
{
public:
	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
	VkPipelineVertexInputStateCreateInfo vertexInputInfo;
	VkPipelineInputAssemblyStateCreateInfo inputAssembly;
	VkViewport viewport;
	VkRect2D scissor;
	VkPipelineViewportStateCreateInfo viewportState;
	VkPipelineRasterizationStateCreateInfo rasterizer;
	VkPipelineMultisampleStateCreateInfo multisampling;
	VkPipelineColorBlendAttachmentState colorBlendAttachment;
	VkPipelineColorBlendStateCreateInfo colorBlending;
	VkPipelineDynamicStateCreateInfo dynamicState;
	VkGraphicsPipelineCreateInfo pipelineInfo;
	VkPipelineDepthStencilStateCreateInfo depthStencil;
	VkPipelineLayout pipelineLayout;
	VkVertexInputBindingDescription _bindingDescription{};
	std::vector<VkVertexInputAttributeDescription> _attributeDescriptions{};

	//////////////////////////////////////////////////////////////////////////////

	PipelineBuilder();
	VkPipeline build(VkDevice device, VkRenderPass pass);
	void setVertexInput(const VkVertexInputBindingDescription& binding,
		const std::vector<VkVertexInputAttributeDescription>& attributes);
	PipelineBuilder& setPipelineLayout(VkPipelineLayout layout)
	{
		this->pipelineLayout = layout;
		return *this;
	}

private:
	VkExtent2D m_SwapChainExtent;
};

class PipelineLayout
{
public:
	PipelineLayout(const PipelineLayout&) = delete;
	PipelineLayout& operator=(const PipelineLayout&) = delete;

	PipelineLayout(const VkDevice device, const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts);
	~PipelineLayout();
	const VkPipelineLayout& getHandle() const { return m_layout; }
private:
	VkDevice m_device;
	VkPipelineLayout m_layout = VK_NULL_HANDLE;
};

class Pipeline
{
public:
	Pipeline(const Pipeline&) = delete;
	Pipeline& operator=(const Pipeline&) = delete;

	Pipeline(VkDevice device, VkPipeline pipeline);
	~Pipeline();

	VkPipeline& getPipeline() { return m_graphicsPipeline; }

private:
	VkPipeline m_graphicsPipeline;
	VkDevice m_device;
};
