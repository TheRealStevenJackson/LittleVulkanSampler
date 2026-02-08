#pragma once

#include "core/asset/Material.h"
#include "platform/graphics/vulkan/VulkanContext.h"
#include "platform/graphics/vulkan/VulkanImage.h"

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

class StbLoader {
public:
	explicit StbLoader(VulkanContext& context);
	~StbLoader() = default;

	// Non-copyable
	StbLoader(const StbLoader&) = delete;
	StbLoader& operator=(const StbLoader&) = delete;

	/**
	 * Load textures from MaterialPaths and create a Material.
	 * Returns nullptr on failure.
	 */
	std::unique_ptr<Material> loadMaterial(const MaterialPaths& paths);

	/**
	 * Load an image file using stb_image and create a VulkanImage with ImageView and Sampler.
	 * Returns empty struct (with nullptr image) on failure.
	 */
	LoadedImage loadImage(const std::string& filepath);

	/**
	 * Create a 1x1 white texture (RGBA = 1.0, 1.0, 1.0, 1.0).
	 * Returns nullptr on failure.
	 */
	std::unique_ptr<VulkanImage> createWhiteTexture();

private:
	VkSampler createSampler(VulkanContext& context);

	VulkanContext& mContext;
};
