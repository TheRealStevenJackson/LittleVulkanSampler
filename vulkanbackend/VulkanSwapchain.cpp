#include "VulkanSwapchain.h"
#include <stdexcept>
#include <iostream>
#include <limits>

VulkanSwapchain::VulkanSwapchain(VulkanContext& context, GLFWwindow* window)
	: context_(context) {
	createSwapchain(window);
	createImageViews();
}

VulkanSwapchain::~VulkanSwapchain() {
	VkDevice device = context_.device();
	for (auto view : imageViews_) {
		vkDestroyImageView(device, view, nullptr);
	}

	if (swapchain_ != VK_NULL_HANDLE) {
		vkDestroySwapchainKHR(device, swapchain_, nullptr);
	}
}

void VulkanSwapchain::createSwapchain(GLFWwindow* window) {
	auto support = context_.querySwapchainSupport(context_.physicalDevice());

	VkSurfaceFormatKHR surfaceFormat = context_.chooseSwapSurfaceFormat(support.formats);
	VkPresentModeKHR presentMode = context_.chooseSwapPresentMode(support.present_modes);
	VkExtent2D extent = context_.chooseSwapExtent(support.capabilities, window);

	mImageCount = support.capabilities.minImageCount + 1;
	if (support.capabilities.maxImageCount > 0 &&
		mImageCount > support.capabilities.maxImageCount)
		mImageCount = support.capabilities.maxImageCount;

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = context_.surface();
	createInfo.minImageCount = mImageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	auto indices = context_.findQueueFamilies(context_.physicalDevice());
	uint32_t queueFamilyIndices[] = {
		indices.graphics_family.value(),
		indices.present_family.value()
	};

	if (indices.graphics_family != indices.present_family) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	createInfo.preTransform = support.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(context_.device(), &createInfo, nullptr, &swapchain_) != VK_SUCCESS)
		throw std::runtime_error("Failed to create swapchain.");

	// Retrieve swapchain images
	vkGetSwapchainImagesKHR(context_.device(), swapchain_, &mImageCount, nullptr);
	images_.resize(mImageCount);
	vkGetSwapchainImagesKHR(context_.device(), swapchain_, &mImageCount, images_.data());

	imageFormat_ = surfaceFormat.format;
	extent_ = extent;
}

void VulkanSwapchain::createImageViews() {
	imageViews_.resize(images_.size());

	for (size_t i = 0; i < images_.size(); i++) {
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = images_[i];
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = imageFormat_;

		viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(context_.device(), &viewInfo, nullptr, &imageViews_[i]) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create image views for swapchain.");
		}
	}

	std::cout << "Created " << imageViews_.size() << " swapchain image views." << std::endl;
}