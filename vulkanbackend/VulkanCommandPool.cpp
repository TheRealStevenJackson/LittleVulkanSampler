#include "vulkanbackend/VulkanCommandPool.h"
#include "vulkanbackend/VulkanContext.h"

#include <stdexcept>

VulkanCommandPool::VulkanCommandPool(const VulkanContext& context)
	: context_(context) {
	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = context_.findQueueFamilies(context_.physicalDevice()).graphics_family.value();

	if (vkCreateCommandPool(context_.device(), &poolInfo, nullptr, &pool_) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create command pool.");
	}
}

VulkanCommandPool::~VulkanCommandPool() {
	if (pool_ != VK_NULL_HANDLE) {
		vkDestroyCommandPool(context_.device(), pool_, nullptr);
	}
}

std::vector<VkCommandBuffer> VulkanCommandPool::allocate(uint32_t count) {
	std::vector<VkCommandBuffer> buffers(count);

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = pool_;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = count;

	if (vkAllocateCommandBuffers(context_.device(), &allocInfo, buffers.data()) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate command buffers.");
	}

	return buffers;
}

void VulkanCommandPool::begin(VkCommandBuffer cmd, VkCommandBufferUsageFlags flags) const {
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = flags;

	if (vkBeginCommandBuffer(cmd, &beginInfo) != VK_SUCCESS) {
		throw std::runtime_error("Failed to begin recording command buffer.");
	}
}

void VulkanCommandPool::end(VkCommandBuffer cmd) const {
	if (vkEndCommandBuffer(cmd) != VK_SUCCESS) {
		throw std::runtime_error("Failed to record command buffer.");
	}
}