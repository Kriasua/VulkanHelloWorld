#pragma once
#include <vulkan/vulkan.h>
#include <vector>
class shaderManager
{
public:
	static VkShaderModule createShaderModule(const VkDevice& device, const std::vector<char>& code);
};