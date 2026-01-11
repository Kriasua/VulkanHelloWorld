#pragma once
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
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

class Device
{
public:
	Device(GLFWwindow* window);
	~Device();

	Device(const Device&) = delete;
	Device& operator=(const Device&) = delete;


private:

};