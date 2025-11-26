#include "RenderEngine.h"

//#include <vulkan/vulkan.h>
#include <stdexcept>
#include <iostream>

RenderEngine::RenderEngine(Window& window)
	: window_(window), context_(window.getHandle()) {
	swapchain_ = std::make_unique<VulkanSwapchain>(context_, window_.getHandle());
}

RenderEngine::~RenderEngine() {
	vkDeviceWaitIdle(context_.device());

	swapchain_.reset();

	std::cout << "[RenderEngine] Shutdown complete." << std::endl;
}

void RenderEngine::drawFrame() {

}

void RenderEngine::onResize() {
	vkDeviceWaitIdle(context_.device());
	swapchain_.reset();
	swapchain_ = std::make_unique<VulkanSwapchain>(context_, window_.getHandle());

	std::cout << "[RenderEngine] Swapchain recreated after resize." << std::endl;
}