#pragma once

#include "engine/graphics/renderer/VulkanContext.h"
#include "engine/graphics/renderer/VulkanDescriptorSetLayout.h"

class VulkanPipelineLayout {
public:

	VulkanPipelineLayout(VulkanContext&, VulkanDescriptorSetLayout&);
	VulkanPipelineLayout(VulkanContext&, const std::vector<VulkanDescriptorSetLayout*>&);
	~VulkanPipelineLayout();

	VkPipelineLayout pipelineLayout() const { return mPipelineLayout; }

private:
	void createPipelineLayout();
	void createPipelineLayout(const std::vector<VulkanDescriptorSetLayout*>&);

	VulkanContext& mContext;
	VulkanDescriptorSetLayout* mDescriptorSetLayout = nullptr;
	std::vector<VulkanDescriptorSetLayout*> mDescriptorSetLayouts;
	VkPipelineLayout mPipelineLayout = VK_NULL_HANDLE;
};