#pragma once
#include "../Vertex.h"
#include "../Core/Devices.h"
#include <vector>
#include <string>
#include <vulkan/vulkan.h>

class Model
{
public:
	Model(Devices& device, const std::string path);
	~Model();

	Model(const Model&) = delete;
	Model& operator=(const Model&) = delete;

	void bind(VkCommandBuffer cmdbuff);
	void draw(VkCommandBuffer cmdbuff);

private:
	std::vector<Vertex> m_vertices;
	std::vector<uint32_t> m_indices;
	uint32_t m_indexCount;
	Devices& m_device;

	VkBuffer m_vertexBuffer = VK_NULL_HANDLE;
	VkDeviceMemory m_vertexBufferMemory = VK_NULL_HANDLE;

	VkBuffer m_indexBuffer = VK_NULL_HANDLE;
	VkDeviceMemory m_indexBufferMemory = VK_NULL_HANDLE;

	void loadModel(const std::string path);
	void createVertexBuffer();
	void createIndexBuffer();

};
