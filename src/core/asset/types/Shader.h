#pragma once

#include "platform/graphics/vulkan/VulkanContext.h"
#include "platform/graphics/vulkan/VulkanShaderModule.h"

#include <memory>
#include <string>
#include <vector>
#include <vulkan/vulkan.h>

class Shader {
public:
	/** Construct from SPIR-V byte code (e.g. from ShaderLoader::readFile). */
	Shader(VulkanContext& context, const std::vector<char>& code, std::string sourcePath = {});

	Shader(const Shader&) = delete;
	Shader& operator=(const Shader&) = delete;

	Shader(Shader&&) noexcept = default;
	Shader& operator=(Shader&&) noexcept = default;

	~Shader() = default;

	VulkanShaderModule* module() const { return mModule.get(); }
	VkShaderModule vkModule() const { return mModule ? mModule->module() : VK_NULL_HANDLE; }

	const std::string& sourcePath() const { return mSourcePath; }
	void setSourcePath(std::string path) { mSourcePath = std::move(path); }

	bool isValid() const { return mModule != nullptr; }

private:
	VulkanContext& mContext;
	std::unique_ptr<VulkanShaderModule> mModule;
	std::string mSourcePath;
};
