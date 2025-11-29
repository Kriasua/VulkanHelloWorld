#include "VertexBuffer.h"
#include <stdexcept>


void VertexBuffer::createVertexBuffer(VkDevice device, VkPhysicalDevice physicalDevice, VkBuffer& vertexBuffer, VkDeviceMemory& vertexBufferMemory)
{
	//创建vertexbuffer
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = sizeof(vertices[0]) * vertices.size();
	bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	if (vkCreateBuffer(device, &bufferInfo, nullptr, &vertexBuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create vertex buffer!");
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(device, vertexBuffer, &memRequirements);


	//分配内存
	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;

	uint32_t memoryTypeIndex = VertexBuffer::findMemoryType(physicalDevice, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	allocInfo.memoryTypeIndex = memoryTypeIndex;
	if (vkAllocateMemory(device, &allocInfo, nullptr, &vertexBufferMemory) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate vertex buffer memory!");
	}

	//把刚才分配的内存绑定到vertex buffer上
	vkBindBufferMemory(device, vertexBuffer, vertexBufferMemory, 0);

	void* data;
	vkMapMemory(device, vertexBufferMemory, 0, bufferInfo.size, 0, &data);
	memcpy(data, vertices.data(), (size_t)bufferInfo.size);
	vkUnmapMemory(device, vertexBufferMemory);
}

//寻找符合条件的内存类型，因为显存有很多不同的类型
uint32_t VertexBuffer::findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
	{
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
		{
			return i;
		}
	}
	throw std::runtime_error("failed to find suitable memory type!");
}

VkVertexInputBindingDescription VertexLayout::getBindingDescription()
{
	VkVertexInputBindingDescription bindingDescription{};
	bindingDescription.binding = 0;
	bindingDescription.stride = m_stride;
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	return bindingDescription;
}

std::vector<VkVertexInputAttributeDescription> VertexLayout::getAttributeDescriptions()
{
	return m_AttributeDescriptions;
}

void VertexLayout::addAttribute(int countOfDimensional)
{
	VkVertexInputAttributeDescription attributeDescription{};
	attributeDescription.binding = 0;
	attributeDescription.location = m_Locationindex++;
	switch (countOfDimensional)
	{
	case 1:
		attributeDescription.format = VK_FORMAT_R32_SFLOAT;
		break;
	case 2:
		attributeDescription.format = VK_FORMAT_R32G32_SFLOAT;
		break;
	case 3:
		attributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
		break;
	case 4:
		attributeDescription.format = VK_FORMAT_R32G32B32A32_SFLOAT;
		break;
	}
	size_t size = sizeof(float) * countOfDimensional;
	attributeDescription.offset = m_stride;
	m_stride += size;
	m_AttributeDescriptions.push_back(attributeDescription);
}
