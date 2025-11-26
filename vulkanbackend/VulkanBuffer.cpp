#include "vulkanbackend/VulkanBuffer.h"
#include "vulkanbackend/VulkanContext.h"

#include <stdexcept>

VulkanBuffer::VulkanBuffer(
	const VulkanContext& context,
	VkDeviceSize size,
	VkBufferUsageFlags usage,
	VkMemoryPropertyFlags properties
)
	: context_(context), size_(size) {
	createBuffer(usage, properties);
}

VulkanBuffer::VulkanBuffer(
	const VulkanContext& context,
	const void* data,
	VkDeviceSize size,
	VkBufferUsageFlags usage
)
	: context_(context), size_(size) {
	createBuffer(
		usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	);

	upload(data, size);
}

VulkanBuffer::~VulkanBuffer() {
	if (buffer_ != VK_NULL_HANDLE) {
		vkDestroyBuffer(context_.device(), buffer_, nullptr);
	}

	if (memory_ != VK_NULL_HANDLE) {
		vkFreeMemory(context_.device(), memory_, nullptr);
	}
}

void VulkanBuffer::createBuffer(VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) {
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size_;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(context_.device(), &bufferInfo, nullptr, &buffer_) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create buffer.");
	}

	VkMemoryRequirements memReq;
	vkGetBufferMemoryRequirements(context_.device(), buffer_, &memReq);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memReq.size;
	allocInfo.memoryTypeIndex = findMemoryType(memReq.memoryTypeBits, properties);

	if (vkAllocateMemory(context_.device(), &allocInfo, nullptr, &memory_) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate buffer memory.");
	}

	vkBindBufferMemory(context_.device(), buffer_, memory_, 0);
}

uint32_t VulkanBuffer::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
	VkPhysicalDeviceMemoryProperties memProps;
	vkGetPhysicalDeviceMemoryProperties(context_.physicalDevice(), &memProps);

	for (uint32_t i = 0; i < memProps.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) &&
			(memProps.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	throw std::runtime_error("Failed to find suitable memory type.");
}

void VulkanBuffer::upload(const void* data, VkDeviceSize size) {
	void* mapped = nullptr;
	vkMapMemory(context_.device(), memory_, 0, size, 0, &mapped);
	std::memcpy(mapped, data, static_cast<size_t>(size));
	vkUnmapMemory(context_.device(), memory_);
}