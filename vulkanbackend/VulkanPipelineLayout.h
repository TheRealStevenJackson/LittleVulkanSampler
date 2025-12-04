#pragma once

#include "vulkanbackend/VulkanContext.h"
#include "vulkanbackend/VulkanDescriptorSetLayout.h"

class VulkanPipelineLayout {
public:

	VulkanPipelineLayout(VulkanContext&, VulkanDescriptorSetLayout&);
	~VulkanPipelineLayout();

private:
	void createPipelineLayout();

	VulkanContext& mContext;
	VulkanDescriptorSetLayout& mDescriptorSetLayout;
	VkPipelineLayout mPipelineLayout = VK_NULL_HANDLE;
};