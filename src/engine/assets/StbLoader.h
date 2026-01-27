#pragma once

#include "src/engine/graphics/Material.h"
#include "engine/graphics/renderer/VulkanContext.h"

#include <memory>
#include <string>

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
	 * Load an image file using stb_image and create a VulkanImage.
	 * Returns nullptr on failure.
	 */
	std::unique_ptr<VulkanImage> loadImage(const std::string& filepath);

private:

	VulkanContext& mContext;
};
