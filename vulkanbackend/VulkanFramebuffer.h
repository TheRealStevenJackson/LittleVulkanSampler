#pragma once

#include "vulkanbackend/VulkanContext.h"

class VulkanFramebuffer {
public:
	VulkanFramebuffer(VulkanContext&, VkRenderPass, VkImageView, VkImageView, VkExtent2D);
	~VulkanFramebuffer();

private:
	void createFramebuffer();

	VulkanContext& mContext;
	VkFramebuffer mFramebuffer = VK_NULL_HANDLE;
	VkRenderPass mRenderPass = VK_NULL_HANDLE;
	VkImageView mColorView = VK_NULL_HANDLE;
	VkImageView mDepthView = VK_NULL_HANDLE;
	VkExtent2D mExtent;
};