#include "PipelineBuilder.h"
#include <glm/glm.hpp>
#include <stdexcept>


PipelineBuilder::PipelineBuilder()
{
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	//false是标准做法。 true的话会把所有超出近远平面的片段都挤压为近平面上的值，通常用于阴影贴图，需要开启GPU特性
	rasterizer.depthClampEnable = VK_FALSE;
	//true时几何体处理完顶点之后，直接丢掉.当只想利用 GPU 的顶点着色器做数学计算（Transform Feedback），或者只想要深度图不需要颜色时用。
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	//三角形填充方式，这里是填满（标准做法）
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	//正面判定，三角形顶点按顺时针方向排列为正面
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f;
	rasterizer.depthBiasClamp = 0.0f;
	rasterizer.depthBiasSlopeFactor = 0.0f;

	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f;
	multisampling.pSampleMask = nullptr;
	multisampling.alphaToCoverageEnable = VK_FALSE;
	multisampling.alphaToOneEnable = VK_FALSE;

	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;

	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;

	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = static_cast<uint32_t>(m_dynamicStates.size());
	dynamicState.pDynamicStates = m_dynamicStates.data();

}



VkPipeline PipelineBuilder::build(VkDevice device, VkRenderPass pass)
{
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = shaderStages.size();
	pipelineInfo.pStages = shaderStages.data();
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = &depthStencil;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicState;
	pipelineInfo.layout = pipelineLayout;
	pipelineInfo.renderPass = pass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineInfo.basePipelineIndex = -1;
	VkPipeline pipieline;
	if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipieline) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create graphics pipeline!");
	}

	return pipieline;
}

void PipelineBuilder::setVertexInput(const VkVertexInputBindingDescription& binding, const std::vector<VkVertexInputAttributeDescription>& attributes)
{
	_bindingDescription = binding;
	_attributeDescriptions = attributes;

	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &_bindingDescription;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(_attributeDescriptions.size());
	vertexInputInfo.pVertexAttributeDescriptions = _attributeDescriptions.data();
}




void PipelineBuilder::enableDepthTest()
{
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;

	// 1. 开启深度测试：每次画像素时，都要拿当前的深度和深度缓冲里的值做比较
	depthStencil.depthTestEnable = VK_TRUE;

	// 2. 开启深度写入：如果测试通过了（它在最前面），就把它自己的深度写进缓冲里，挡住后面的
	depthStencil.depthWriteEnable = VK_TRUE;

	// 3. 比较运算符：VK_COMPARE_OP_LESS 表示“深度值更小（离相机更近）的优先”
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;

	// 4. 下面这些高级特性暂时不用，关掉
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.minDepthBounds = 0.0f; // Optional
	depthStencil.maxDepthBounds = 1.0f; // Optional
	depthStencil.stencilTestEnable = VK_FALSE; // 模板测试也先关着
}

PipelineLayout::PipelineLayout(const VkDevice device, const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts)
	:m_device(device)
{
	VkPushConstantRange pushConstantRange{};
	pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT; // 顶点阶段用
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof(glm::mat4); // 一个 mat4 是 64 字节

	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = descriptorSetLayouts.size();
	pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
	pipelineLayoutInfo.pushConstantRangeCount = 1;
	pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
	//pipelineLayoutInfo.pPushConstantRanges = nullptr;


	if (vkCreatePipelineLayout(m_device, &pipelineLayoutInfo, nullptr, &m_layout) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create pipeline layout!");
	}
}

PipelineLayout::~PipelineLayout()
{
	if (m_layout != VK_NULL_HANDLE) {
		vkDestroyPipelineLayout(m_device, m_layout, nullptr);
	}
}



Pipeline::Pipeline(VkDevice device, VkPipeline pipeline):
	m_device(device), m_graphicsPipeline(pipeline)
{

}

Pipeline::~Pipeline()
{
	if (m_graphicsPipeline != VK_NULL_HANDLE) {
		vkDestroyPipeline(m_device, m_graphicsPipeline, nullptr);
	}

	if (m_deslayout != VK_NULL_HANDLE) {
		vkDestroyDescriptorSetLayout(m_device, m_deslayout, nullptr);
	}
}
