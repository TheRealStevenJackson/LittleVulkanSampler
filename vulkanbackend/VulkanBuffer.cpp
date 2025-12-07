#include "vulkanbackend/VulkanBuffer.h"
#include "vulkanbackend/VulkanContext.h"

#include <stdexcept>

VulkanBuffer::VulkanBuffer(VulkanContext& context, VkDeviceSize size, VkBufferUsageFlags bufferUsage, 
	VmaMemoryUsage memoryUsage, VkMemoryPropertyFlags requiredFlags)
	: mContext(context),
	mSize(size) 
{
	VkBufferCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	info.size = size;
	info.usage = bufferUsage;
	info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	mAllocatedBuffer = mContext.vma().createBuffer(size, bufferUsage, memoryUsage, requiredFlags);
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