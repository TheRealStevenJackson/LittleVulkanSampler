 #pragma once

#include "vulkanbackend/VulkanRenderPass.h"
#include "vulkanbackend/VulkanFramebuffer.h"
#include "vulkanbackend/VulkanPipeline.h"
#include "vulkanbackend/VulkanDescriptorSet.h"

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
	void bindVertexBuffer(VulkanBuffer&);
	void bindIndexBuffer(VulkanBuffer&);
	void draw();
	void drawIndexed(uint32_t);
	void endRenderPass();
	void end();

private:
	VkCommandBuffer mCommandBuffer = VK_NULL_HANDLE;
};