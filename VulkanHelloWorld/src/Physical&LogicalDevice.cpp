//#include "Physical&m_device->getLogicalDevice().h"
#include "SwapChain.h"
#include <iostream>
#include "Core/Devices.h"


//bool PhysicalAndLogicalDeviceAssis::isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface)
//{
//	VkPhysicalDeviceProperties deviceProperties;
//	vkGetPhysicalDeviceProperties(device, &deviceProperties);
//
//	VkPhysicalDeviceFeatures deviceFeatures;
//	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
//
//	QueueFamilyIndices indices = findQueueFamilies(device, surface);
//
//	//检查该设备是否支持我们需要的设备扩展，比如交换链扩展
//	//我们需要的设备扩展列表定义在“SwapChain.h”的deviceExtensions数组里面，目前只有交换链扩展
//	//后续可能还会添加别的扩展，例如光线追踪相关的扩展
//	bool extensionsSupported = SwapChainAssist::checkDeviceExtensionSupport(device);
//
//	bool swapChainAdequate = false;
//
//	//只有在设备支持我们需要的扩展后，才去查询交换链的支持情况
//	if (extensionsSupported)
//	{
//		SwapChainSupportDetails swapChainSupport = SwapChainAssist::querySwapChainSupport(device, surface);
//		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
//	}
//
//	if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU
//		&&
//		deviceFeatures.geometryShader
//		&&
//		deviceFeatures.samplerAnisotropy
//		&&
//		indices.isComplete()
//		&&
//		swapChainAdequate)
//	{
//		std::cout << "device name: " << deviceProperties.deviceName << std::endl;
//		return true;
//	}
//
//	return false;
//}
//
//QueueFamilyIndices PhysicalAndLogicalDeviceAssis::findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface)
//{
//	// 查找一个支持图形操作的队列族
//	QueueFamilyIndices indices;
//	uint32_t queueFamilyCount = 0;
//	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
//	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
//	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
//
//	int i = 0;
//	for (const auto& queueFamily : queueFamilies)
//	{
//		//检查该队列族是否支持图形操作
//		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
//		{
//			indices.graphicsFamily = i;
//		}
//
//		//检查该队列族是否支持surface的present操作
//		VkBool32 presentSupport = false;
//
//		//询问在当前device的第i个队列族是否支持在指定的surface上进行present操作
//		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
//
//		if (queueFamily.queueCount > 0 && presentSupport)
//		{
//			indices.presentFamily = i;
//		}
//
//		if (indices.isComplete())
//		{
//			break;
//		}
//
//		i++;
//	}
//	return indices;
//}