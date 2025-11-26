#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include "VulkanContext.h"

class VulkanSwapchain {
public:
	VulkanSwapchain(VulkanContext&, GLFWwindow*);
	~VulkanSwapchain();

	VkFormat getImageFormat() const { return imageFormat_; }
	VkExtent2D getExtent() const { return extent_; }
	uint32_t imageCount() const { return mImageCount; }
	const std::vector<VkImageView>& getImageViews() const { return imageViews_; }
	VkSwapchainKHR getHandle() const { return swapchain_; }

private:
	void createSwapchain(GLFWwindow*);
	void createImageViews();

	VulkanContext& context_;
	VkSwapchainKHR swapchain_ = VK_NULL_HANDLE;
	VkFormat imageFormat_;
	VkExtent2D extent_;
	uint32_t mImageCount;
	std::vector<VkImage> images_;
	std::vector<VkImageView> imageViews_;
};