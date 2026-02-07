#pragma once

#include <vulkan/vulkan.h>

#include <vk_mem_alloc.h>

class VulkanMemory
{
public:
	VulkanMemory(VkInstance, VkPhysicalDevice, VkDevice);
	~VulkanMemory();

	struct AllocatedBuffer {
		VkBuffer buffer = VK_NULL_HANDLE;
		VmaAllocation allocation = VK_NULL_HANDLE;
		VmaAllocationInfo info{};
	};

	struct AllocatedImage {
		VkImage image = VK_NULL_HANDLE;
		VkImageView imageView = VK_NULL_HANDLE;
		VmaAllocation allocation = VK_NULL_HANDLE;
		VmaAllocationInfo info{};
	};


	AllocatedBuffer createBuffer(
		VkDeviceSize size,
		VkBufferUsageFlags usage,
		VmaMemoryUsage memoryUsage,
		VkMemoryPropertyFlags requiredFlags = 0,
		VmaAllocationCreateFlags allocFlags = 0
	);

	void destroyBuffer(const AllocatedBuffer& buffer);

	AllocatedImage createImage(
		const VkImageCreateInfo& imageInfo,
		VmaMemoryUsage memoryUsage,
		const VkImageViewCreateInfo* viewInfo = nullptr);

	void destroyImage(const AllocatedImage& image);

	void uploadToBuffer(AllocatedBuffer& buffer, const void* data, VkDeviceSize size);

private:
	void uploadHostVisible(VmaAllocation allocation, const void* data, VkDeviceSize size);
	void uploadDeviceLocal(AllocatedBuffer& dst, const void* data, VkDeviceSize size);

	VmaAllocator mAllocator = VK_NULL_HANDLE;
	VkDevice mDevice = VK_NULL_HANDLE;

};
