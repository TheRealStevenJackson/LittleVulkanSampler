#pragma once

#include "core/common/IRenderScene.h"

#include <cstdint>
#include <unordered_map>
#include <vector>

namespace core {

struct RenderProxyEntry {
	Transform transform;
	std::vector<MeshId> meshIds;
	MaterialId materialId = InvalidMaterialId;
};

/**
 * Default implementation of IRenderScene. Stores render proxy entries by handle
 * and supports registration and updates.
 */
class RenderScene : public IRenderScene {
public:
	uint32_t registerProxy(const std::vector<MeshId>& meshIds, MaterialId materialId, const Transform& transform) override;
	void updateProxy(uint32_t handle, const std::vector<MeshId>& meshIds, MaterialId materialId, const Transform& transform) override;

	/** Log the current number of render proxies to stdout. */
	void logProxyCount() const;

private:
	std::unordered_map<uint32_t, RenderProxyEntry> m_proxies;
	uint32_t m_nextHandle = 1;
};

} // namespace core
