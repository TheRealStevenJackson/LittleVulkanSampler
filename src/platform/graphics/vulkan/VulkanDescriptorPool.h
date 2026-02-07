#pragma once

#include "platform/graphics/vulkan/VulkanContext.h"

#include <cstdint>

class VulkanDescriptorPool {
public:
	VulkanDescriptorPool(VulkanContext&, VkDescriptorType);
	VulkanDescriptorPool(VulkanContext&, VkDescriptorType, uint32_t descriptorCount);
	VulkanDescriptorPool(VulkanContext&, const std::vector<VkDescriptorPoolSize>&, uint32_t maxSets);
	~VulkanDescriptorPool();

	VkDescriptorPool descriptorPool() const { return mDescriptorPool; }

private:
	void createDescriptorPool();
	void createDescriptorPool(const std::vector<VkDescriptorPoolSize>&, uint32_t maxSets);

	VulkanContext& mContext;
	VkDescriptorType mDescriptorType;
	uint32_t mDescriptorCount = 1;
	std::vector<VkDescriptorPoolSize> mPoolSizes;

	VkDescriptorPool mDescriptorPool = VK_NULL_HANDLE;

};