#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include<vector>

const std::vector<const char*> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

struct SwapChainSupportDetails
{
	//VkSurfaceCapabilitiesKHR是一个结构体，里面包含了交换链的一些基本能力
	VkSurfaceCapabilitiesKHR capabilities; 

	//VkSurfaceFormatKHR是一个结构体，里面包含了format和colorSpace
	std::vector<VkSurfaceFormatKHR> formats; 

	//VkPresentModeKHR包含了交换链的显示模式，包括有无垂直同步等，极为重要
	std::vector<VkPresentModeKHR> presentModes; 
};

class SwapChainAssist
{
public:
	static bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	static SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
	static VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	static VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	static VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window);
};
