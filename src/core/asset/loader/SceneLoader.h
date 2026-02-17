#pragma once

#include "core/asset/types/Scene.h"

#include <filesystem>
#include <optional>
#include <string>

class AssetManager;

/**
 * Loads glTF 2.0 (.glb) files and returns a Scene. Only includes in rootNodes
 * (and children) nodes that have a mesh, at least one material (per primitive),
 * and a transform. Node hierarchy from the glTF is preserved.
 *
 * When an AssetManager is provided, materials and meshes are registered there
 * and scene node meshIds/materialIds refer to those asset IDs.
 */
class SceneLoader {
public:
	/**
	 * Load a GLB file and build a Scene, optionally registering meshes and materials
	 * with the given AssetManager.
	 * \param assetManager If non-null, materials and meshes are created and registered here;
	 *        node meshIds/materialIds will be AssetManager IDs. If null, nodes use glTF indices.
	 * \param path Path to the .glb file.
	 * \return The parsed Scene, or std::nullopt on parse error or if the file has no default scene.
	 */
	std::optional<Scene> loadGLB(AssetManager* assetManager, const std::filesystem::path& path);

	/**
	 * Load a GLB file (no asset registration). Nodes use glTF mesh/material indices.
	 */
	std::optional<Scene> loadGLB(const std::filesystem::path& path);

	/**
	 * Load a GLB file from a given string path (convenience overload).
	 */
	std::optional<Scene> loadGLB(const std::string& path);
};
