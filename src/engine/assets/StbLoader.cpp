#define STB_IMAGE_IMPLEMENTATION
#include "engine/assets/StbLoader.h"

#include "engine/graphics/renderer/VulkanImage.h"
#include "engine/graphics/renderer/VulkanMemory.h"

#include <stb_image.h>
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <filesystem>
#include <iostream>
#include <stdexcept>

StbLoader::StbLoader(VulkanContext& context)
	: mContext(context)
{
}

std::unique_ptr<Material> StbLoader::loadMaterial(const MaterialPaths& paths)
{
	auto material = std::make_unique<Material>(mContext, paths);

	// Load albedo texture
	if (!paths.albedoPath.empty()) {
		auto albedoImage = loadImage(paths.albedoPath);
		if (albedoImage) {
			material->setAlbedoMap(std::move(albedoImage));
		}
	}

	// Load normal map
	if (!paths.normalPath.empty()) {
		auto normalImage = loadImage(paths.normalPath);
		if (normalImage) {
			material->setNormalMap(std::move(normalImage));
		}
	}

	// Load metallic texture
	if (!paths.metallicPath.empty()) {
		auto metallicImage = loadImage(paths.metallicPath);
		if (metallicImage) {
			material->setMetallicMap(std::move(metallicImage));
		}
	}

	// Load roughness texture
	if (!paths.roughnessPath.empty()) {
		auto roughnessImage = loadImage(paths.roughnessPath);
		if (roughnessImage) {
			material->setRoughnessMap(std::move(roughnessImage));
		}
	}

	return material;
}

std::unique_ptr<VulkanImage> StbLoader::loadImage(const std::string& filepath)
{
	// Check if file exists
	if (!std::filesystem::exists(filepath)) {
		std::cerr << "Image file not found: " << filepath << std::endl;
		return nullptr;
	}

	// Load image using stb_image
	int width, height, channels;
	stbi_uc* pixels = stbi_load(filepath.c_str(), &width, &height, &channels, STBI_rgb_alpha);

	if (!pixels) {
		std::cerr << "Failed to load image: " << filepath << " - " << stbi_failure_reason() << std::endl;
		return nullptr;
	}

	// Free stb_image data (we'll need to reload it when actually uploading)
	// For now, we'll just validate the image can be loaded
	stbi_image_free(pixels);

	// TODO: Implement full image creation and upload
	// This requires:
	// 1. Create VkImage with proper format and usage flags
	// 2. Create staging buffer and upload pixel data
	// 3. Use command buffer to copy from staging to image
	// 4. Transition image layout to SHADER_READ_ONLY_OPTIMAL
	// 5. Create VulkanImage wrapper
	// 
	// For now, return nullptr to indicate image loading is not yet fully implemented
	// The Material will use uniform values instead of textures

	std::cout << "Image loaded successfully: " << filepath << " (" << width << "x" << height << ", " << channels << " channels)" << std::endl;
	std::cout << "Note: Texture upload to GPU not yet implemented - Material will use uniform values" << std::endl;

	return nullptr;
}
