#pragma once

#include "vulkanbackend/VulkanContext.h"
#include "vulkanbackend/VulkanDescriptorPool.h"
#include "vulkanbackend/VulkanDescriptorSetLayout.h"
#include "vulkanbackend/VulkanBuffer.h"

class VulkanDescriptorSet {
public:
	VulkanDescriptorSet(VulkanContext&, VulkanDescriptorPool&, VulkanDescriptorSetLayout&);

	VulkanDescriptorSet(const VulkanDescriptorSet&) = delete;
	VulkanDescriptorSet& operator=(const VulkanDescriptorSet&) = delete;

	VulkanDescriptorSet(VulkanDescriptorSet&&) noexcept = default;
	VulkanDescriptorSet& operator=(VulkanDescriptorSet&&) noexcept = default;

	~VulkanDescriptorSet();

	VkDescriptorSet descriptorSet() const { return mDescriptorSet; }

	void writeUniformBuffer(VulkanBuffer&, VkDeviceSize);

private:
	void allocateDescriptorSet();

	VulkanContext& mContext;
	VulkanDescriptorPool& mDescriptorPool;
	VulkanDescriptorSetLayout& mDescriptorSetLayout;

	VkDescriptorSet mDescriptorSet = VK_NULL_HANDLE;

};