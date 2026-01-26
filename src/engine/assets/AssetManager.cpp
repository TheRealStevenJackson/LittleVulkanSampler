#include "engine/assets/AssetManager.h"

AssetManager::AssetManager(VulkanContext& context)
	: mContext(context)
	, mObjLoader(context)
{
}

MeshId AssetManager::nextMeshId() {
	return m_nextMeshId++;
}

std::vector<MeshId> AssetManager::loadObj(const std::string& filepath) {
	std::vector<MeshId> ids;
	auto meshes = mObjLoader.loadFromFile(filepath);
	for (auto& mesh : meshes) {
		MeshId id = nextMeshId();
		m_meshMap[id] = std::move(mesh);
		ids.push_back(id);
	}
	return ids;
}

MeshId AssetManager::loadObjCombined(const std::string& filepath) {
	auto mesh = mObjLoader.loadFromFileCombined(filepath);
	if (!mesh)
		return InvalidMeshId;
	MeshId id = nextMeshId();
	m_meshMap[id] = std::move(mesh);
	return id;
}

bool AssetManager::loadObjFromPaths(const std::vector<std::string>& pathsToTry,
	std::vector<MeshId>& outMeshIds,
	std::string& outLoadedPath)
{
	for (const auto& path : pathsToTry) {
		auto ids = loadObj(path);
		if (!ids.empty()) {
			outMeshIds = std::move(ids);
			outLoadedPath = path;
			return true;
		}
	}
	outMeshIds.clear();
	outLoadedPath.clear();
	return false;
}

Mesh* AssetManager::getMesh(MeshId id) {
	auto it = m_meshMap.find(id);
	return it != m_meshMap.end() ? it->second.get() : nullptr;
}

const Mesh* AssetManager::getMesh(MeshId id) const {
	auto it = m_meshMap.find(id);
	return it != m_meshMap.end() ? it->second.get() : nullptr;
}
