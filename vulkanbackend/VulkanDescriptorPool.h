#pragma once

#include "vulkanbackend/VulkanContext.h"

class VulkanDescriptorPool {
public:
	VulkanDescriptorPool(VulkanContext&, VkDescriptorType);
	~VulkanDescriptorPool();

	VkDescriptorPool descriptorPool() const { return mDescriptorPool; }

private:
	void createDescriptorPool();

	VulkanContext& mContext;
	VkDescriptorType mDescriptorType;

	VkDescriptorPool mDescriptorPool = VK_NULL_HANDLE;

};