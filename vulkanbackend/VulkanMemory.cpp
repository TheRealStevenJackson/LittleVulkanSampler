#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#include "vulkanbackend/VulkanMemory.h"

#include <stdexcept>
#include <iostream>

VulkanMemory::VulkanMemory(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device)
	: mDevice(device)
{
	VmaAllocatorCreateInfo allocatorInfo{};
	allocatorInfo.instance = instance;
	allocatorInfo.physicalDevice = physicalDevice;
	allocatorInfo.device = device;

	vmaCreateAllocator(&allocatorInfo, &mAllocator);

	std::cout << "VMA created." << std::endl;
}

VulkanMemory::~VulkanMemory()
{
	if (mAllocator != VK_NULL_HANDLE) {
		vmaDestroyAllocator(mAllocator);
		mAllocator = VK_NULL_HANDLE;
	}

	std::cout << "VMA destroyed." << std::endl;
}

VulkanMemory::AllocatedBuffer VulkanMemory::createBuffer(
	VkDeviceSize size,
	VkBufferUsageFlags usage,
	VmaMemoryUsage memoryUsage,
	VkMemoryPropertyFlags requiredFlags)
{
	std::cout << "VMA creating buffer." << std::endl;
	AllocatedBuffer result{};

	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VmaAllocationCreateInfo allocInfo{};
	allocInfo.usage = memoryUsage;
	allocInfo.requiredFlags = requiredFlags;

	if (vmaCreateBuffer(mAllocator, &bufferInfo, &allocInfo, &result.buffer, &result.allocation, &result.info) != VK_SUCCESS) {
		throw std::runtime_error("Falied to allocate VMA buffer.");
	}

	return result;
}

void VulkanMemory::destroyBuffer(const AllocatedBuffer& buffer)
{
	std::cout << "VMA deleting buffer." << std::endl;
	if (buffer.buffer) {
		vmaDestroyBuffer(mAllocator, buffer.buffer, buffer.allocation);
	}
}

VulkanMemory::AllocatedImage VulkanMemory::createImage(
	const VkImageCreateInfo& imageInfo,
	VmaMemoryUsage memoryUsage,
	const VkImageViewCreateInfo* viewInfo)
{
	std::cout << "VMA creating image." << std::endl;
	AllocatedImage result{};
	
	VmaAllocationCreateInfo allocInfo{};
	allocInfo.usage = memoryUsage;

	if (vmaCreateImage(mAllocator, &imageInfo, &allocInfo, &result.image, &result.allocation, &result.info) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create VMA image.");
	}

	if (viewInfo) {
		VkImageViewCreateInfo imageViewInfo = *viewInfo;
		imageViewInfo.image = result.image;
		vkCreateImageView(mDevice, &imageViewInfo, nullptr, &result.imageView);
	}

	return result;
}

void VulkanMemory::destroyImage(const AllocatedImage& image)
{
	std::cout << "VMA deleting image." << std::endl;
	if (image.imageView) {
		vkDestroyImageView(mDevice, image.imageView, nullptr);
	}

	if (image.image) {
		vmaDestroyImage(mAllocator, image.image, image.allocation);
	}
}
