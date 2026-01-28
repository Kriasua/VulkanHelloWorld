#include "Swapchain.h"
#include "../Core/ValidationLayerAssist.h"

SwapChain::SwapChain(Devices& deviceRef, VkExtent2D windowExtent) : m_device(deviceRef), m_windowExtent(windowExtent)
{
	createSwapChain();
	createImageViews();
}

void SwapChain::createSwapChain()
{
	// --- 阶段 1: 协商 (委托给辅助类) ---
		// 查询物理设备对交换链的支持情况 (它能做什么？)
	SwapChainSupportDetails swapChainSupport = m_device.getSwapChainSupportDetails(m_device.getPhysicalDevice());

	// 根据查询到的结果，“协商”出我们想要的最佳规格
	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
	VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

	// --- 阶段 2: 决定图像数量 ---
	// 我们想要的数量是“最小支持数 + 1”（尝试开启三重缓冲）
	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

	//maxImageCount为0表示没有上限
	//或者我们想要的imageCount已经超过了硬件支持的上限，就必须让步
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
	{
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	// (保存这些协商好的结果，后面创建ImageView时会用到)
	m_swapChainImageFormat = surfaceFormat.format;
	m_swapChainExtent = extent;

	// --- 阶段 3: 填充 CreateInfo 结构体 (核心) ---
	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = m_device.getSurface();

	//只是请求或者建议，并不保证一定能得到这么多的image，有可能更多，所以在下面还要重新查询一次
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

	QueueFamilyIndices indices = m_device.getQueueFamilyIndices(m_device.getPhysicalDevice());
	std::vector<uint32_t> queueFamilyIndices = { indices.graphicsFamily.value(), indices.presentFamily.value() };

	if (indices.graphicsFamily != indices.presentFamily)
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices.data();
	}
	else
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;
	}

	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(m_device.getLogicalDevice(), &createInfo, nullptr, &m_swapChain) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create swap-chain!");
	}

	//再重新查询一次，得到实际的image数量
	vkGetSwapchainImagesKHR(m_device.getLogicalDevice(), m_swapChain, &imageCount, nullptr);
	m_swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(m_device.getLogicalDevice(), m_swapChain, &imageCount, m_swapChainImages.data());
}

void SwapChain::createImageViews()
{
	m_swapChainImageViews.resize(m_swapChainImages.size());

	//遍历交换链里的每一张图像，创建对应的ImageView
	for (size_t i = 0; i < m_swapChainImages.size(); i++)
	{
		m_swapChainImageViews[i] = createImageView(m_swapChainImages[i], m_swapChainImageFormat);
	}
}

VkImageView SwapChain::createImageView(VkImage image, VkFormat format)
{
	VkImageViewCreateInfo viewInfo = {};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = format;
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;
	VkImageView imageView;
	if (vkCreateImageView(m_device.getLogicalDevice(), &viewInfo, nullptr, &imageView) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create texture image view!");
	}
	return imageView;
}

void SwapChain::createFramebuffers(VkRenderPass renderPass)
{
	m_swapChainFramebuffers.resize(m_swapChainImageViews.size());
	for (size_t i = 0; i < m_swapChainImageViews.size(); i++)
	{
		VkImageView attachments[] = {
			m_swapChainImageViews[i]
		};
		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = m_swapChainExtent.width;
		framebufferInfo.height = m_swapChainExtent.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(m_device.getLogicalDevice(), &framebufferInfo, nullptr, &m_swapChainFramebuffers[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create framebuffer!");
		}
	}
}

VkSurfaceFormatKHR SwapChain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
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

VkPresentModeKHR SwapChain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
	if (!enableValidationLayers)
	{
		//首先寻找mailbox模式，这是一种低延迟且无撕裂的显示模式
		for (const auto& availablePresentMode : availablePresentModes)
		{
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				return availablePresentMode;
			}
		}
	}

	//如果找不到mailbox模式，就使用FIFO模式，这是一种强制垂直同步的显示模式
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D SwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
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
		width = m_windowExtent.width;
		height = m_windowExtent.height;
		VkExtent2D actualExtent = { static_cast<uint32_t>(width),static_cast<uint32_t>(height) };

		actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));

		actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));
		return actualExtent;
	}
}

SwapChain::~SwapChain()
{
	for (const auto& framebuffer : m_swapChainFramebuffers)
	{
		vkDestroyFramebuffer(m_device.getLogicalDevice(), framebuffer, nullptr);
	}

	// 1. 先销毁 ImageViews (因为它们依赖于 SwapChain Images)
	for (auto imageView : m_swapChainImageViews)
	{
		vkDestroyImageView(m_device.getLogicalDevice(), imageView, nullptr);
	}

	// 2. 最后销毁 SwapChain (Images 会随之自动销毁，不需要手动 destroy Image)
	if (m_swapChain != VK_NULL_HANDLE)
	{
		vkDestroySwapchainKHR(m_device.getLogicalDevice(), m_swapChain, nullptr);
	}
}
