#pragma once

#include "engine/graphics/renderer/VulkanContext.h"
#include "engine/graphics/renderer/VulkanMemory.h"

class VulkanImage {
public:
	VulkanImage(VulkanContext&, VkFormat, VkExtent2D);

	VulkanImage(const VulkanImage&) = delete;
	VulkanImage& operator=(const VulkanImage&) = delete;

	VulkanImage(VulkanImage&&) noexcept = default;
	VulkanImage& operator=(VulkanImage&&) noexcept = default;

	~VulkanImage();

	VkFormat format() const { return mFormat; }
	VkImageView imageView() const { return mAllocatedImage.imageView; }

private:
	void createImage();
	VkImageAspectFlagBits getAspectFlags(VkFormat);

	VulkanContext& mContext;
	VkFormat mFormat;
	VkExtent2D mExtent;

	VulkanMemory::AllocatedImage mAllocatedImage;

};