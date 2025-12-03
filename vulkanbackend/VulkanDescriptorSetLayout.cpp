#include "vulkanbackend/VulkanDescriptorSetLayout.h"

#include <stdexcept>

VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(VulkanContext& context, VkDescriptorType type, VkShaderStageFlagBits stageFlags)
	: mContext(context),
	  mType(type),
	  mStageFlags(stageFlags)
{
	createDescriptorSetLayout();
}

VulkanDescriptorSetLayout::~VulkanDescriptorSetLayout()
{
	if (mDescriptorSetLayout) {
		vkDestroyDescriptorSetLayout(mContext.device(), mDescriptorSetLayout, nullptr);
	}
}

void VulkanDescriptorSetLayout::createDescriptorSetLayout()
{
	VkDescriptorSetLayoutBinding layoutBinding{};
	layoutBinding.binding = 0;
	layoutBinding.descriptorType = mType;
	layoutBinding.descriptorCount = 1;
	layoutBinding.stageFlags = mStageFlags;

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = 1;
	layoutInfo.pBindings = &layoutBinding;

	if (vkCreateDescriptorSetLayout(mContext.device(), &layoutInfo, nullptr, &mDescriptorSetLayout) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create descriptor set layout.");
	}
}