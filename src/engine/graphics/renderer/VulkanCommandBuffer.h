 #pragma once

#include "engine/graphics/renderer/VulkanRenderPass.h"
#include "engine/graphics/renderer/VulkanFramebuffer.h"
#include "engine/graphics/renderer/VulkanPipeline.h"
#include "engine/graphics/renderer/VulkanDescriptorSet.h"
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
	void bindVertexBuffer(VulkanBuffer&);
	void bindIndexBuffer(VulkanBuffer&);
	void draw();
	void drawIndexed(uint32_t);
	void endRenderPass();
	void end();

private:
	VkCommandBuffer mCommandBuffer = VK_NULL_HANDLE;
};