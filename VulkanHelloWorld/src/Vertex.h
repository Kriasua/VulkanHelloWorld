#pragma once
#include<glm/glm.hpp>
#include<vector>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


template<typename T>
struct VertexAttributeTraits {
	static const bool is_valid = false;
};

// 2. 特化：告诉编译器 float 对应什么
template<> struct VertexAttributeTraits<float> {
	static const bool is_valid = true;
	static const VkFormat format = VK_FORMAT_R32_SFLOAT;
	static const uint32_t size = sizeof(float);
};

// 3. 特化：glm::vec2
template<> struct VertexAttributeTraits<glm::vec2> {
	static const bool is_valid = true;
	static const VkFormat format = VK_FORMAT_R32G32_SFLOAT;
	static const uint32_t size = sizeof(glm::vec2);
};

// 4. 特化：glm::vec3
template<> struct VertexAttributeTraits<glm::vec3> {
	static const bool is_valid = true;
	static const VkFormat format = VK_FORMAT_R32G32B32_SFLOAT;
	static const uint32_t size = sizeof(glm::vec3);
};

// 5. 特化：glm::vec4
template<> struct VertexAttributeTraits<glm::vec4> {
	static const bool is_valid = true;
	static const VkFormat format = VK_FORMAT_R32G32B32A32_SFLOAT;
	static const uint32_t size = sizeof(glm::vec4);
};

class VertexLayout
{
public:
	VkVertexInputBindingDescription getBindingDescription();
	std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
	void addAttribute(int countOfDimensional);
	template<typename T>
	void push()
	{
		static_assert(VertexAttributeTraits<T>::is_valid, "Unsupported vertex attribute type");
		VkVertexInputAttributeDescription attributeDescription{};
		attributeDescription.binding = 0;
		attributeDescription.location = m_Locationindex++;
		attributeDescription.format = VertexAttributeTraits<T>::format;
		attributeDescription.offset = m_stride;
		m_stride += VertexAttributeTraits<T>::size;
		m_AttributeDescriptions.push_back(attributeDescription);
	}

private:
	uint32_t m_stride = 0;
	uint32_t m_Locationindex = 0;
	std::vector<VkVertexInputAttributeDescription> m_AttributeDescriptions;
};


const std::vector<float> vertices = {

	-0.5f, -0.5f, 1.0f, 0.0f, 0.0f,

	0.5f, -0.5f, 0.0f, 1.0f, 0.0f,

	0.5f, 0.5f, 0.0f, 0.0f, 1.0f,

	-0.5f, 0.5f, 1.0f, 1.0f, 1.0f
};

const std::vector<uint16_t> indices = { 0,1,2,2,3,0 };


class VertexBuffer
{
public:
	static void createVertexBuffer(VkCommandPool commandPool, VkQueue graphicsQueue, VkDevice device, VkPhysicalDevice physicalDevice, VkBuffer& vertexBuffer, VkDeviceMemory& vertexBufferMemory);
};