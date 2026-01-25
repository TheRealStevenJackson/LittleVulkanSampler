#include "VulkanContext.h"

#include <stdexcept>
#include <iostream>
#include <vector>
#include <cstring>
#include <set>
#include <algorithm>

// -----------------------------------------------------------------------------
// Validation layers
// -----------------------------------------------------------------------------
#ifdef _DEBUG
const bool kEnableValidationLayers = true;
#else
const bool kEnableValidationLayers = false;
#endif

const std::vector<const char*> kValidationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

// -----------------------------------------------------------------------------
// Debug messenger utilities
// -----------------------------------------------------------------------------
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData) {
	std::cerr << "[Vulkan] " << pCallbackData->pMessage << std::endl;
	return VK_FALSE;
}

static VkResult CreateDebugUtilsMessengerEXT(
		VkInstance instance,
		const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator,
		VkDebugUtilsMessengerEXT* pDebugMessenger) {
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)
		vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr)
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	return VK_ERROR_EXTENSION_NOT_PRESENT;
}

static void DestroyDebugUtilsMessengerEXT(
		VkInstance instance,
		VkDebugUtilsMessengerEXT debugMessenger,
		const VkAllocationCallbacks* pAllocator) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)
		vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr)
		func(instance, debugMessenger, pAllocator);
}

// -----------------------------------------------------------------------------
// VukanContext implementation
// -----------------------------------------------------------------------------
VulkanContext::VulkanContext(GLFWwindow* window) {
	createInstance();
	setupDebugMessenger();
	createSurface(window);
	pickPhysicalDevice();
	createLogicalDevice();
	
	vulkanMemory = std::make_unique<VulkanMemory>(
		instance_,
		physical_device_,
		device_
	);

	std::cout << "Vulkan instance and GPU initialized successfully." << std::endl;
}

VulkanContext::~VulkanContext() {
	vulkanMemory.reset();

	if (device_ != VK_NULL_HANDLE)
		vkDestroyDevice(device_, nullptr);

	if (debug_messenger_ != VK_NULL_HANDLE)
		DestroyDebugUtilsMessengerEXT(instance_, debug_messenger_, nullptr);

	if (surface_ != VK_NULL_HANDLE)
		vkDestroySurfaceKHR(instance_, surface_, nullptr);

	if (instance_ != VK_NULL_HANDLE)
		vkDestroyInstance(instance_, nullptr);
}

// -----------------------------------------------------------------------------
// Private methods
// -----------------------------------------------------------------------------
void VulkanContext::createInstance() {
	// Check validation layer support if enabled
	if (kEnableValidationLayers && !checkValidationLayerSupport())
		throw std::runtime_error("Validation layers requested, but not available.");

	// Application info
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Little Vulkan Engine";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "Little Vulkan Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_3;

	// Required extensions from GLFW
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
	if (kEnableValidationLayers)
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

	// Instance create info
	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	if (kEnableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(kValidationLayers.size());
		createInfo.ppEnabledLayerNames = kValidationLayers.data();

		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
		populateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
	}
	else {
		createInfo.enabledLayerCount = 0;
		createInfo.pNext = nullptr;
	}

	if (vkCreateInstance(&createInfo, nullptr, &instance_) != VK_SUCCESS)
		throw std::runtime_error("Failed to create Vulkan instance.");
}

void VulkanContext::setupDebugMessenger() {
	if (!kEnableValidationLayers)
		return;

	VkDebugUtilsMessengerCreateInfoEXT createInfo{};
	populateDebugMessengerCreateInfo(createInfo);

	if (CreateDebugUtilsMessengerEXT(instance_, &createInfo, nullptr, &debug_messenger_) != VK_SUCCESS)
		throw std::runtime_error("Failed to set up debug messenger.");
}

void VulkanContext::createSurface(GLFWwindow* window) {
	if (glfwCreateWindowSurface(instance_, window, nullptr, &surface_) != VK_SUCCESS)
		throw std::runtime_error("Failed to create window surface.");
}

