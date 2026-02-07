#pragma once

#include "platform/graphics/vulkan/VulkanContext.h"
#include "platform/graphics/vulkan/VulkanMemory.h"

#include <vulkan/vulkan.h>

class VulkanImage {
public:
	/** Depth/stencil attachment (e.g. depth buffer). */
	VulkanImage(VulkanContext&, VkFormat, VkExtent2D);

	/** 2D texture with given usage (e.g. TRANSFER_DST | SAMPLED). */
	VulkanImage(VulkanContext&, VkFormat, VkExtent2D, VkImageUsageFlags usage);

	VulkanImage(const VulkanImage&) = delete;
	VulkanImage& operator=(const VulkanImage&) = delete;

	VulkanImage(VulkanImage&&) noexcept = default;
	VulkanImage& operator=(VulkanImage&&) noexcept = default;

	~VulkanImage();

	VkFormat format() const { return mFormat; }
	VkExtent2D extent() const { return mExtent; }
	VkImage image() const { return mAllocatedImage.image; }
	VkImageView imageView() const { return mAllocatedImage.imageView; }

private:
	void createDepthImage();
	void createTextureImage(VkImageUsageFlags usage);
	VkImageAspectFlags getAspectFlags(VkFormat) const;

	VulkanContext& mContext;
	VkFormat mFormat;
	VkExtent2D mExtent;
	bool mIsTexture = false;

	VulkanMemory::AllocatedImage mAllocatedImage;
};