#pragma once

#include "platform/graphics/vulkan/VulkanContext.h"
#include "platform/graphics/vulkan/VulkanImage.h"

#include <memory>
#include <string>
#include <vulkan/vulkan.h>

class Texture {
public:
	/** Construct from an existing VulkanImage (takes ownership). */
	Texture(VulkanContext& context, std::unique_ptr<VulkanImage> image);
	/** Optional source path for debugging/cache keys. */
	Texture(VulkanContext& context, std::unique_ptr<VulkanImage> image, std::string sourcePath);

	Texture(const Texture&) = delete;
	Texture& operator=(const Texture&) = delete;

	Texture(Texture&&) noexcept = default;
	Texture& operator=(Texture&&) noexcept = default;

	~Texture() = default;

	VulkanImage* image() const { return mImage.get(); }
	VkImage vkImage() const { return mImage ? mImage->image() : VK_NULL_HANDLE; }
	VkImageView imageView() const { return mImage ? mImage->imageView() : VK_NULL_HANDLE; }
	VkFormat format() const { return mImage ? mImage->format() : VK_FORMAT_UNDEFINED; }
	VkExtent2D extent() const { return mImage ? mImage->extent() : VkExtent2D{0, 0}; }
	uint32_t width() const { return extent().width; }
	uint32_t height() const { return extent().height; }

	const std::string& sourcePath() const { return mSourcePath; }
	void setSourcePath(std::string path) { mSourcePath = std::move(path); }

	bool isValid() const { return mImage != nullptr; }

private:
	VulkanContext& mContext;
	std::unique_ptr<VulkanImage> mImage;
	std::string mSourcePath;
};
