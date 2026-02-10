#include "engine/scene/SceneLoader.h"

SceneLoader::SceneLoader(AssetManager& assetManager)
	: m_assetManager(&assetManager)
{
}

bool SceneLoader::loadFromPaths(const std::vector<std::string>&,
	std::vector<Entity>&,
	std::string& outLoadedPath)
{
	outLoadedPath.clear();
	return false;
}

std::optional<Entity> SceneLoader::loadOneFromPaths(const std::vector<std::string>&,
	std::string* outLoadedPath)
{
	if (outLoadedPath) outLoadedPath->clear();
	return std::nullopt;
}

bool SceneLoader::loadGltfFromPaths(const std::vector<std::string>&,
	std::vector<Entity>&,
	std::string& outLoadedPath)
{
	outLoadedPath.clear();
	return false;
}

std::optional<Entity> SceneLoader::loadOneGltfFromPaths(const std::vector<std::string>&,
	std::string* outLoadedPath)
{
	if (outLoadedPath) outLoadedPath->clear();
	return std::nullopt;
}
