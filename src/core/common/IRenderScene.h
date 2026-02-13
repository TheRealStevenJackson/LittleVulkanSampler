#pragma once

#include "core/asset/AssetManager.h"
#include "core/common/RenderDataTypes.h"
#include "core/math/Transform.h"

#include <cstdint>
#include <vector>

namespace core {

/**
 * Interface for a scene that holds render proxies. Implementations manage
 * registration and updates of proxies for rendering.
 */
struct IRenderScene {
	virtual ~IRenderScene() = default;

	/** Register a proxy from a RenderProxyUpdate; returns an opaque handle. 0 means invalid/failure. */
	virtual uint32_t registerProxy(const RenderProxyUpdate& update) = 0;

	/** Update an existing proxy by the handle returned from registerProxy. */
	virtual void updateProxy(uint32_t handle, const std::vector<MeshId>& meshIds, MaterialId materialId, const Transform& transform) = 0;
};

} // namespace core
