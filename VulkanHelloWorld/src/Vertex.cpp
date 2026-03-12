#include "Vertex.h"
#include "Buffer.h"
#include <stdexcept>



VkVertexInputBindingDescription VertexLayout::getBindingDescription()
{
	VkVertexInputBindingDescription bindingDescription{};
	//绑定几号的顶点缓冲？
	bindingDescription.binding = 0;

	//一个顶点有多大？
	bindingDescription.stride = m_stride;
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	return bindingDescription;
}

std::vector<VkVertexInputAttributeDescription> VertexLayout::getAttributeDescriptions()
{
	return m_AttributeDescriptions;
}

