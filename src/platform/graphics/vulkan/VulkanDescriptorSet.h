#pragma once

#include "platform/graphics/vulkan/VulkanContext.h"
#include "platform/graphics/vulkan/VulkanDescriptorPool.h"
#include "platform/graphics/vulkan/VulkanDescriptorSetLayout.h"
#include "platform/graphics/vulkan/VulkanBuffer.h"
#include "platform/graphics/vulkan/VulkanImage.h"

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
	void writeUniformBuffer(VulkanBuffer&, VkDeviceSize, uint32_t binding);
	void writeCombinedImageSampler(VulkanImage&, VkSampler, uint32_t binding);

private:
	void allocateDescriptorSet();

	VulkanContext& mContext;
	VulkanDescriptorPool& mDescriptorPool;
	VulkanDescriptorSetLayout& mDescriptorSetLayout;

	VkDescriptorSet mDescriptorSet = VK_NULL_HANDLE;

};