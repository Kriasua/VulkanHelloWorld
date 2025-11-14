#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include<vector>
#include<optional>

struct QueueFamilyIndices
{
	std::optional<uint32_t>(graphicsFamily);
	std::optional<uint32_t>(presentFamily);
	bool isComplete()
	{
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

class PhysicalAndLogicalDeviceAssis
{
public:
	static bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface);
	static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);

};
