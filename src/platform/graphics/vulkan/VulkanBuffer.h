#pragma once

#include "platform/graphics/vulkan/VulkanContext.h"
#include "platform/graphics/vulkan/VulkanMemory.h"

#include <vulkan/vulkan.h>
#include <cstdint>
#include <cstring>

class VulkanBuffer {
public:
	VulkanBuffer(VulkanContext& context, VkDeviceSize size,
		VkBufferUsageFlags bufferUsage, VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_AUTO,
		VkMemoryPropertyFlags requiredFlags = 0
	);
	/** Size-only with allocation flags (e.g. VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT for mappable uploads). */
	VulkanBuffer(VulkanContext& context, VkDeviceSize size,
		VkBufferUsageFlags bufferUsage, VmaMemoryUsage memoryUsage,
		VkMemoryPropertyFlags requiredFlags, VmaAllocationCreateFlags allocFlags);

	VulkanBuffer(VulkanContext& context, const void* data, VkDeviceSize size,
		VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
		VkMemoryPropertyFlags requiredFlags = 0, VmaAllocationCreateFlags allocFlags = 0
	);

	~VulkanBuffer();

	VkBuffer buffer() const { return mAllocatedBuffer.buffer; }
	VmaAllocation allocation() const { return mAllocatedBuffer.allocation; }

	void upload(const void* data, VkDeviceSize size);
	/** Upload data at offset (e.g. for dynamic UBO slot). */
	void upload(VkDeviceSize offset, const void* data, VkDeviceSize size);

	VkDeviceSize size() const { return mSize; }

private:
	VulkanContext& mContext;
	VkDeviceSize mSize;

	VulkanMemory::AllocatedBuffer mAllocatedBuffer;
};