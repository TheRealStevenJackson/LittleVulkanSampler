#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "vulkanbackend/VulkanMemory.h"

#include <optional>
#include <vector>
#include <memory>

struct SwapchainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> present_modes;
};

struct QueueFamilyIndices {
	std::optional<uint32_t> graphics_family;
	std::optional<uint32_t> present_family;
	bool isComplete() const { return graphics_family.has_value() && present_family.has_value(); }
};

class VulkanContext {
public:
	explicit VulkanContext(GLFWwindow* window);
	~VulkanContext();

	VkInstance instance() const { return instance_; }
	VkSurfaceKHR surface() const { return surface_; }
	VkPhysicalDevice physicalDevice() const { return physical_device_; }
	VkDevice device() const { return device_; }
	VkQueue graphicsQueue() const { return graphics_queue_; }
	VkQueue presentQueue() const { return present_queue_; }
	VulkanMemory& vma() { return *vulkanMemory; }

	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice) const;
	SwapchainSupportDetails querySwapchainSupport(VkPhysicalDevice) const;

	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats) const;
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& modes) const;
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window) const;


private:
	void createInstance();
	void setupDebugMessenger();
	void createSurface(GLFWwindow*);
	void pickPhysicalDevice();
	void createLogicalDevice();

	bool isDeviceSuitable(VkPhysicalDevice) const;

	bool checkValidationLayerSupport();
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT&);

	VkInstance instance_ = VK_NULL_HANDLE;
	VkDebugUtilsMessengerEXT debug_messenger_ = VK_NULL_HANDLE;
	VkSurfaceKHR surface_ = VK_NULL_HANDLE;
	VkPhysicalDevice physical_device_ = VK_NULL_HANDLE;
	VkDevice device_ = VK_NULL_HANDLE;
	VkQueue graphics_queue_ = VK_NULL_HANDLE;
	VkQueue present_queue_ = VK_NULL_HANDLE;
	std::unique_ptr<VulkanMemory> vulkanMemory;
};