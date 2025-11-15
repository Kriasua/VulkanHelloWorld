#include "SwapChain.h"
#include<set>
#include<string>
#include<algorithm>
bool SwapChainAssist::checkDeviceExtensionSupport(VkPhysicalDevice device)
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	//把需要的扩展放到一个set里面
	//因为set有自动去重的功能
	//这样我们就可以方便地删除已经找到的扩展
	//最后看看set是不是空的就知道所有需要的扩展都找到了没有
	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	for (const auto& extension : availableExtensions)
	{
		requiredExtensions.erase(extension.extensionName);
	}
	return requiredExtensions.empty();
}

SwapChainSupportDetails SwapChainAssist::querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
{
	SwapChainSupportDetails details;
	//查询交换链的基本能力
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

	//查询交换链支持的像素格式和颜色空间
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
	if (formatCount != 0)
	{
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
	}

	//查询交换链支持的显示模式
	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
	if (presentModeCount != 0)
	{
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
	}
	return details;
}

VkSurfaceFormatKHR SwapChainAssist::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	//如果交换链没有特别指定格式，就用我们想要的格式
	if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED)
	{
		return { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
	}
	//否则在可用的格式中寻找我们想要的格式
	for (const auto& availableFormat : availableFormats)
	{
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM &&
			availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return availableFormat;
		}
	}
	//如果找不到我们想要的格式，就返回第一个可用的格式
	return availableFormats[0];
}

VkPresentModeKHR SwapChainAssist::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
	//首先寻找mailbox模式，这是一种低延迟且无撕裂的显示模式
	for (const auto& availablePresentMode : availablePresentModes)
	{
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			return availablePresentMode;
		}
	}

	//如果找不到mailbox模式，就使用FIFO模式，这是一种强制垂直同步的显示模式
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D SwapChainAssist::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window)
{
	//如果交换链已经指定了分辨率，就直接使用
	//否则就使用窗口的分辨率，但要确保在交换链支持的范围内
	//UINT32_MAX是一个特殊值，表示交换链没有指定分辨率
	//我们需要自己决定分辨率
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		return capabilities.currentExtent;
	}
	else
	{
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);

		VkExtent2D actualExtent = {static_cast<uint32_t>(width),static_cast<uint32_t>(height)};

		actualExtent.width = std::max(capabilities.minImageExtent.width,std::min(capabilities.maxImageExtent.width, actualExtent.width));

		actualExtent.height = std::max(capabilities.minImageExtent.height,std::min(capabilities.maxImageExtent.height, actualExtent.height));
		return actualExtent;
	}
}