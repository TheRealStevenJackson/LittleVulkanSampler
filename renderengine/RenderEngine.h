#pragma once

#include <memory>
#include "platform/Window.h"
#include "vulkanbackend/VulkanContext.h"
#include "vulkanbackend/VulkanSwapchain.h"

class RenderEngine {
public:
	explicit RenderEngine(Window& window);
	~RenderEngine();

	void drawFrame();

	void onResize();

private:
	Window& window_;
	VulkanContext context_;
	std::unique_ptr<VulkanSwapchain> swapchain_;
};