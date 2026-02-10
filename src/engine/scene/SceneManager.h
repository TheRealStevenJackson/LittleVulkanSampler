#pragma once

#include "engine/scene/SceneLoader.h"
#include "engine/scene/SceneResource.h"
#include "core/asset/AssetManager.h"
#include "engine/input/Controller.h"
#include "platform/graphics/vulkan/VulkanDescriptorSetLayout.h"

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

/**
 * Manages loading and storage of scenes. Uses SceneLoader to load OBJ or glTF/GLB
 * from given paths and stores the result in named SceneResources.
 */
class SceneManager {
public:
	explicit SceneManager(AssetManager& assetManager);

	SceneManager(const SceneManager&) = delete;
	SceneManager& operator=(const SceneManager&) = delete;

	/**
	 * Try loading a scene from the given paths. Tries OBJ first, then glTF/GLB.
	 * \param sceneName Name to register the scene under (for getScene / setCurrentScene).
	 * \param pathsToTry List of paths to try (e.g. relative to executable and build dir).
	 * \return true if any path succeeded and the scene was stored under sceneName.
	 */
	bool loadScene(const std::string& sceneName,
		const std::vector<std::string>& pathsToTry);

	/**
	 * Load scene from paths and store in a new SceneResource, without naming.
	 * \return The loaded SceneResource, or std::nullopt if all paths failed.
	 */
	std::optional<SceneResource> loadScene(const std::vector<std::string>& pathsToTry);

	/** Get a named scene. Returns nullptr if not found. */
	SceneResource* getScene(const std::string& sceneName);
	const SceneResource* getScene(const std::string& sceneName) const;

	/** Set the current scene by name. No-op if name not found. */
	void setCurrentScene(const std::string& sceneName);

	/** Get the current scene's resource, or nullptr if none set. */
	SceneResource* currentScene() { return m_currentScene; }
	const SceneResource* currentScene() const { return m_currentScene; }

	/** Clear current scene reference (does not unload the named scene). */
	void clearCurrentScene() { m_currentScene = nullptr; }

	/** Remove a named scene from storage. */
	void unloadScene(const std::string& sceneName);

	/**
	 * Temporary: load a single entity from OBJ paths (tries paths, loads mesh + materials, creates Entity with controller).
	 * Stores the entity in the manager; use loadedEntity() to access it.
	 * If materialDescriptorLayout is non-null, updates material descriptor sets after loading (for rendering).
	 * \return true if loading succeeded, false if all paths failed.
	 */
	bool loadEntityTemporary(const std::vector<std::string>& pathsToTry,
		engine::Controller* controller,
		VulkanDescriptorSetLayout* materialDescriptorLayout = nullptr);

	/** Temporary: get the first entity loaded by loadEntityTemporary. Returns nullptr if none loaded. */
	Entity* loadedEntity() { return m_loadedEntities.empty() ? nullptr : &m_loadedEntities.front(); }
	const Entity* loadedEntity() const { return m_loadedEntities.empty() ? nullptr : &m_loadedEntities.front(); }

	/** Update all loaded entities with the given delta time. */
	void update(float dt);

	SceneLoader& loader() { return m_loader; }
	const SceneLoader& loader() const { return m_loader; }

private:
	AssetManager* m_assetManager;
	SceneLoader m_loader;
	std::unordered_map<std::string, SceneResource> m_scenes;
	SceneResource* m_currentScene = nullptr;
	std::vector<Entity> m_loadedEntities;
};
