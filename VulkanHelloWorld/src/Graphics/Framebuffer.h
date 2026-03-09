#pragma once
#include <vulkan/vulkan.h>
#include <vector>

class Framebuffer
{
public:
	Framebuffer(VkDevice device, VkRenderPass renderPass, VkExtent2D extent, const std::vector<VkImageView>& attachments);
	~Framebuffer();

	Framebuffer(const Framebuffer&) = delete;
	Framebuffer& operator=(const Framebuffer&) = delete;

	VkFramebuffer getHandle() const { return m_handle; }

private:
	VkDevice m_device;
	VkFramebuffer m_handle = VK_NULL_HANDLE;
};
