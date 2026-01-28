#include "engine/graphics/renderer/VulkanDescriptorPool.h"

#include <stdexcept>

VulkanDescriptorPool::VulkanDescriptorPool(VulkanContext& context, VkDescriptorType descriptorType)
	: mContext(context),
	  mDescriptorType(descriptorType),
	  mDescriptorCount(1)
{
	createDescriptorPool();
}

VulkanDescriptorPool::VulkanDescriptorPool(VulkanContext& context, VkDescriptorType descriptorType, uint32_t descriptorCount)
	: mContext(context),
	  mDescriptorType(descriptorType),
	  mDescriptorCount(descriptorCount)
{
	createDescriptorPool();
}

VulkanDescriptorPool::VulkanDescriptorPool(VulkanContext& context, const std::vector<VkDescriptorPoolSize>& poolSizes, uint32_t maxSets)
	: mContext(context),
	  mPoolSizes(poolSizes)
{
	createDescriptorPool(poolSizes, maxSets);
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
	poolSize.descriptorCount = mDescriptorCount;

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

void VulkanDescriptorPool::createDescriptorPool(const std::vector<VkDescriptorPoolSize>& poolSizes, uint32_t maxSets)
{
	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	poolInfo.maxSets = maxSets;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();

	if (vkCreateDescriptorPool(mContext.device(), &poolInfo, nullptr, &mDescriptorPool) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create descriptor pool.");
	}
}