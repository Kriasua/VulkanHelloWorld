#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include<glm/glm.hpp>
#include<vector>

struct UniformBufferObject {
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
	float time;
};


class Descriptor
{
public:
	static void createDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout& descriptorSetLayout);
	static void updateUniformBuffer(VkDevice device, std::vector<VkDeviceMemory>& uniformBuffersMemory, VkExtent2D swapChainExtent, uint32_t currentImage);
	static void createUniformBuffers(VkDevice device, VkPhysicalDevice physicalDevice, const std::vector<VkImage>& swapChainImages, std::vector<VkBuffer>& uniformBuffers, std::vector<VkDeviceMemory>& uniformBuffersMemory);
	static void createDescriptorPool(VkDevice device, size_t swapChainImagesSize, VkDescriptorPool& descriptorPool);
	static void createDescriptorSets(VkDevice device, VkDescriptorSetLayout descriptorSetLayout, VkDescriptorPool descriptorPool, size_t swapChainImagesSize, std::vector<VkBuffer>& uniformBuffers, std::vector<VkDescriptorSet>& descriptorSets, VkImageView textureImageView, VkSampler textureSampler);
};