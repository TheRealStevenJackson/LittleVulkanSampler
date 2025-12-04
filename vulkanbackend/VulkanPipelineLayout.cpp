#include "vulkanbackend/VulkanPipelineLayout.h"

#include <stdexcept>
#include <iostream>

VulkanPipelineLayout::VulkanPipelineLayout(VulkanContext& context, VulkanDescriptorSetLayout& descriptorSetLayout)
	: mContext(context),
	  mDescriptorSetLayout(descriptorSetLayout)
{
	createPipelineLayout();

	std::cout << "Created pipeline layouat." << std::endl;
}

VulkanPipelineLayout::~VulkanPipelineLayout()
{
	std::cout << "Destroying pipeline layout." << std::endl;

	if (mPipelineLayout) {
		vkDestroyPipelineLayout(mContext.device(), mPipelineLayout, nullptr);
	}
}

void VulkanPipelineLayout::createPipelineLayout()
{
	std::vector<VkDescriptorSetLayout> descriptorSetLayouts{};
	descriptorSetLayouts.push_back(mDescriptorSetLayout.descriptorsetLayout());
		
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();

	if (vkCreatePipelineLayout(mContext.device(), &pipelineLayoutInfo, nullptr, &mPipelineLayout) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create pipeline layout.");
	}
}