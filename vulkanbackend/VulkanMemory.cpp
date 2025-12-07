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
	VkMemoryPropertyFlags requiredFlags,
	VmaAllocationCreateFlags allocFlags)
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
	allocInfo.flags = allocFlags;

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

void VulkanMemory::uploadHostVisible(VmaAllocation allocation, const void* data, VkDeviceSize size) 
{
	void* mapped = nullptr;
	vmaMapMemory(mAllocator, allocation, &mapped);
	std::memcpy(mapped, data, static_cast<size_t>(size));
	vmaUnmapMemory(mAllocator, allocation);
}

void VulkanMemory::uploadDeviceLocal(AllocatedBuffer& dst, const void* data, VkDeviceSize size)
{
	// Create staging buffer
	AllocatedBuffer staging = createBuffer(
		size,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VMA_MEMORY_USAGE_CPU_ONLY,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	);

	uploadHostVisible(staging.allocation, data, size);

	// Create command buffer for copy
	VkCommandPool tempPool;
	VkCommandBuffer tempCmd;

	// --- Begin single-time command ---
	{
		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = 0; // Replace with your graphics queue family index
		poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

		vkCreateCommandPool(mDevice, &poolInfo, nullptr, &tempPool);

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = tempPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;

		vkAllocateCommandBuffers(mDevice, &allocInfo, &tempCmd);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(tempCmd, &beginInfo);
	}

	// --- Record copy ---
	VkBufferCopy copy{};
	copy.srcOffset = 0;
	copy.dstOffset = 0;
	copy.size = size;

	vkCmdCopyBuffer(tempCmd, staging.buffer, dst.buffer, 1, &copy);

	vkEndCommandBuffer(tempCmd);

	// Submit
	VkQueue queue;
	vkGetDeviceQueue(mDevice, 0, 0, &queue); // Replace with your graphics queue index

	VkSubmitInfo submit{};
	submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit.commandBufferCount = 1;
	submit.pCommandBuffers = &tempCmd;

	vkQueueSubmit(queue, 1, &submit, VK_NULL_HANDLE);
	vkQueueWaitIdle(queue);

	// Cleanup
	vkFreeCommandBuffers(mDevice, tempPool, 1, &tempCmd);
	vkDestroyCommandPool(mDevice, tempPool, nullptr);
	destroyBuffer(staging);
}

void VulkanMemory::uploadToBuffer(AllocatedBuffer& buffer, const void* data, VkDeviceSize size)
{
	// Query memory properties
	VkMemoryPropertyFlags memFlags = 0;
	vmaGetAllocationMemoryProperties(mAllocator, buffer.allocation, &memFlags);

	bool isHost = (memFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0;

	if (isHost) {
		uploadHostVisible(buffer.allocation, data, size);
	}
	else {
		uploadDeviceLocal(buffer, data, size);
	}

}
