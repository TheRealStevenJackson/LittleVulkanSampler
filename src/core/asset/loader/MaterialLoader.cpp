#include "core/asset/loader/MaterialLoader.h"
#include "core/asset/loader/ImageLoader.h"
#include "core/asset/types/Texture.h"

#include <iostream>

MaterialLoader::MaterialLoader(VulkanContext& context)
	: mContext(context)
	, mImageLoader(std::make_unique<ImageLoader>(context))
{
}

MaterialLoader::~MaterialLoader() = default;

std::unique_ptr<Material> MaterialLoader::loadMaterial(const MaterialPaths& paths)
{
	auto material = std::make_unique<Material>(mContext, paths);

	if (!paths.albedoPath.empty()) {
		auto loadedImage = mImageLoader->loadImage(paths.albedoPath);
		if (loadedImage.image) {
			material->setAlbedoMap(std::make_unique<Texture>(mContext, std::move(loadedImage.image), paths.albedoPath));
			std::cout << "Set albedo map: " << paths.albedoPath << std::endl;
		}
	}
	if (!paths.normalPath.empty()) {
		auto loadedImage = mImageLoader->loadImage(paths.normalPath);
		if (loadedImage.image) {
			material->setNormalMap(std::make_unique<Texture>(mContext, std::move(loadedImage.image), paths.normalPath));
			std::cout << "Set normal map: " << paths.normalPath << std::endl;
		}
	}
	if (!paths.metallicPath.empty()) {
		auto loadedImage = mImageLoader->loadImage(paths.metallicPath);
		if (loadedImage.image) {
			material->setMetallicMap(std::make_unique<Texture>(mContext, std::move(loadedImage.image), paths.metallicPath));
			std::cout << "Set metallic map: " << paths.metallicPath << std::endl;
		}
	}
	if (!paths.roughnessPath.empty()) {
		auto loadedImage = mImageLoader->loadImage(paths.roughnessPath);
		if (loadedImage.image) {
			material->setRoughnessMap(std::make_unique<Texture>(mContext, std::move(loadedImage.image), paths.roughnessPath));
			std::cout << "Set roughness map: " << paths.roughnessPath << std::endl;
		}
	}

	auto aoImage = mImageLoader->createWhiteTexture();
	if (aoImage) {
		material->setAoMap(std::make_unique<Texture>(mContext, std::move(aoImage)));
		std::cout << "Set AO map: 1x1 white texture (1.0)" << std::endl;
	}

	return material;
}