bool VulkanContext::checkValidationLayerSupport() {
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : kValidationLayers) {
		bool found = false;
		for (const auto& layerProperties : availableLayers) {
			if (strcmp(layerName, layerProperties.layerName) == 0) {
				found = true;
				break;
			}
		}
		if (!found)
			return false;
	}
	return true;
}

void VulkanContext::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity =
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType =
		VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
	createInfo.pUserData = nullptr;
}

void VulkanContext::pickPhysicalDevice() {
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance_, &deviceCount, nullptr);
	if (deviceCount == 0)
		throw std::runtime_error("Failed to find GPUs with Vulkan support.");

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance_, &deviceCount, devices.data());

	for (const auto& device : devices) {
		VkPhysicalDeviceProperties props;
		vkGetPhysicalDeviceProperties(device, &props);
		if (isDeviceSuitable(device)) {
			physical_device_ = device;
			break;
		}
	}

	if (physical_device_ == VK_NULL_HANDLE)
		throw std::runtime_error("Failed to find a suitable GPU.");

	VkPhysicalDeviceProperties props;
	vkGetPhysicalDeviceProperties(physical_device_, &props);
	std::cout << "Selected GPU: " << props.deviceName << std::endl;
}

void VulkanContext::createLogicalDevice() {
	QueueFamilyIndices indices = findQueueFamilies(physical_device_);

	// Unique queue families (in case graphics and present are different)
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = {
		indices.graphics_family.value(),
		indices.present_family.value()
	};

	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies) {
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures{};

	// Device create info
	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.pEnabledFeatures = &deviceFeatures;

	// Device extensions (needed for swapchain)
	const std::vector<const char*> deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};
	createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();

	// Validation layers (deprecated for device-level, but still useful for backward compat)
	//if (kEnableValidationLayers) {
	//	createInfo.enabledLayerCount = static_cast<uint32_t> (kValidationLayers.size());
	//	createInfo.ppEnabledExtensionNames = kValidationLayers.data();
	//}
	//else {
	//	createInfo.enabledLayerCount = 0;
	//}

	if (vkCreateDevice(physical_device_, &createInfo, nullptr, &device_) != VK_SUCCESS)
		throw std::runtime_error("Failed to create logical device.");

	vkGetDeviceQueue(device_, indices.graphics_family.value(), 0, &graphics_queue_);
	vkGetDeviceQueue(device_, indices.present_family.value(), 0, &present_queue_);
}

SwapchainSupportDetails VulkanContext::querySwapchainSupport(VkPhysicalDevice device) const {
	SwapchainSupportDetails details;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface_, &details.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &formatCount, nullptr);
	if (formatCount != 0) {
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &formatCount, details.formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface_, &presentModeCount, nullptr);
	if (presentModeCount != 0) {
		details.present_modes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface_, &presentModeCount, details.present_modes.data());
	}

	return details;
}

VkSurfaceFormatKHR VulkanContext::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats) const {
	for (const auto& availableFormat : formats) {
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
			availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return availableFormat;
		}
	}

	return formats[0]; //fallback
}

VkPresentModeKHR VulkanContext::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& modes) const {
	// Mailbox is triple-buffered vsync-off (lowest latency)
	for (const auto& mode : modes) {
		if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
			return mode;
	}
	// Fallback to guaranteed mode
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanContext::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window) const {
	if (capabilities.currentExtent.width != UINT32_MAX) {
		return capabilities.currentExtent;
	}
	else {
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);

		VkExtent2D actualExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		actualExtent.width = std::clamp(actualExtent.width,
			capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height,
			capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actualExtent;
	}
}

bool VulkanContext::isDeviceSuitable(VkPhysicalDevice device) const {
	QueueFamilyIndices indices = findQueueFamilies(device);
	return indices.isComplete();
}

QueueFamilyIndices VulkanContext::findQueueFamilies(VkPhysicalDevice device) const {
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const auto& queueFamily : queueFamilies) {
		// Check for graphics support
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphics_family = i;
		}

		// Check for presentation support
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface_, &presentSupport);
		if (presentSupport) {
			indices.present_family = i;
		}

		if (indices.isComplete())
			break;

		i++;
	}

	return indices;
}