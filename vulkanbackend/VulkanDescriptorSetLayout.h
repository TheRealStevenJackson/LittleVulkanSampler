#pragma once

#include "vulkanbackend/VulkanContext.h"

class VulkanDescriptorSetLayout {
public:

	VulkanDescriptorSetLayout(VulkanContext&, VkDescriptorType, VkShaderStageFlagBits);
	~VulkanDescriptorSetLayout();

private:
	void createDescriptorSetLayout();

	VulkanContext& mContext;
	VkDescriptorType mType;
	VkShaderStageFlagBits mStageFlags;
	VkDescriptorSetLayout mDescriptorSetLayout = VK_NULL_HANDLE;
};