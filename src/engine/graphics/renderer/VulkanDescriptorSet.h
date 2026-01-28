#pragma once

#include "engine/graphics/renderer/VulkanContext.h"
#include "engine/graphics/renderer/VulkanDescriptorPool.h"
#include "engine/graphics/renderer/VulkanDescriptorSetLayout.h"
#include "engine/graphics/renderer/VulkanBuffer.h"
#include "engine/graphics/renderer/VulkanImage.h"

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