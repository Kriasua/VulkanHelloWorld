#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>
#include <stdexcept>
#include <functional>
#include <cstdlib>
#include <vector>
#include <unordered_set>

const int WIDTH = 800;
const int HEIGHT = 600;

//校验层，只要不是DEBUG模式就禁用校验层
const std::vector<const char*> validationLayers = {"VK_LAYER_KHRONOS_validation"};
#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

//因为vkCreateDebugUtilsMessengerEXT是一个扩展函数，它不是Vulkan 1.0核心的一部分，所以不能直接调用它。
//必须在运行时动态地从驱动程序中加载这个函数。
//所以这个函数的参数和vkCreateDebugUtilsMessengerEXT完全一样，只是多了一步获取函数指针的过程。
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pCallback)
{
	//询问，嘿！请问你知道vkCreateDebugUtilsMessengerEXT函数在哪里吗？
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	//找到了，就调用它
	if (func != nullptr)
	{
		return func(instance, pCreateInfo, pAllocator, pCallback);
	}
	else
	{
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT callback, const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr)
	{
		func(instance, callback, pAllocator);
	}
}


class HelloTriangleApplication
{
public:

	GLFWwindow* window;
	void run()
	{
		initWindow();
		initVulkan();
		mainLoop();
		cleanUp();
	}

private:

	VkInstance instance;
	VkDebugUtilsMessengerEXT callback;

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,VkDebugUtilsMessageTypeFlagsEXT messageType,const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
	{
		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
		std::cout << "pUserData: " << (const char*)pUserData << std::endl;
		return VK_FALSE;
	}

	std::vector<const char*> getRequiredExtensions()
	{
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		//接受一对迭代器，在这里是原始指针，把C风格数组const char**转换成std::vector<const char*>
		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
		if (enableValidationLayers)
		{
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}
		return extensions;
	}

	bool checkValidationLayerSupport()
	{
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
		std::cout << "available layers:" << std::endl;

		for (const char* layerName : validationLayers)
		{
			bool layerFound = false;
			
			for (const auto& layerProperties : availableLayers)
			{
				std::cout <<"\t" << layerProperties.layerName << std::endl;
				if (strcmp(layerName, layerProperties.layerName) == 0)
				{
					layerFound = true;
					break;
				}
			}
			if (!layerFound)
			{
				return false;
			}
		}

		std::cout << "\n";

		return true;
	}

	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
	{
		createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

		createInfo.pfnUserCallback = debugCallback;
		createInfo.pUserData = (char*)("我叼你妈的");
	}

	void createInstance()
	{
		if (enableValidationLayers && !checkValidationLayerSupport())
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
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
			populateDebugMessengerCreateInfo(debugCreateInfo);
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
		std::vector<VkExtensionProperties> extensionProperties(extensionCount+1);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensionProperties.data());
		std::cout << "availble extensions:" << std::endl;
		for (const auto& extension : extensionProperties)
		{
			std::cout <<"\t" << extension.extensionName << std::endl;
		}

		//GLFW扩展
		std::vector<const char*> glfwExtensions = getRequiredExtensions();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(glfwExtensions.size());
		createInfo.ppEnabledExtensionNames = glfwExtensions.data();
		createInfo.enabledLayerCount = 0;


		if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
		{
			throw std::runtime_error("fail to create instance!");
		}

	}



	void setupDebugCallback()
	{
		if (!enableValidationLayers)
		{
			return;
		}
		VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
		populateDebugMessengerCreateInfo(createInfo);

		if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &callback) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to set up debug callback!");
		}

	}

	void initWindow() {
		glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan",
			nullptr, nullptr);
	}

	void initVulkan()
	{
		createInstance();
		setupDebugCallback();
	}

	void mainLoop()
	{
		while (!glfwWindowShouldClose(window))
		{
			glfwPollEvents();
		}
	}

	void cleanUp()
	{
		if (enableValidationLayers)
		{
			DestroyDebugUtilsMessengerEXT(instance, callback, nullptr);
		}
		vkDestroyInstance(instance, nullptr);
		glfwDestroyWindow(window);
		glfwTerminate();
	}
};


int main()
{
	HelloTriangleApplication app;
	try
	{
		app.run();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
