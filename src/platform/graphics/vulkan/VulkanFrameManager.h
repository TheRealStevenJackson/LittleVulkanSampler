#pragma once

#include "platform/graphics/vulkan/VulkanContext.h"
#include "platform/graphics/vulkan/VulkanCommandPool.h"
#include "platform/graphics/vulkan/VulkanCommandBuffer.h"
#include "platform/graphics/vulkan/VulkanSwapchain.h"

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