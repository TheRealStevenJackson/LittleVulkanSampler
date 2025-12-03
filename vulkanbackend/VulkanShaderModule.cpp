#include "vulkanbackend/VulkanShaderModule.h"
#include "vulkanbackend/VulkanContext.h"

#include <fstream>
#include <stdexcept>
#include <iostream>

VulkanShaderModule::VulkanShaderModule(const VulkanContext& context, const std::string& filepath)
	: context_(context) {
	auto code = readFile(filepath);
	module_ = createShaderModule(code);

	std::cout << "Created shader module." << std::endl;
}

VulkanShaderModule::~VulkanShaderModule() {
	if (module_ != VK_NULL_HANDLE) {
		vkDestroyShaderModule(context_.device(), module_, nullptr);
	}

	std::cout << "Destroyed shader module." << std::endl;
}

std::vector<char> VulkanShaderModule::readFile(const std::string& filepath) {
	std::ifstream file(filepath, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("Failed to open shader file: " + filepath);
	}

	size_t filesize = static_cast<size_t>(file.tellg());
	std::vector<char> buffer(filesize);

	file.seekg(0);
	file.read(buffer.data(), filesize);
	file.close();

	return buffer;
}

VkShaderModule VulkanShaderModule::createShaderModule(const std::vector<char>& code) {
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(context_.device(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create shader module.");
	}

	return shaderModule;
}