#pragma once

#include "engine/graphics/renderer/VulkanContext.h"
#include "engine/graphics/renderer/VulkanCommandPool.h"
#include "engine/graphics/renderer/VulkanCommandBuffer.h"
#include "engine/graphics/renderer/VulkanSwapchain.h"

class VulkanFrameManager {
public:
	VulkanFrameManager(VulkanContext&, uint32_t, VulkanSwapchain&);
	~VulkanFrameManager();

	uint32_t beginFrame();
	VkCommandBuffer getCommandBuffer();
	void endFrame(VkCommandBuffer, uint32_t);

private:
	struct Frame {
		VkFence inFlightFence;
		VkSemaphore imageAvailable;
		VkSemaphore renderFinished;
		VkCommandBuffer commandBuffer;
	};

	VulkanContext& mContext;
	VulkanSwapchain& mSwapchain;
	uint32_t mFramesInFlight = 0;
	uint32_t mCurrentFrame = 0;
	VulkanCommandPool mCommandPool;

	std::vector<Frame> mFrames;
};