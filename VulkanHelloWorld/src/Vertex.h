#pragma once
#include<glm/glm.hpp>
#include<vector>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLM_ENABLE_EXPERIMENTAL // 必须加这个宏才能用 glm 的 hash 功能
#include <glm/gtx/hash.hpp>


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
	template<typename T>
	void push()
	{
		static_assert(VertexAttributeTraits<T>::is_valid, "Unsupported vertex attribute type");
		VkVertexInputAttributeDescription attributeDescription{};
		//绑定到几号顶点缓冲区
		attributeDescription.binding = 0;

		//它是一个顶点中的第几个属性？
		attributeDescription.location = m_Locationindex++;
		attributeDescription.format = VertexAttributeTraits<T>::format;

		//它在一个顶点所有属性中的偏移量
		attributeDescription.offset = m_stride;
		m_stride += VertexAttributeTraits<T>::size;
		m_AttributeDescriptions.push_back(attributeDescription);
	}

private:
	uint32_t m_stride = 0;
	uint32_t m_Locationindex = 0;
	std::vector<VkVertexInputAttributeDescription> m_AttributeDescriptions;
};


//const std::vector<float> vertices = {
//
//	-0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
//
//	0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
//
//	0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
//
//	-0.5f, 0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f
//};
//
//const std::vector<uint16_t> indices = { 0,1,2,2,3,0 };

// 真正的 3D 顶点结构体
struct Vertex {
	glm::vec3 pos;      // 位置 (X, Y, Z)
	glm::vec3 color;    // 颜色 (R, G, B)
	glm::vec2 texCoord; // UV 贴图坐标 (U, V)
	glm::vec3 normal;   // 法线 (X, Y, Z) - 光照全靠它！

	// 重载 == 运算符：告诉 C++ 什么样的两个顶点算作“同一个”
	bool operator==(const Vertex& other) const {
		return pos == other.pos &&
			color == other.color &&
			texCoord == other.texCoord &&
			normal == other.normal;
	}
};

// 注入 std::hash：告诉哈希表如何为这个 Vertex 生成唯一的 ID (用于极速去重)
//标准库的hash没见过自定义的Vertex，所以用模板特化的方法告诉它如何辨别两个不一样的Vertex
//下面那些位运算不用管，会用就行
namespace std {
	template<> struct hash<Vertex> {
		size_t operator()(Vertex const& vertex) const {
			return ((hash<glm::vec3>()(vertex.pos) ^
				(hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
				(hash<glm::vec2>()(vertex.texCoord) << 1) ^
				(hash<glm::vec3>()(vertex.normal) << 1);
		}
	};
}


class VertexBuffer
{
public:
	static void createVertexBuffer(VkCommandPool commandPool, VkQueue graphicsQueue, VkDevice device, VkPhysicalDevice physicalDevice, VkBuffer& vertexBuffer, VkDeviceMemory& vertexBufferMemory, std::vector<Vertex>& vertices);
};