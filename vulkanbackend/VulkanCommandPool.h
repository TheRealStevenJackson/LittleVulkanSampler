#pragma once

#include <vulkan/vulkan.h>
#include <vector>

class VulkanContext;

class VulkanCommandPool {
public:
	explicit VulkanCommandPool(const VulkanContext& context);
	~VulkanCommandPool();

	std::vector<VkCommandBuffer> allocate(uint32_t count);

	void begin(VkCommandBuffer cmd, VkCommandBufferUsageFlags flags = 0) const;
	void end(VkCommandBuffer cmd) const;

	VkCommandPool pool() const { return pool_; }

private:
	const VulkanContext& context_;
	VkCommandPool pool_ = VK_NULL_HANDLE;
};