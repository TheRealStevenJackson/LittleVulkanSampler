#pragma once

#include "core/asset/types/Shader.h"
#include "platform/graphics/vulkan/VulkanContext.h"

#include <memory>
#include <string>
#include <vector>

class ShaderLoader {
public:
	explicit ShaderLoader(VulkanContext& context);

	ShaderLoader(const ShaderLoader&) = delete;
	ShaderLoader& operator=(const ShaderLoader&) = delete;

	/**
	 * Read shader file contents (e.g. .vert.spv or .frag.spv).
	 * Throws std::runtime_error if the file cannot be opened.
	 */
	static std::vector<char> readFile(const std::string& filepath);

	/**
	 * Load a shader from a file path (e.g. .vert.spv or .frag.spv).
	 * Returns nullptr on failure (file not found or invalid SPIR-V).
	 */
	std::unique_ptr<Shader> loadShader(const std::string& filepath);

private:
	VulkanContext& mContext;
};
