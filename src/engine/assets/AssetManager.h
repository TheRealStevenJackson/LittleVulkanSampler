#pragma once

#include "engine/assets/ObjLoader.h"
#include "engine/graphics/renderer/VulkanContext.h"
#include "src/engine/graphics/Mesh.h"

#include <memory>
#include <string>
#include <vector>

/**
 * AssetManager handles loading of .obj files that include .mtl (material) files.
 * OBJ and MTL are loaded via ObjLoader; MTL base path is resolved from the OBJ
 * directory so referenced materials and textures are found correctly.
 */
class AssetManager {
public:
	explicit AssetManager(VulkanContext& context);
	~AssetManager() = default;

	// Non-copyable
	AssetManager(const AssetManager&) = delete;
	AssetManager& operator=(const AssetManager&) = delete;

	/**
	 * Load an OBJ file (and its referenced .mtl). Returns one mesh per shape.
	 * Returns empty vector on failure.
	 */
	std::vector<std::unique_ptr<Mesh>> loadObj(const std::string& filepath);

	/**
	 * Load an OBJ file and combine all shapes into a single mesh.
	 * Returns nullptr on failure.
	 */
	std::unique_ptr<Mesh> loadObjCombined(const std::string& filepath);

	/**
	 * Try loading from multiple paths (e.g. relative to executable vs build dir).
	 * Fills outMeshes and outLoadedPath on success; clears them on failure.
	 * Returns true if any path succeeded, false otherwise.
	 */
	bool loadObjFromPaths(const std::vector<std::string>& pathsToTry,
		std::vector<std::unique_ptr<Mesh>>& outMeshes,
		std::string& outLoadedPath);

private:
	VulkanContext& mContext;
	ObjLoader mObjLoader;
};
