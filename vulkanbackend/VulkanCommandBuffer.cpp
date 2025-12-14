#include "vulkanbackend/VulkanCommandBuffer.h"

#include <stdexcept>
#include <array>

VulkanCommandBuffer::VulkanCommandBuffer(VkCommandBuffer cmd)
	: mCommandBuffer(cmd)
{
}

VulkanCommandBuffer::~VulkanCommandBuffer()
{
}

void VulkanCommandBuffer::begin()
{
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	if (vkBeginCommandBuffer(mCommandBuffer, &beginInfo) != VK_SUCCESS) {
		throw std::runtime_error("Failed to begin command buffer.");
	}
}

void VulkanCommandBuffer::beginRenderPass(VulkanRenderPass& renderPass, VulkanFramebuffer& framebuffer)
{
	VkRect2D renderArea;
	renderArea.extent.width = 720;
	renderArea.extent.height = 720;
	renderArea.offset = { 0, 0 };

	std::array<VkClearValue, 2> clearValues{};
	clearValues[0].color = {{0.1f, 0.1f, 0.1f, 1.0f}};
	clearValues[1].depthStencil = { 1.0f, 0 };

	VkRenderPassBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	beginInfo.renderPass = renderPass.renderPass();
	beginInfo.framebuffer = framebuffer.framebuffer();
	beginInfo.renderArea = renderArea;
	beginInfo.clearValueCount = clearValues.size();
	beginInfo.pClearValues = clearValues.data();

	vkCmdBeginRenderPass(mCommandBuffer, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void VulkanCommandBuffer::bindPipeline(VulkanPipeline& pipeline)
{
	vkCmdBindPipeline(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipeline());
}

void VulkanCommandBuffer::setViewport(VkExtent2D extent)
{
	VkViewport viewport{};
	viewport.x = 0;
	viewport.y = 0;
	viewport.width = extent.width;
	viewport.height = extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	vkCmdSetViewport(mCommandBuffer, 0, 1, &viewport);
}

void VulkanCommandBuffer::setScissor(VkExtent2D extent)
{
	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = extent;

	vkCmdSetScissor(mCommandBuffer, 0, 1, &scissor);
}

void VulkanCommandBuffer::bindDescriptorSet(VulkanPipelineLayout& pipelineLayout, VulkanDescriptorSet& descriptorSet)
{
	VkDescriptorSet set = descriptorSet.descriptorSet();
	vkCmdBindDescriptorSets(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout.pipelineLayout(),
		0, 1, &set, 0, nullptr);
}

void VulkanCommandBuffer::bindVertexBuffer(VulkanBuffer& vertexBuffer)
{
	VkBuffer vb = vertexBuffer.buffer();
	VkDeviceSize offsets[] = { 0 };

	vkCmdBindVertexBuffers(mCommandBuffer, 0, 1, &vb, offsets);
}

void VulkanCommandBuffer::bindIndexBuffer(VulkanBuffer& indexBuffer)
{
	vkCmdBindIndexBuffer(mCommandBuffer, indexBuffer.buffer(), 0, VK_INDEX_TYPE_UINT16);
}

void VulkanCommandBuffer::draw()
{
}

void VulkanCommandBuffer::drawIndexed(uint32_t size)
{
	vkCmdDrawIndexed(mCommandBuffer, size, 1, 0, 0, 0);
}

void VulkanCommandBuffer::endRenderPass()
{
	vkCmdEndRenderPass(mCommandBuffer);
}

void VulkanCommandBuffer::end()
{
}
