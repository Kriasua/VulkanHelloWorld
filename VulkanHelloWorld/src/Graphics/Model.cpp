#include "Model.h"
#include <tiny_obj_loader.h>
#include <stdexcept>
#include "../Buffer.h"

Model::Model(Devices& device, const std::string path) : m_device(device)
{
	this->loadModel(path);
	createVertexBuffer();
	createIndexBuffer();
	m_vertices.clear();
	m_vertices.shrink_to_fit();
	m_indexCount = static_cast<uint32_t>(m_indices.size());
	m_indices.clear();
	m_indices.shrink_to_fit();
}

void Model::bind(VkCommandBuffer cmdbuff)
{
	std::vector<VkBuffer> vertexbuffers = { m_vertexBuffer };
	std::vector<VkDeviceSize> offsets = { 0 };

	vkCmdBindVertexBuffers(cmdbuff, 0, 1, vertexbuffers.data(), offsets.data());

	vkCmdBindIndexBuffer(cmdbuff, m_indexBuffer, 0, VK_INDEX_TYPE_UINT32);
}

void Model::draw(VkCommandBuffer cmdbuff)
{
	vkCmdDrawIndexed(cmdbuff, m_indexCount, 1, 0, 0, 0);
}

void Model::loadModel(const std::string path)
{
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn, err;

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str())) {
		throw std::runtime_error(warn + err);
	}

	std::unordered_map<Vertex, uint32_t> uniqueVertices{};

	for (const auto& shape : shapes) {
		for (const auto& index : shape.mesh.indices) {
			Vertex vertex{};

			// 1. 抓取位置 (XYZ)
			vertex.pos = {
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2]
			};

			// 2. 抓取 UV 坐标 (注意：Vulkan 的 V 轴和 OBJ 格式是上下颠倒的！)
			if (index.texcoord_index >= 0) {
				vertex.texCoord = {
					attrib.texcoords[2 * index.texcoord_index + 0],
					1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
				};
			}

			// 3. 抓取法线 
			if (index.normal_index >= 0) {
				vertex.normal = {
					attrib.normals[3 * index.normal_index + 0],
					attrib.normals[3 * index.normal_index + 1],
					attrib.normals[3 * index.normal_index + 2]
				};
			}

			// 4. 颜色 
			vertex.color = { 1.0f, 1.0f, 1.0f };

			// 5. 哈希去重校验
			if (uniqueVertices.count(vertex) == 0) {
				uniqueVertices[vertex] = static_cast<uint32_t>(m_vertices.size());
				m_vertices.push_back(vertex);
			}

			m_indices.push_back(uniqueVertices[vertex]);
		}
	}
}

void Model::createVertexBuffer()
{
	VkDeviceSize bufferSize = sizeof(m_vertices[0]) * m_vertices.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	//创建buffer
	Buffer::createBuffer(m_device.getLogicalDevice(), m_device.getPhysicalDevice(), bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(m_device.getLogicalDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, m_vertices.data(), (size_t)bufferSize);
	vkUnmapMemory(m_device.getLogicalDevice(), stagingBufferMemory);

	//创建真正的vertex buffer
	Buffer::createBuffer(m_device.getLogicalDevice(), m_device.getPhysicalDevice(),bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_vertexBuffer, m_vertexBufferMemory);

	Buffer::copyBuffer(m_device.getLogicalDevice(), m_device.getCommandPool(), m_device.getGraphicsQueue(), stagingBuffer, m_vertexBuffer, bufferSize);
	vkDestroyBuffer(m_device.getLogicalDevice(), stagingBuffer, nullptr);
	vkFreeMemory(m_device.getLogicalDevice(), stagingBufferMemory, nullptr);
}

void Model::createIndexBuffer()
{
	VkDeviceSize bufferSize = sizeof(m_indices[0]) * m_indices.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	//创建buffer
	Buffer::createBuffer(m_device.getLogicalDevice(), m_device.getPhysicalDevice(), bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(m_device.getLogicalDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, m_indices.data(), (size_t)bufferSize);
	vkUnmapMemory(m_device.getLogicalDevice(), stagingBufferMemory);

	Buffer::createBuffer(m_device.getLogicalDevice(), m_device.getPhysicalDevice(), bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_indexBuffer, m_indexBufferMemory);
	Buffer::copyBuffer(m_device.getLogicalDevice(), m_device.getCommandPool(), m_device.getGraphicsQueue(), stagingBuffer, m_indexBuffer, bufferSize);
	vkDestroyBuffer(m_device.getLogicalDevice(), stagingBuffer, nullptr);
	vkFreeMemory(m_device.getLogicalDevice(), stagingBufferMemory, nullptr);
}


Model::~Model()
{
	if (m_indexBuffer != VK_NULL_HANDLE)
	{
		vkDestroyBuffer(m_device.getLogicalDevice(), m_indexBuffer, nullptr);
	}

	if (m_indexBufferMemory != VK_NULL_HANDLE)
	{
		vkFreeMemory(m_device.getLogicalDevice(), m_indexBufferMemory, nullptr);
	}

	if (m_vertexBuffer != VK_NULL_HANDLE)
	{
		vkDestroyBuffer(m_device.getLogicalDevice(), m_vertexBuffer, nullptr);
	}

	if (m_vertexBufferMemory != VK_NULL_HANDLE)
	{
		vkFreeMemory(m_device.getLogicalDevice(), m_vertexBufferMemory, nullptr);
	}
}
