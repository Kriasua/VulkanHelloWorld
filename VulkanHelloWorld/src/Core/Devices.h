#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include<vector>
#include<optional>

struct QueueFamilyIndices
{
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;
	inline bool isComplete()
	{
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

struct SwapChainSupportDetails
{
	//VkSurfaceCapabilitiesKHR是一个结构体，里面包含了交换链的一些基本能力
	VkSurfaceCapabilitiesKHR capabilities;

	//VkSurfaceFormatKHR是一个结构体，里面包含了format和colorSpace
	std::vector<VkSurfaceFormatKHR> formats;

	//VkPresentModeKHR包含了交换链的显示模式，包括有无垂直同步等，极为重要
	std::vector<VkPresentModeKHR> presentModes;
};

class Devices
{
public:
	const static inline std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	Devices(GLFWwindow* window);
	~Devices();

	Devices(const Devices&) = delete;
	Devices& operator=(const Devices&) = delete;

	VkInstance getInstance() const { return m_instance; }
	VkPhysicalDevice getPhysicalDevice() const { return m_physicalDevice; }
	VkDevice getLogicalDevice() const { return m_logicalDevice; }
	VkQueue getGraphicsQueue() const { return m_graphicsQueue; }
	VkQueue getPresentQueue() const { return m_presentQueue; }
	VkCommandPool getCommandPool() const { return m_commandPool; }
	VkSurfaceKHR getSurface() const { return m_surface; }
	VkDebugUtilsMessengerEXT getDebugCallback() const { return m_callback; }
	SwapChainSupportDetails getSwapChainSupportDetails(VkPhysicalDevice device) { return querySwapChainSupport(m_physicalDevice); }
	QueueFamilyIndices getQueueFamilyIndices(VkPhysicalDevice device) { return findQueueFamilies(device); }


private:
	

	VkInstance m_instance;
	VkDebugUtilsMessengerEXT m_callback;
	VkSurfaceKHR m_surface; // Surface 必须放这里，因为选择物理设备时要检查是否支持显示
	VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
	VkDevice m_logicalDevice;
	VkQueue m_graphicsQueue;
	VkQueue m_presentQueue;
	VkCommandPool m_commandPool;

	void createInstance();
	void setupDebugCallback();
	void createSurface(GLFWwindow* window);
	void pickPhysicalDevice();
	void createLogicalDevice();
	void createCommandPool();
	bool isDeviceSuitable(VkPhysicalDevice device);
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
};