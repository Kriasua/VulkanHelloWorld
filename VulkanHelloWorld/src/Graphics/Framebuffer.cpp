#include "Framebuffer.h"
#include <stdexcept>

Framebuffer::Framebuffer(VkDevice device, VkRenderPass renderPass, VkExtent2D extent, const std::vector<VkImageView>& attachments)
	: m_device(device)
{
	VkFramebufferCreateInfo framebufferInfo{};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.renderPass = renderPass;
	framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	framebufferInfo.pAttachments = attachments.data();
	framebufferInfo.width = extent.width;
	framebufferInfo.height = extent.height;
	framebufferInfo.layers = 1;

	if (vkCreateFramebuffer(m_device, &framebufferInfo, nullptr, &m_handle) != VK_SUCCESS) {
		throw std::runtime_error("failed to create framebuffer!");
	}
}

Framebuffer::~Framebuffer()
{
	if (m_handle != VK_NULL_HANDLE) {
		vkDestroyFramebuffer(m_device, m_handle, nullptr);
	}
}
