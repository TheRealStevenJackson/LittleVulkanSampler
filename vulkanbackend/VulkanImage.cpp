#include "vulkanbackend/VulkanImage.h"

#include <vk_mem_alloc.h>

#include <stdexcept>
#include <iostream>

VulkanImage::VulkanImage(VulkanContext& context, VkFormat format, VkExtent2D extent)
	: mContext(context),
	  mFormat(format),
	  mExtent(extent)
{
	createImage();
	
	std::cout << "Created image and image view" << std::endl;
}

VulkanImage::~VulkanImage()
{
	auto& vma = mContext.vma();

	vma.destroyImage(mAllocatedImage);

	std::cout << "Destroyed image and image view" << std::endl;
}

void VulkanImage::createImage()
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

	auto& vma = mContext.vma();

	mAllocatedImage = vma.createImage(imageInfo, VMA_MEMORY_USAGE_AUTO, nullptr);
}

