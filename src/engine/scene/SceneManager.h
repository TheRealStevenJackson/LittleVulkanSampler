#pragma once

#include "engine/scene/object/Camera.h"
#include "engine/scene/object/Light.h"
#include "core/asset/AssetManager.h"
#include "core/common/IRenderScene.h"
#include "engine/input/Controller.h"
#include "platform/graphics/vulkan/VulkanDescriptorSetLayout.h"
#include <glm/glm.hpp>

#include <string>
#include <vector>

/**
 * Manages loading and storage of scene entities, cameras, and lights.
 */
class SceneManager {
public:
	explicit SceneManager(AssetManager& assetManager);

	SceneManager(const SceneManager&) = delete;
	SceneManager& operator=(const SceneManager&) = delete;

	/**
	 * Load a glTF 2.0 .glb scene via AssetManager::loadScene.
	 * \param filepath Path to the .glb file.
	 */
	void loadScene(const std::string& filepath, VulkanDescriptorSetLayout* materialDescriptorLayout);

	/**
	 * Temporary: load a single entity from OBJ paths (tries paths, loads mesh + materials, creates Entity with controller).
	 * Stores the entity in the manager; use loadedEntity() to access it.
	 * If materialDescriptorLayout is non-null, updates material descriptor sets after loading (for rendering).
	 * \param initialTransform Initial model matrix for the entity (default: identity at origin).
	 * \return true if loading succeeded, false if all paths failed.
	 */
	bool loadEntityTemporary(const std::vector<std::string>& pathsToTry,
		engine::Controller* controller,
		VulkanDescriptorSetLayout* materialDescriptorLayout = nullptr,
		const glm::mat4& initialTransform = glm::mat4(1.0f));

	/** Temporary: get the first entity loaded by loadEntityTemporary. Returns nullptr if none loaded. */
	Entity* loadedEntity() { return m_loadedEntities.empty() ? nullptr : &m_loadedEntities.front(); }
	const Entity* loadedEntity() const { return m_loadedEntities.empty() ? nullptr : &m_loadedEntities.front(); }

	/**
	 * Temporary: create a camera with initial view and projection matrices and store it in the manager.
	 * Use loadedCamera() to access the camera.
	 * \return true (camera is always created).
	 */
	bool loadCameraTemporary(const glm::mat4& view, const glm::mat4& proj);

	/** Temporary: get the first camera created by loadCameraTemporary. Returns nullptr if none. */
	Camera* loadedCamera() { return m_cameras.empty() ? nullptr : &m_cameras.front(); }
	const Camera* loadedCamera() const { return m_cameras.empty() ? nullptr : &m_cameras.front(); }

	/**
	 * Temporary: create a directional light with the given direction and color (vec4 for UBO compatibility) and store it.
	 * Use lights() to access the lights.
	 * \return true (light is always created).
	 */
	bool loadLightTemporary(const glm::vec4& direction, const glm::vec4& color);

	/** All lights created by loadLightTemporary. */
	Light* loadedLight() { return m_lights.empty() ? nullptr : &m_lights.front(); }
	const Light* loadedLight() const { return m_lights.empty() ? nullptr : &m_lights.front(); }

	/** Update all loaded entities with the given delta time. */
	void update(float dt);

	/** Set the render scene (e.g. from Renderer::getRenderScene()). Can be nullptr. */
	void setRenderScene(core::IRenderScene* renderScene) { m_renderScene = renderScene; }

	/** Get the current render scene, or nullptr if not set. */
	core::IRenderScene* renderScene() { return m_renderScene; }
	const core::IRenderScene* renderScene() const { return m_renderScene; }

private:
	AssetManager* m_assetManager;
	core::IRenderScene* m_renderScene = nullptr;
	std::vector<Entity> m_loadedEntities;
	std::vector<Camera> m_cameras;
	std::vector<Light> m_lights;
};
