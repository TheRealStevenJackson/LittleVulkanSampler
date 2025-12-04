#pragma once

#include "vulkanbackend/VulkanContext.h"

class VulkanDescriptorSetLayout {
public:

	VulkanDescriptorSetLayout(VulkanContext&, VkDescriptorType, VkShaderStageFlagBits);
	~VulkanDescriptorSetLayout();

	VkDescriptorSetLayout descriptorsetLayout() const { return mDescriptorSetLayout; }

private:
	void createDescriptorSetLayout();

	VulkanContext& mContext;
	VkDescriptorType mType;
	VkShaderStageFlagBits mStageFlags;
	VkDescriptorSetLayout mDescriptorSetLayout = VK_NULL_HANDLE;
};