#include "Vertex.h"
#include "Buffer.h"
#include <stdexcept>

void VertexBuffer::createVertexBuffer(VkCommandPool commandPool, VkQueue graphicsQueue, 
	VkDevice device, VkPhysicalDevice physicalDevice, VkBuffer& vertexBuffer, 
	VkDeviceMemory& vertexBufferMemory, std::vector<Vertex>& vertices)
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



void IndexBuffer::createIndexBuffer(VkCommandPool commandPool, VkQueue graphicsQueue, 
	VkDevice device, VkPhysicalDevice physicalDevice, VkBuffer& indexBuffer, 
	VkDeviceMemory& indexBufferMemory, std::vector<uint32_t>& indices)
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