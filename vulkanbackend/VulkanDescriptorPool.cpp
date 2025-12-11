#include "vulkanbackend/VulkanDescriptorPool.h"

#include <stdexcept>

VulkanDescriptorPool::VulkanDescriptorPool(VulkanContext& context, VkDescriptorType descriptorType)
	: mContext(context),
	mDescriptorType(descriptorType)
{
	createDescriptorPool();
}

VulkanDescriptorPool::~VulkanDescriptorPool()
{
	if (mDescriptorPool) {
		vkDestroyDescriptorPool(mContext.device(), mDescriptorPool, nullptr);
	}
}

void VulkanDescriptorPool::createDescriptorPool()
{
	VkDescriptorPoolSize poolSize{};
	poolSize.type = mDescriptorType;
	poolSize.descriptorCount = 1;

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	poolInfo.maxSets = 10;
	poolInfo.poolSizeCount = 1;
	poolInfo.pPoolSizes = &poolSize;

	if (vkCreateDescriptorPool(mContext.device(), &poolInfo, nullptr, &mDescriptorPool) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create descriptor pool.");
	}
}