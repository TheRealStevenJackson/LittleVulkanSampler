#include "core/asset/types/Texture.h"

Texture::Texture(VulkanContext& context, std::unique_ptr<VulkanImage> image)
	: mContext(context)
	, mImage(std::move(image))
{
}

Texture::Texture(VulkanContext& context, std::unique_ptr<VulkanImage> image, std::string sourcePath)
	: mContext(context)
	, mImage(std::move(image))
	, mSourcePath(std::move(sourcePath))
{
}
