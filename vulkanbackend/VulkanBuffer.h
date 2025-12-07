#pragma once

#include "vulkanbackend/VulkanContext.h"
#include "vulkanbackend/VulkanMemory.h"

#include <vulkan/vulkan.h>
#include <cstdint>
#include <cstring>

class VulkanBuffer {
public:
	VulkanBuffer(VulkanContext& context, VkDeviceSize size,
		VkBufferUsageFlags bufferUsage, VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_AUTO,
		VkMemoryPropertyFlags requiredFlags = 0
	);

	VulkanBuffer(VulkanContext& context, const void* data, VkDeviceSize size,
		VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
		VkMemoryPropertyFlags requiredFlags = 0, VmaAllocationCreateFlags allocFlags = 0
	);

	~VulkanBuffer();

	VkBuffer buffer() const { return mAllocatedBuffer.buffer; }
	VmaAllocation allocation() const { return mAllocatedBuffer.allocation; }

	void upload(const void* data, VkDeviceSize size);

private:
	VulkanContext& mContext;
	VkDeviceSize mSize;

	VulkanMemory::AllocatedBuffer mAllocatedBuffer;
};