#include "vulkanbackend/VulkanFramebuffer.h"

#include <stdexcept>
#include <iostream>

VulkanFramebuffer::VulkanFramebuffer(VulkanContext& context, VkRenderPass renderPass, VkImageView colorView, VkImageView depthView, VkExtent2D extent)
	: mContext(context),
	  mRenderPass(renderPass),
	  mColorView(colorView),
	  mDepthView(depthView),
	  mExtent(extent)
{
	createFramebuffer();

	std::cout << "Created framebuffer." << std::endl;
}

VulkanFramebuffer::~VulkanFramebuffer()
{
	std::cout << "Deleting framebuffer." << std::endl;
	if (mFramebuffer != VK_NULL_HANDLE) {
		vkDestroyFramebuffer(mContext.device(), mFramebuffer, nullptr);
	}
}

void VulkanFramebuffer::createFramebuffer()
{
	std::vector<VkImageView> attachments;
	attachments.reserve(2);
	attachments.push_back(mColorView);
	attachments.push_back(mDepthView);

	VkFramebufferCreateInfo framebufferInfo{};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.renderPass = mRenderPass;
	framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	framebufferInfo.pAttachments = attachments.data();
	framebufferInfo.width = mExtent.width;
	framebufferInfo.height = mExtent.height;
	framebufferInfo.layers = 1;

	if (vkCreateFramebuffer(mContext.device(), &framebufferInfo, nullptr, &mFramebuffer) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create framebuffer.");
	}
}