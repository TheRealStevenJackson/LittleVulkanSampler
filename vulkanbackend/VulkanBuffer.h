#pragma once

#include <vulkan/vulkan.h>
#include <cstdint>
#include <cstring>

class VulkanContext;

class VulkanBuffer {
public:
	VulkanBuffer(
		const VulkanContext& context,
		VkDeviceSize size,
		VkBufferUsageFlags usage,
		VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	);

	VulkanBuffer(
		const VulkanContext& context,
		const void* data,
		VkDeviceSize size,
		VkBufferUsageFlags usage
	);

	~VulkanBuffer();

	VkBuffer buffer() const { return buffer_; }
	VkDeviceMemory memory() const { return memory_; }
	VkDeviceSize size() const { return size_; }

	void upload(const void* data, VkDeviceSize size);

private:
	const VulkanContext& context_;
	VkBuffer buffer_ = VK_NULL_HANDLE;
	VkDeviceMemory memory_ = VK_NULL_HANDLE;
	VkDeviceSize size_ = 0;

	void createBuffer(VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
};