 #pragma once

#include "platform/graphics/vulkan/VulkanRenderPass.h"
#include "platform/graphics/vulkan/VulkanFramebuffer.h"
#include "platform/graphics/vulkan/VulkanPipeline.h"
#include "platform/graphics/vulkan/VulkanDescriptorSet.h"
#include <cstdint>
#include <vector>

class VulkanCommandBuffer {
public:
	VulkanCommandBuffer(VkCommandBuffer);
	~VulkanCommandBuffer();

	VkCommandBuffer getHandle() const { return mCommandBuffer; }

	void begin();
	void beginRenderPass(VulkanRenderPass&, VulkanFramebuffer&, VkExtent2D);
	void bindPipeline(VulkanPipeline&);
	void setViewport(VkExtent2D);
	void setScissor(VkExtent2D);
	void bindDescriptorSet(VulkanPipelineLayout& pipelineLayout, VulkanDescriptorSet&);
	void bindDescriptorSets(VulkanPipelineLayout& pipelineLayout, const std::vector<VulkanDescriptorSet*>&);
	/** Bind descriptor sets with dynamic offsets (order must match dynamic bindings in sets). */
	void bindDescriptorSets(VulkanPipelineLayout& pipelineLayout, const std::vector<VulkanDescriptorSet*>& descriptorSets, const std::vector<uint32_t>& dynamicOffsets);
	void bindVertexBuffer(VulkanBuffer&);
	void bindIndexBuffer(VulkanBuffer&);
	void draw();
	void drawIndexed(uint32_t);
	void endRenderPass();
	void end();

private:
	VkCommandBuffer mCommandBuffer = VK_NULL_HANDLE;
};