#include "platform/graphics/vulkan/VulkanBuffer.h"
#include "platform/graphics/vulkan/VulkanContext.h"

#include <stdexcept>

VulkanBuffer::VulkanBuffer(VulkanContext& context, VkDeviceSize size, VkBufferUsageFlags bufferUsage, 
	VmaMemoryUsage memoryUsage, VkMemoryPropertyFlags requiredFlags)
	: mContext(context),
	mSize(size) 
{
	mAllocatedBuffer = mContext.vma().createBuffer(size, bufferUsage, memoryUsage, requiredFlags);
}

VulkanBuffer::VulkanBuffer(VulkanContext& context, VkDeviceSize size, VkBufferUsageFlags bufferUsage,
	VmaMemoryUsage memoryUsage, VkMemoryPropertyFlags requiredFlags, VmaAllocationCreateFlags allocFlags)
	: mContext(context),
	mSize(size)
{
	mAllocatedBuffer = mContext.vma().createBuffer(size, bufferUsage, memoryUsage, requiredFlags, allocFlags);
}

VulkanBuffer::VulkanBuffer(VulkanContext& context, const void* data, VkDeviceSize size,
	VkBufferUsageFlags bufferUsage, VmaMemoryUsage memoryUsage, VkMemoryPropertyFlags requiredFlags,
	VmaAllocationCreateFlags allocFlags)
	: mContext(context),
	mSize(size) 
{
	mAllocatedBuffer = mContext.vma().createBuffer(size, bufferUsage | VK_BUFFER_USAGE_TRANSFER_DST_BIT, memoryUsage, requiredFlags, allocFlags);

	upload(data, size);
}

VulkanBuffer::~VulkanBuffer() {
	mContext.vma().destroyBuffer(mAllocatedBuffer);
}

void VulkanBuffer::upload(const void* data, VkDeviceSize size) {
	mContext.vma().uploadToBuffer(mAllocatedBuffer, data, size);
}

void VulkanBuffer::upload(VkDeviceSize offset, const void* data, VkDeviceSize size) {
	mContext.vma().uploadToBuffer(mAllocatedBuffer, offset, data, size);
}