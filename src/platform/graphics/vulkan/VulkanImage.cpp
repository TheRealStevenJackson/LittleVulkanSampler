#include "platform/graphics/vulkan/VulkanImage.h"

#include <vk_mem_alloc.h>

#include <stdexcept>
#include <iostream>

VulkanImage::VulkanImage(VulkanContext& context, VkFormat format, VkExtent2D extent)
	: mContext(context),
	  mFormat(format),
	  mExtent(extent)
{
	createDepthImage();
	std::cout << "Created depth image and image view" << std::endl;
}

VulkanImage::VulkanImage(VulkanContext& context, VkFormat format, VkExtent2D extent, VkImageUsageFlags usage)
	: mContext(context),
	  mFormat(format),
	  mExtent(extent),
	  mIsTexture(true)
{
	createTextureImage(usage);
	std::cout << "Created texture image and image view" << std::endl;
}

VulkanImage::~VulkanImage()
{
	mContext.vma().destroyImage(mAllocatedImage);
	std::cout << "Destroyed image and image view" << std::endl;
}

void VulkanImage::createDepthImage()
{
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.format = mFormat;
	imageInfo.extent.width = mExtent.width;
	imageInfo.extent.height = mExtent.height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	VkImageViewCreateInfo imageViewInfo{};
	imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewInfo.format = mFormat;
	imageViewInfo.subresourceRange.aspectMask = getAspectFlags(mFormat);
	imageViewInfo.subresourceRange.baseMipLevel = 0;
	imageViewInfo.subresourceRange.levelCount = 1;
	imageViewInfo.subresourceRange.baseArrayLayer = 0;
	imageViewInfo.subresourceRange.layerCount = 1;

	mAllocatedImage = mContext.vma().createImage(imageInfo, VMA_MEMORY_USAGE_AUTO, &imageViewInfo);
}

void VulkanImage::createTextureImage(VkImageUsageFlags usage)
{
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.format = mFormat;
	imageInfo.extent.width = mExtent.width;
	imageInfo.extent.height = mExtent.height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.usage = usage;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	VkImageViewCreateInfo imageViewInfo{};
	imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewInfo.format = mFormat;
	imageViewInfo.subresourceRange.aspectMask = getAspectFlags(mFormat);
	imageViewInfo.subresourceRange.baseMipLevel = 0;
	imageViewInfo.subresourceRange.levelCount = 1;
	imageViewInfo.subresourceRange.baseArrayLayer = 0;
	imageViewInfo.subresourceRange.layerCount = 1;

	mAllocatedImage = mContext.vma().createImage(imageInfo, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE, &imageViewInfo);
}

VkImageAspectFlags VulkanImage::getAspectFlags(VkFormat format) const
{
	switch (format) {
	case VK_FORMAT_D32_SFLOAT:
		return VK_IMAGE_ASPECT_DEPTH_BIT;
	case VK_FORMAT_R8G8B8A8_SRGB:
	case VK_FORMAT_R8G8B8A8_UNORM:
		return VK_IMAGE_ASPECT_COLOR_BIT;
	default:
		break;
	}
	return VK_IMAGE_ASPECT_COLOR_BIT;
}
