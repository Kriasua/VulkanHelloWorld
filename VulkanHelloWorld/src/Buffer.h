#pragma once
#include<glm/glm.hpp>
#include<vector>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include<array>

class Buffer
{
public:
	static void createBuffer(VkDevice device, VkPhysicalDevice physicalDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	static void copyBuffer(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	static uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);
	static void copyBufferToImage(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
};

class IndexBuffer
{
public:
	static void createIndexBuffer(VkCommandPool commandPool, VkQueue graphicsQueue, VkDevice device, VkPhysicalDevice physicalDevice, VkBuffer& vertexBuffer, VkDeviceMemory& vertexBufferMemory);
};

class CommandBuffer
{
public:
	static VkCommandBuffer beginSingleTimeCommands(VkDevice device, VkCommandPool commandPool);
	static void endSingleTimeCommands(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, VkCommandBuffer commandBuffer);
};



