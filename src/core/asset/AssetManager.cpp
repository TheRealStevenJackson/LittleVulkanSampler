#include "core/asset/AssetManager.h"
#include "core/asset/types/Texture.h"

#include <iostream>

AssetManager::AssetManager(VulkanContext& context)
	: mContext(context)
	, mMeshLoader(context)
	, mImageLoader(context)
{
}

MeshId AssetManager::nextMeshId() {
	return m_nextMeshId++;
}

MaterialId AssetManager::nextMaterialId() {
	return m_nextMaterialId++;
}

std::vector<MeshId> AssetManager::loadObj(const std::string& filepath) {
	std::vector<MeshId> ids;
	auto meshes = mMeshLoader.loadFromFile(filepath);
	for (auto& mesh : meshes) {
		MeshId id = nextMeshId();
		m_meshMap[id] = std::move(mesh);
		ids.push_back(id);
	}
	return ids;
}

MeshId AssetManager::loadObjCombined(const std::string& filepath) {
	auto mesh = mMeshLoader.loadFromFileCombined(filepath);
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

Material* AssetManager::getMaterial(MaterialId id) {
	auto it = m_materialMap.find(id);
	return it != m_materialMap.end() ? it->second.get() : nullptr;
}

const Material* AssetManager::getMaterial(MaterialId id) const {
	auto it = m_materialMap.find(id);
	return it != m_materialMap.end() ? it->second.get() : nullptr;
}

std::vector<MaterialId> AssetManager::loadMaterials(const std::string& filepath)
{
	std::vector<MaterialId> materialIds;
	
	// Extract material paths from the OBJ file's MTL
	std::vector<MaterialPaths> paths = mMeshLoader.extractMaterialPaths(filepath);
	
	for (const auto& path : paths) {
		// Create a material for this MaterialPaths
		auto material = std::make_unique<Material>(mContext, path);
		
		// Load and set each texture image if the path is not empty
		if (!path.albedoPath.empty()) {
			auto loadedImage = mImageLoader.loadImage(path.albedoPath);
			if (loadedImage.image) {
				material->setAlbedoMap(std::make_unique<Texture>(mContext, std::move(loadedImage.image), path.albedoPath));
				std::cout << "Set albedo map: " << path.albedoPath << std::endl;
			}
		}
		if (!path.normalPath.empty()) {
			auto loadedImage = mImageLoader.loadImage(path.normalPath);
			if (loadedImage.image) {
				material->setNormalMap(std::make_unique<Texture>(mContext, std::move(loadedImage.image), path.normalPath));
				std::cout << "Set normal map: " << path.normalPath << std::endl;
			}
		}
		if (!path.metallicPath.empty()) {
			auto loadedImage = mImageLoader.loadImage(path.metallicPath);
			if (loadedImage.image) {
				material->setMetallicMap(std::make_unique<Texture>(mContext, std::move(loadedImage.image), path.metallicPath));
				std::cout << "Set metallic map: " << path.metallicPath << std::endl;
			}
		}
		if (!path.roughnessPath.empty()) {
			auto loadedImage = mImageLoader.loadImage(path.roughnessPath);
			if (loadedImage.image) {
				material->setRoughnessMap(std::make_unique<Texture>(mContext, std::move(loadedImage.image), path.roughnessPath));
				std::cout << "Set roughness map: " << path.roughnessPath << std::endl;
			}
		}
		
		// Set default AO map as 1x1 white texture (value 1.0)
		auto aoImage = mImageLoader.createWhiteTexture();
		if (aoImage) {
			material->setAoMap(std::make_unique<Texture>(mContext, std::move(aoImage)));
			std::cout << "Set AO map: 1x1 white texture (1.0)" << std::endl;
		}
		
		// Store the material
		MaterialId id = nextMaterialId();
		m_materialMap[id] = std::move(material);
		materialIds.push_back(id);
	}
	
	return materialIds;
}
