#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include "../Core/Devices.h"
#include "../Graphics/Swapchain.h"

class Renderer
{
public:
	Renderer(Devices& device, SwapChain& swapchain, const int maxFrame);
	~Renderer();
	Renderer(const Renderer&) = delete;
	Renderer& operator=(const Renderer&) = delete;

private:
	Devices& m_device;
	SwapChain& m_swapchain;
	const int m_MAX_FRAMES_IN_FLIGHT;
	std::vector<VkSemaphore> m_imageAvailableSemaphores;
	std::vector<VkSemaphore> m_renderFinishedSemaphores;
	std::vector<VkFence> m_inFlightFences;

	void createSyncObjects();
};
