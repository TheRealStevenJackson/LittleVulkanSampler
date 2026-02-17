#pragma once

#include "platform/graphics/vulkan/VulkanContext.h"
#include "platform/graphics/vulkan/VulkanImage.h"

#include <cstddef>
#include <memory>
#include <string>
#include <vulkan/vulkan.h>

// Structure containing image, image view, and sampler for a loaded texture
struct LoadedImage {
	std::unique_ptr<VulkanImage> image;
	VkImageView imageView;
	VkSampler sampler;
	VkDevice device;  // Needed for cleanup

	LoadedImage() : imageView(VK_NULL_HANDLE), sampler(VK_NULL_HANDLE), device(VK_NULL_HANDLE) {}
	
	// Move constructor
	LoadedImage(LoadedImage&& other) noexcept
		: image(std::move(other.image))
		, imageView(other.imageView)
		, sampler(other.sampler)
		, device(other.device)
	{
		other.imageView = VK_NULL_HANDLE;
		other.sampler = VK_NULL_HANDLE;
		other.device = VK_NULL_HANDLE;
	}
	
	// Move assignment
	LoadedImage& operator=(LoadedImage&& other) noexcept {
		if (this != &other) {
			// Clean up existing sampler
			if (sampler != VK_NULL_HANDLE && device != VK_NULL_HANDLE) {
				vkDestroySampler(device, sampler, nullptr);
			}
			image = std::move(other.image);
			imageView = other.imageView;
			sampler = other.sampler;
			device = other.device;
			other.imageView = VK_NULL_HANDLE;
			other.sampler = VK_NULL_HANDLE;
			other.device = VK_NULL_HANDLE;
		}
		return *this;
	}
	
	// Non-copyable
	LoadedImage(const LoadedImage&) = delete;
	LoadedImage& operator=(const LoadedImage&) = delete;
	
	// Destructor - destroys sampler if valid
	~LoadedImage() {
		// Image and imageView are destroyed by VulkanImage destructor
		// Only need to destroy sampler if it was created
		if (sampler != VK_NULL_HANDLE && device != VK_NULL_HANDLE) {
			vkDestroySampler(device, sampler, nullptr);
		}
	}
};

class ImageLoader {
public:
	explicit ImageLoader(VulkanContext& context);
	~ImageLoader() = default;

	// Non-copyable
	ImageLoader(const ImageLoader&) = delete;
	ImageLoader& operator=(const ImageLoader&) = delete;

	/**
	 * Load an image file using stb_image and create a VulkanImage with ImageView and Sampler.
	 * Returns empty struct (with nullptr image) on failure.
	 */
	LoadedImage loadImage(const std::string& filepath);

	/**
	 * Load an image from memory (e.g. embedded GLB buffer) using stbi_load_from_memory.
	 * Returns empty struct (with nullptr image) on failure.
	 */
	LoadedImage loadImageFromMemory(const uint8_t* data, size_t size);

	/**
	 * Create a 1x1 white texture (RGBA = 1.0, 1.0, 1.0, 1.0).
	 * Returns nullptr on failure.
	 */
	std::unique_ptr<VulkanImage> createWhiteTexture();

private:
	VkSampler createSampler(VulkanContext& context);

	VulkanContext& mContext;
};
