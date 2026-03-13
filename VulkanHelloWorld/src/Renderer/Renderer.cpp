#include "Renderer.h"
#include <stdexcept>

Renderer::Renderer(Devices& device, SwapChain& swapchain, const int maxFrame)
	:m_device(device), m_swapchain(swapchain), m_MAX_FRAMES_IN_FLIGHT(maxFrame)
{
	createSyncObjects();
}

Renderer::~Renderer()
{
	for (size_t i = 0; i < m_MAX_FRAMES_IN_FLIGHT; i++)
	{
		vkDestroySemaphore(m_device.getLogicalDevice(), m_renderFinishedSemaphores[i], nullptr);
		vkDestroySemaphore(m_device.getLogicalDevice(), m_imageAvailableSemaphores[i], nullptr);
		vkDestroyFence(m_device.getLogicalDevice(), m_inFlightFences[i], nullptr);
	}
}

void Renderer::createSyncObjects()
{
	size_t n = m_swapchain.getSwapChainImageViews().size();
	m_imageAvailableSemaphores.resize(n);
	m_renderFinishedSemaphores.resize(n);
	m_inFlightFences.resize(n);

	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	for (size_t i = 0; i < m_MAX_FRAMES_IN_FLIGHT; i++)
	{
		if (vkCreateSemaphore(m_device.getLogicalDevice(), &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore(m_device.getLogicalDevice(), &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create semaphores!");
		}
	}

	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	for (size_t i = 0; i < m_MAX_FRAMES_IN_FLIGHT; i++)
	{
		if (vkCreateFence(m_device.getLogicalDevice(), &fenceInfo, nullptr, &m_inFlightFences[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create fences!");
		}
	}
}

