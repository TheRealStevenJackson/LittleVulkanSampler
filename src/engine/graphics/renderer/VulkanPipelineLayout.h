#pragma once

#include "engine/graphics/renderer/VulkanContext.h"
#include "engine/graphics/renderer/VulkanDescriptorSetLayout.h"

class VulkanPipelineLayout {
public:

	VulkanPipelineLayout(VulkanContext&, VulkanDescriptorSetLayout&);
	~VulkanPipelineLayout();

	VkPipelineLayout pipelineLayout() const { return mPipelineLayout; }

private:
	void createPipelineLayout();

	VulkanContext& mContext;
	VulkanDescriptorSetLayout& mDescriptorSetLayout;
	VkPipelineLayout mPipelineLayout = VK_NULL_HANDLE;
};