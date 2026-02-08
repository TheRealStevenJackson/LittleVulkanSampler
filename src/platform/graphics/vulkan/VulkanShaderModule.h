#pragma once

#include <vulkan/vulkan.h>
#include <string>
#include <vector>

class VulkanContext;

class VulkanShaderModule {
public:
	VulkanShaderModule(const VulkanContext& context, const std::vector<char>& code);
	~VulkanShaderModule();

	VkShaderModule module() const { return module_; }

private:
	const VulkanContext& context_;
	VkShaderModule module_ = VK_NULL_HANDLE;

	VkShaderModule createShaderModule(const std::vector<char>& code);
};