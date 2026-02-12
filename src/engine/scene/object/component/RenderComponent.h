#pragma once

#include "core/asset/AssetManager.h"

#include <cstdint>
#include <vector>

/**
 * Game-layer component that references meshes and materials by ID.
 * Used by render systems to draw entities; resolve via AssetManager::getMesh() and AssetManager::getMaterial().
 * Can hold multiple meshes and materials for complex entities.
 * renderProxyIds are handles returned from IRenderScene::registerProxy() (one per registered proxy).
 */
struct RenderComponent {
	std::vector<MeshId> meshIds;
	std::vector<MaterialId> materialIds;
	std::vector<uint32_t> renderProxyIds;
};
