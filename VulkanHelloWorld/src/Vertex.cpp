#include "Vertex.h"
#include "Buffer.h"
#include <stdexcept>

void VertexBuffer::createVertexBuffer(VkCommandPool commandPool, VkQueue graphicsQueue, VkDevice device, VkPhysicalDevice physicalDevice, VkBuffer& vertexBuffer, VkDeviceMemory& vertexBufferMemory)
{
	VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	//创建buffer
	Buffer::createBuffer(device, physicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, vertices.data(), (size_t)bufferSize);
	vkUnmapMemory(device, stagingBufferMemory);

	//创建真正的vertex buffer
	Buffer::createBuffer(device, physicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);

	Buffer::copyBuffer(device, commandPool, graphicsQueue, stagingBuffer, vertexBuffer, bufferSize);
	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);

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

//void VertexLayout::addAttribute(int countOfDimensional)
//{
//	VkVertexInputAttributeDescription attributeDescription{};
//	attributeDescription.binding = 0;
//	attributeDescription.location = m_Locationindex++;
//	switch (countOfDimensional)
//	{
//	case 1:
//		attributeDescription.format = VK_FORMAT_R32_SFLOAT;
//		break;
//	case 2:
//		attributeDescription.format = VK_FORMAT_R32G32_SFLOAT;
//		break;
//	case 3:
//		attributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
//		break;
//	case 4:
//		attributeDescription.format = VK_FORMAT_R32G32B32A32_SFLOAT;
//		break;
//	}
//	size_t size = sizeof(float) * countOfDimensional;
//	attributeDescription.offset = m_stride;
//	m_stride += size;
//	m_AttributeDescriptions.push_back(attributeDescription);
//}

void IndexBuffer::createIndexBuffer(VkCommandPool commandPool, VkQueue graphicsQueue, VkDevice device, VkPhysicalDevice physicalDevice, VkBuffer& indexBuffer, VkDeviceMemory& indexBufferMemory)
{
	VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	//创建buffer
	Buffer::createBuffer(device, physicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, indices.data(), (size_t)bufferSize);
	vkUnmapMemory(device, stagingBufferMemory);

	Buffer::createBuffer(device, physicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);
	Buffer::copyBuffer(device, commandPool, graphicsQueue, stagingBuffer, indexBuffer, bufferSize);
	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);
}