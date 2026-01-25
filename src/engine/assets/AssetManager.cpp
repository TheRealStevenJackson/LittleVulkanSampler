#include "engine/assets/AssetManager.h"

AssetManager::AssetManager(VulkanContext& context)
	: mContext(context)
	, mObjLoader(context)
{
}

std::vector<std::unique_ptr<Mesh>> AssetManager::loadObj(const std::string& filepath) {
	return mObjLoader.loadFromFile(filepath);
}

std::unique_ptr<Mesh> AssetManager::loadObjCombined(const std::string& filepath) {
	return mObjLoader.loadFromFileCombined(filepath);
}

bool AssetManager::loadObjFromPaths(const std::vector<std::string>& pathsToTry,
	std::vector<std::unique_ptr<Mesh>>& outMeshes,
	std::string& outLoadedPath)
{
	for (const auto& path : pathsToTry) {
		auto meshes = mObjLoader.loadFromFile(path);
		if (!meshes.empty()) {
			outMeshes = std::move(meshes);
			outLoadedPath = path;
			return true;
		}
	}
	outMeshes.clear();
	outLoadedPath.clear();
	return false;
}
