#include "core/asset/types/Shader.h"

Shader::Shader(VulkanContext& context, const std::vector<char>& code, std::string sourcePath)
	: mContext(context)
	, mModule(std::make_unique<VulkanShaderModule>(context, code))
	, mSourcePath(std::move(sourcePath))
{
}
