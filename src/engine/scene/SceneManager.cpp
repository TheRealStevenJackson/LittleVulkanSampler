#include "engine/scene/SceneManager.h"

#include <algorithm>
#include <cctype>

static bool pathLooksLikeGltf(const std::string& path) {
	std::string ext;
	auto pos = path.find_last_of('.');
	if (pos != std::string::npos && pos + 1 < path.size()) {
		ext = path.substr(pos + 1);
		for (auto& c : ext) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
	}
	return ext == "gltf" || ext == "glb";
}

SceneManager::SceneManager(AssetManager& assetManager)
	: m_assetManager(&assetManager)
	, m_loader(assetManager)
{
}

bool SceneManager::loadScene(const std::string& sceneName,
	const std::vector<std::string>& pathsToTry)
{
	auto resource = loadScene(pathsToTry);
	if (!resource.has_value())
		return false;
	m_scenes[sceneName] = std::move(*resource);
	return true;
}

std::optional<SceneResource> SceneManager::loadScene(const std::vector<std::string>& pathsToTry)
{
	if (pathsToTry.empty())
		return std::nullopt;

	// If any path looks like glTF/GLB, try glTF first; otherwise try OBJ first.
	bool tryGltfFirst = std::any_of(pathsToTry.begin(), pathsToTry.end(), pathLooksLikeGltf);

	SceneResource out;
	std::string loadedPath;

	if (tryGltfFirst) {
		if (m_loader.loadGltfFromPaths(pathsToTry, out.entities(), loadedPath)) {
			out.setLoadedPath(loadedPath);
			return out;
		}
		if (m_loader.loadFromPaths(pathsToTry, out.entities(), loadedPath)) {
			out.setLoadedPath(loadedPath);
			return out;
		}
	} else {
		if (m_loader.loadFromPaths(pathsToTry, out.entities(), loadedPath)) {
			out.setLoadedPath(loadedPath);
			return out;
		}
		if (m_loader.loadGltfFromPaths(pathsToTry, out.entities(), loadedPath)) {
			out.setLoadedPath(loadedPath);
			return out;
		}
	}

	return std::nullopt;
}

SceneResource* SceneManager::getScene(const std::string& sceneName) {
	auto it = m_scenes.find(sceneName);
	return it != m_scenes.end() ? &it->second : nullptr;
}

const SceneResource* SceneManager::getScene(const std::string& sceneName) const {
	auto it = m_scenes.find(sceneName);
	return it != m_scenes.end() ? &it->second : nullptr;
}

void SceneManager::setCurrentScene(const std::string& sceneName) {
	SceneResource* r = getScene(sceneName);
	m_currentScene = r;
}

void SceneManager::unloadScene(const std::string& sceneName) {
	if (m_currentScene && m_scenes.count(sceneName)) {
		// If we're unloading the current scene, clear current pointer
		if (getScene(sceneName) == m_currentScene)
			m_currentScene = nullptr;
	}
	m_scenes.erase(sceneName);
}
