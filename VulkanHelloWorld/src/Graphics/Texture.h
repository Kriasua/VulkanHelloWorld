#pragma once
#include <vulkan/vulkan.h>
#include <stdexcept>
#include "../Core/Devices.h"

class Texture
{
public:
	Texture(Devices& device,
		uint32_t width,
		uint32_t height,
		VkFormat format,
		VkImageTiling tiling,
		VkImageUsageFlags usage,
		VkMemoryPropertyFlags properties,
		VkImageAspectFlags aspectFlags);
	~Texture();

	Texture(const Texture&) = delete;
	Texture& operator=(const Texture&) = delete;

	const VkImage& getImage() const { return m_image; }
	const VkImageView& getImageView() const { return m_imageView; }
	const VkFormat& getFormat() const { return m_format; }

private:
	Devices& m_device; // 引用Devices 类，方便获取物理和逻辑设备
	VkImage m_image = VK_NULL_HANDLE;
	VkDeviceMemory m_imageMemory = VK_NULL_HANDLE;
	VkImageView m_imageView = VK_NULL_HANDLE;
	VkFormat m_format;

	void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties);
	void createImageView(VkImageAspectFlags aspectFlags);

	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
};
