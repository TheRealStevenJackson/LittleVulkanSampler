#pragma once

#include "platform/graphics/vulkan/VulkanContext.h"

#include <vector>

class VulkanDescriptorSetLayout {
public:

	VulkanDescriptorSetLayout(VulkanContext&, VkDescriptorType, VkShaderStageFlagBits);
	VulkanDescriptorSetLayout(VulkanContext&, const std::vector<VkDescriptorSetLayoutBinding>&);
	~VulkanDescriptorSetLayout();

	VkDescriptorSetLayout descriptorsetLayout() const { return mDescriptorSetLayout; }

private:
	void createDescriptorSetLayout();
	void createDescriptorSetLayout(const std::vector<VkDescriptorSetLayoutBinding>&);

	VulkanContext& mContext;
	VkDescriptorType mType = VK_DESCRIPTOR_TYPE_MAX_ENUM;
	VkShaderStageFlagBits mStageFlags = static_cast<VkShaderStageFlagBits>(0);
	std::vector<VkDescriptorSetLayoutBinding> mBindings;
	VkDescriptorSetLayout mDescriptorSetLayout = VK_NULL_HANDLE;
};