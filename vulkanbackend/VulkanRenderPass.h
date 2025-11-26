#pragma once

#include "vulkanbackend/VulkanContext.h"

class VulkanRenderPass {
public:
	VulkanRenderPass(VulkanContext&, VkFormat, VkFormat);
	~VulkanRenderPass();

	VkRenderPass renderPass() const { return mRenderPass; }

private:
	void createRenderPass();

	VulkanContext& mContext;
	VkRenderPass mRenderPass = VK_NULL_HANDLE;
	VkFormat mImageFormat; 
	VkFormat mDepthFormat;
};