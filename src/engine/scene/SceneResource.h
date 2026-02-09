#pragma once

#include "game/Entity.h"

#include <string>
#include <vector>

/**
 * Represents a loaded scene: the path it was loaded from and the entities it contains.
 * Used by SceneManager to store and reference loaded scene data.
 */
class SceneResource {
public:
	SceneResource() = default;

	SceneResource(const SceneResource&) = delete;
	SceneResource& operator=(const SceneResource&) = delete;

	SceneResource(SceneResource&&) noexcept = default;
	SceneResource& operator=(SceneResource&&) noexcept = default;

	/** Path that was successfully loaded (e.g. .obj, .gltf, .glb). */
	const std::string& loadedPath() const { return m_loadedPath; }
	void setLoadedPath(std::string path) { m_loadedPath = std::move(path); }

	/** Entities in this scene (mesh/material IDs reference AssetManager). */
	std::vector<Entity>& entities() { return m_entities; }
	const std::vector<Entity>& entities() const { return m_entities; }

	/** Whether this resource has any content (path set and at least one entity, or path set). */
	bool isEmpty() const {
		return m_loadedPath.empty() && m_entities.empty();
	}

private:
	std::string m_loadedPath;
	std::vector<Entity> m_entities;
};
