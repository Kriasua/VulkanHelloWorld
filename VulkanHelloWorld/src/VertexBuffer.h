#pragma once
#include<glm/glm.hpp>
#include<vector>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include<array>


class VertexLayout
{
public:
	VkVertexInputBindingDescription getBindingDescription();
	std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
	void addAttribute(int countOfDimensional);
private:
	uint32_t m_stride = 0;
	uint32_t m_Locationindex = 0;
	std::vector<VkVertexInputAttributeDescription> m_AttributeDescriptions;
};


const std::vector<float> vertices = {
	// 顶点 0: 顶部, 红色
	0.0f, -0.5f, 1.0f, 1.0f, 0.0f,
	// 顶点 1: 右下, 绿色
	0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
	// 顶点 2: 左下, 蓝色
	-0.5f, 0.5f, 0.0f, 0.0f, 1.0f
};

class VertexBuffer
{
public:
	static void createVertexBuffer(VkDevice device, VkPhysicalDevice physicalDevice, VkBuffer& vertexBuffer, VkDeviceMemory& vertexBufferMemory);
	static uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);
};



