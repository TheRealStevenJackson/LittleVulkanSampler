#pragma once

#include "engine/graphics/renderer/VulkanContext.h"

#include <cstdint>

class VulkanDescriptorPool {
public:
	VulkanDescriptorPool(VulkanContext&, VkDescriptorType);
	VulkanDescriptorPool(VulkanContext&, VkDescriptorType, uint32_t descriptorCount);
	~VulkanDescriptorPool();

	VkDescriptorPool descriptorPool() const { return mDescriptorPool; }

private:
	void createDescriptorPool();

	VulkanContext& mContext;
	VkDescriptorType mDescriptorType;
	uint32_t mDescriptorCount = 1;

	VkDescriptorPool mDescriptorPool = VK_NULL_HANDLE;

};