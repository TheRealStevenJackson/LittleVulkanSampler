#pragma once

#include "core/asset/AssetManager.h"
#include "game/Entity.h"

#include <optional>
#include <string>
#include <vector>

/**
 * Loads scene data (meshes, materials) via AssetManager and produces entities.
 * Encapsulates the flow: try paths -> load OBJ + materials -> create Entity with RenderComponent.
 */
class SceneLoader {
public:
	explicit SceneLoader(AssetManager& assetManager);

	SceneLoader(const SceneLoader&) = delete;
	SceneLoader& operator=(const SceneLoader&) = delete;

	/**
	 * Try loading an OBJ from the given paths; loads materials from the OBJ's MTL.
	 * On success, appends one Entity (with mesh and material IDs set) to outEntities
	 * and sets outLoadedPath to the path that succeeded.
	 * \return true if any path succeeded, false otherwise.
	 */
	bool loadFromPaths(const std::vector<std::string>& pathsToTry,
		std::vector<Entity>& outEntities,
		std::string& outLoadedPath);

	/**
	 * Load a single OBJ from the first path that succeeds.
	 * \return The created Entity, or std::nullopt if all paths failed.
	 */
	std::optional<Entity> loadOneFromPaths(const std::vector<std::string>& pathsToTry,
		std::string* outLoadedPath = nullptr);

	/**
	 * Try loading a glTF/GLB from the given paths using fastgltf.
	 * On success, appends one Entity per glTF node/mesh to outEntities and sets outLoadedPath.
	 * \return true if any path succeeded, false otherwise.
	 */
	bool loadGltfFromPaths(const std::vector<std::string>& pathsToTry,
		std::vector<Entity>& outEntities,
		std::string& outLoadedPath);

	/**
	 * Load a single glTF/GLB from the first path that succeeds.
	 * \return The first created Entity, or std::nullopt if all paths failed.
	 */
	std::optional<Entity> loadOneGltfFromPaths(const std::vector<std::string>& pathsToTry,
		std::string* outLoadedPath = nullptr);

private:
	AssetManager* m_assetManager;
};
