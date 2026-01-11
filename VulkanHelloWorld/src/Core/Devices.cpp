#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include "Devices.h"
#include "ValidationLayerAssist.h"
#include <set>

Devices::Devices(GLFWwindow* window)
{
	createInstance();
	setupDebugCallback();
	createSurface(window);
	pickPhysicalDevice();
	createLogicalDevice();
	createCommandPool();
}

void Devices::createInstance()
{
	if (enableValidationLayers && !ValidationLayerAssist::checkValidationLayerSupport())
	{
		throw std::runtime_error("validation layer check, but not AVAILABLE!");
	}

	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Hello-Triangle";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No-Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};

	if (enableValidationLayers)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(ValidationLayerAssist::validationLayers.size());
		createInfo.ppEnabledLayerNames = ValidationLayerAssist::validationLayers.data();
		ValidationLayerAssist::populateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
	}
	else
	{
		createInfo.enabledLayerCount = 0;
		createInfo.pNext = nullptr;
	}

	//经典两段式查询扩展
	//第一次查询先获取扩展数量，然后分配足够的空间存储给vector，第二次查询获取具体的扩展名字并打印
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	std::vector<VkExtensionProperties> extensionProperties(extensionCount + 1);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensionProperties.data());
	std::cout << "availble extensions:" << std::endl;
	for (const auto& extension : extensionProperties)
	{
		std::cout << "\t" << extension.extensionName << std::endl;
	}

	//GLFW扩展
	std::vector<const char*> glfwExtensions = ValidationLayerAssist::getRequiredExtensions();
	createInfo.enabledExtensionCount = static_cast<uint32_t>(glfwExtensions.size());
	createInfo.ppEnabledExtensionNames = glfwExtensions.data();


	if (vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS)
	{
		throw std::runtime_error("fail to create instance!");
	}
}

void Devices::setupDebugCallback()
{
	if (!enableValidationLayers)
	{
		return;
	}
	VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
	ValidationLayerAssist::populateDebugMessengerCreateInfo(createInfo);

	if (ValidationLayerAssist::CreateDebugUtilsMessengerEXT(m_instance, &createInfo, nullptr, &m_callback) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to set up debug callback!");
	}
}

void Devices::createSurface(GLFWwindow* window)
{
	if (glfwCreateWindowSurface(m_instance, window, nullptr, &m_surface) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create window surface!");
	}
}

void Devices::pickPhysicalDevice()
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);
	if (deviceCount == 0)
	{
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());
	for (const auto& device : devices)
	{
		if (isDeviceSuitable(device))
		{
			m_physicalDevice = device;
			break;
		}
	}

	if (m_physicalDevice == VK_NULL_HANDLE)
	{
		throw std::runtime_error("failed to find a suitable GPU!");
	}
}

void Devices::createLogicalDevice()
{
	QueueFamilyIndices indices = findQueueFamilies(this->m_physicalDevice);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

	//思考这里为什么要用set去重？
	std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };
	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures = {};
	deviceFeatures.samplerAnisotropy = VK_TRUE;

	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pEnabledFeatures = &deviceFeatures;
	createInfo.enabledExtensionCount = static_cast<uint32_t>(Devices::deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = Devices::deviceExtensions.data();
	if (enableValidationLayers)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(ValidationLayerAssist::validationLayers.size());
		createInfo.ppEnabledLayerNames = ValidationLayerAssist::validationLayers.data();
	}
	else
	{
		createInfo.enabledLayerCount = 0;
	}

	if (vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_logicalDevice) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create logical device!");
	}

	//这个队列又是依赖于逻辑设备的
	vkGetDeviceQueue(m_logicalDevice, indices.graphicsFamily.value(), 0, &m_graphicsQueue);
	vkGetDeviceQueue(m_logicalDevice, indices.presentFamily.value(), 0, &m_presentQueue);
}

void Devices::createCommandPool()
{
	QueueFamilyIndices queueFamilyIndices = findQueueFamilies(this->m_physicalDevice);
	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
	if (vkCreateCommandPool(m_logicalDevice, &poolInfo, nullptr, &m_commandPool) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create command pool!");
	}
}

bool Devices::isDeviceSuitable(VkPhysicalDevice device)
{
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);

	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	QueueFamilyIndices indices = findQueueFamilies(device);

	//检查该设备是否支持我们需要的设备扩展，比如交换链扩展
	//我们需要的设备扩展列表定义在“SwapChain.h”的deviceExtensions数组里面，目前只有交换链扩展
	//后续可能还会添加别的扩展，例如光线追踪相关的扩展
	bool extensionsSupported = checkDeviceExtensionSupport(device);

	bool swapChainAdequate = false;

	//只有在设备支持我们需要的扩展后，才去查询交换链的支持情况
	if (extensionsSupported)
	{
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}

	if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU
		&&
		deviceFeatures.geometryShader
		&&
		deviceFeatures.samplerAnisotropy
		&&
		indices.isComplete()
		&&
		swapChainAdequate)
	{
		std::cout << "device name: " << deviceProperties.deviceName << std::endl;
		return true;
	}

	return false;
}

QueueFamilyIndices Devices::findQueueFamilies(VkPhysicalDevice device)
{
	// 查找一个支持图形操作的队列族
	QueueFamilyIndices indices;
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const auto& queueFamily : queueFamilies)
	{
		//检查该队列族是否支持图形操作
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			indices.graphicsFamily = i;
		}

		//检查该队列族是否支持surface的present操作
		VkBool32 presentSupport = false;

		//询问在当前device的第i个队列族是否支持在指定的surface上进行present操作
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_surface, &presentSupport);

		if (queueFamily.queueCount > 0 && presentSupport)
		{
			indices.presentFamily = i;
		}

		if (indices.isComplete())
		{
			break;
		}

		i++;
	}
	return indices;
}

bool Devices::checkDeviceExtensionSupport(VkPhysicalDevice device)
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

SwapChainSupportDetails Devices::querySwapChainSupport(VkPhysicalDevice device)
{
	SwapChainSupportDetails details;
	//查询交换链的基本能力
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_surface, &details.capabilities);

	//查询交换链支持的像素格式和颜色空间
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, nullptr);
	if (formatCount != 0)
	{
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, details.formats.data());
	}

	//查询交换链支持的显示模式
	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount, nullptr);
	if (presentModeCount != 0)
	{
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount, details.presentModes.data());
	}
	return details;
}

Devices::~Devices()
{
	vkDestroyCommandPool(m_logicalDevice, m_commandPool, nullptr);
	vkDestroyDevice(m_logicalDevice, nullptr);
	if (enableValidationLayers)
	{
		ValidationLayerAssist::DestroyDebugUtilsMessengerEXT(m_instance, m_callback, nullptr);
	}
	vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
	vkDestroyInstance(m_instance, nullptr);
}

