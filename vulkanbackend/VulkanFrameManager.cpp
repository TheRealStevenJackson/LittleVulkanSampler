#include "vulkanbackend/VulkanFrameManager.h"

#include <stdexcept>

VulkanFrameManager::VulkanFrameManager(VulkanContext& context, uint32_t framesInFlight, VulkanSwapchain& swapchain)
	:mContext(context),
	mFramesInFlight(framesInFlight),
	mCommandPool(context),
	mSwapchain(swapchain)
{
	// Create command pool
	std::vector<VkCommandBuffer> commandBuffers = mCommandPool.allocate(framesInFlight);

	// Create frame resources
	mFrames = std::vector<Frame>(framesInFlight);
	mFrames.reserve(framesInFlight);
	for (uint32_t i = 0; i < framesInFlight; i++) {
		// Create fence
		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		if (vkCreateFence(mContext.device(), &fenceInfo, nullptr, &(mFrames[i].inFlightFence)) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create fence.");
		}

		// Create semaphores
		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		if (vkCreateSemaphore(mContext.device(), &semaphoreInfo, nullptr, &mFrames[i].imageAvailable) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create semaphore.");
		}
		if (vkCreateSemaphore(mContext.device(), &semaphoreInfo, nullptr, &mFrames[i].renderFinished) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create semaphore.");
		}

		// Attach command buffer
		mFrames[i].commandBuffer = commandBuffers[i];
	}
}

VulkanFrameManager::~VulkanFrameManager()
{
	for (auto& frame : mFrames) {
		if (frame.inFlightFence) {
			vkDestroyFence(mContext.device(), frame.inFlightFence, nullptr);
		}
		if (frame.imageAvailable) {
			vkDestroySemaphore(mContext.device(), frame.imageAvailable, nullptr);
		}
		if (frame.renderFinished) {
			vkDestroySemaphore(mContext.device(), frame.renderFinished, nullptr);
		}
	}
}

uint32_t VulkanFrameManager::beginFrame()
{
	Frame& frame = mFrames[mCurrentFrame];

	vkWaitForFences(
		mContext.device(),
		1, &frame.inFlightFence,
		VK_TRUE, UINT64_MAX
	);

	vkResetFences(mContext.device(), 1, &frame.inFlightFence);

	uint32_t imageIndex = 0;
	VkResult res = vkAcquireNextImageKHR(
		mContext.device(),
		mSwapchain.getHandle(),
		UINT64_MAX,
		frame.imageAvailable,
		VK_NULL_HANDLE,
		&imageIndex
	);

	if (res == VK_ERROR_OUT_OF_DATE_KHR) {
		throw std::runtime_error("Swapchain out of date.");
	}
	if (res != VK_SUCCESS && res != VK_SUBOPTIMAL_KHR) {
		throw std::runtime_error("Failed to acquire swapchain image.");
	}
	
	if (vkResetCommandBuffer(frame.commandBuffer, 0) != VK_SUCCESS) {
		throw std::runtime_error("Failed to reset command buffer.");
	}

	return mCurrentFrame;
}

VkCommandBuffer VulkanFrameManager::getCommandBuffer()
{
	return mFrames[mCurrentFrame].commandBuffer;
}

void VulkanFrameManager::endFrame(VkCommandBuffer cmd, uint32_t imageIndex)
{
	Frame& frame = mFrames[mCurrentFrame];

	if (vkEndCommandBuffer(cmd) != VK_SUCCESS) {
		throw std::runtime_error("Failed to record command buffer.");
	}

	VkSemaphore waitSem = frame.imageAvailable;
	
	VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	VkSemaphore signalSem = frame.renderFinished;

	VkSubmitInfo submit{};
	submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit.waitSemaphoreCount = 1;
	submit.pWaitSemaphores = &waitSem;
	submit.pWaitDstStageMask = &waitStage;
	submit.commandBufferCount = 1;
	submit.pCommandBuffers = &cmd;
	submit.signalSemaphoreCount = 1;
	submit.pSignalSemaphores = &signalSem;

	if (vkQueueSubmit(mContext.graphicsQueue(), 1, &submit, frame.inFlightFence) != VK_SUCCESS) {
		throw std::runtime_error("Failed to submit command buffer.");
	}

	VkSwapchainKHR sc = mSwapchain.getHandle();

	VkPresentInfoKHR present{};
	present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present.waitSemaphoreCount = 1;
	present.pWaitSemaphores = &frame.renderFinished;
	present.swapchainCount = 1;
	present.pSwapchains = &sc;
	present.pImageIndices = &imageIndex;

	VkResult res = vkQueuePresentKHR(mContext.presentQueue(), &present);
	if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR) {
		throw std::runtime_error("Swapchain out of date.");
	}

	mCurrentFrame = (mCurrentFrame + 1) % mFramesInFlight;
}

