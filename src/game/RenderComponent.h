#pragma once

#include "core/asset/AssetManager.h"

#include <vector>

/**
 * Game-layer component that references meshes and materials by ID.
 * Used by render systems to draw entities; resolve via AssetManager::getMesh() and AssetManager::getMaterial().
 * Can hold multiple meshes and materials for complex entities.
 */
struct RenderComponent {
	std::vector<MeshId> meshIds;
	std::vector<MaterialId> materialIds;
};
