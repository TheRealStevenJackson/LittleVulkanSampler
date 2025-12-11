#include "vulkanbackend/VulkanDescriptorSet.h"

#include <stdexcept>

VulkanDescriptorSet::VulkanDescriptorSet(VulkanContext& context, VulkanDescriptorPool& descriptorPool, VulkanDescriptorSetLayout& descriptorSetLayout)
	: mContext(context),
	mDescriptorPool(descriptorPool),
	mDescriptorSetLayout(descriptorSetLayout)
{
	allocateDescriptorSet();
}

VulkanDescriptorSet::~VulkanDescriptorSet()
{
	if (mDescriptorSet) {
		vkFreeDescriptorSets(mContext.device(), mDescriptorPool.descriptorPool(), 1, &mDescriptorSet);
	}
}

void VulkanDescriptorSet::allocateDescriptorSet()
{
	std::vector<VkDescriptorSetLayout> descriptorSetLayouts{
		mDescriptorSetLayout.descriptorsetLayout()
	};

	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = mDescriptorPool.descriptorPool();
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = descriptorSetLayouts.data();

	if (vkAllocateDescriptorSets(mContext.device(), &allocInfo, &mDescriptorSet) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate descriptor set.");
	}
}

void VulkanDescriptorSet::writeUniformBuffer(VulkanBuffer& uniformBuffer, VkDeviceSize size)
{
	VkDescriptorBufferInfo bufferInfo{};
	bufferInfo.buffer = uniformBuffer.buffer();
	bufferInfo.offset = 0;
	bufferInfo.range = size;
	
	VkWriteDescriptorSet write{};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.dstSet = mDescriptorSet;
	write.dstBinding = 0;
	write.dstArrayElement = 0;
	write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	write.descriptorCount = 1;
	write.pBufferInfo = &bufferInfo;

	vkUpdateDescriptorSets(mContext.device(), 1, &write, 0, nullptr);
}