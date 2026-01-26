#pragma once

#include "engine/assets/AssetManager.h"

/**
 * Game-layer component that references a mesh by ID.
 * Used by render systems to draw entities; resolve via AssetManager::getMesh().
 */
struct RenderComponent {
	MeshId meshId = InvalidMeshId;
};
