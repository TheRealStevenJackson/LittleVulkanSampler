#pragma once

#include "core/asset/types/Material.h"
#include "platform/graphics/vulkan/VulkanContext.h"

#include <memory>
#include <string>

class ImageLoader;

class MaterialLoader {
public:
	explicit MaterialLoader(VulkanContext& context);
	~MaterialLoader();

	MaterialLoader(const MaterialLoader&) = delete;
	MaterialLoader& operator=(const MaterialLoader&) = delete;

	/**
	 * Load a material from texture paths using ImageLoader.
	 * Loads albedo, normal, metallic, roughness from paths; sets AO to 1x1 white texture.
	 * Returns nullptr on failure.
	 */
	std::unique_ptr<Material> loadMaterial(const MaterialPaths& paths);

private:
	VulkanContext& mContext;
	std::unique_ptr<ImageLoader> mImageLoader;
};
