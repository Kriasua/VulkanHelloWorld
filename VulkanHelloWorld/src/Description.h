#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include<glm/glm.hpp>
#include<vector>
#include "Graphics/Camera.h"


class Descriptor
{
public:
	static VkDescriptorSetLayout createDescriptorSetLayout(VkDevice device);
	static VkDescriptorSetLayout createShadowDescriptorSetLayout(VkDevice device);
};