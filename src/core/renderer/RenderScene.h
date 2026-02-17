#pragma once

#include "core/common/IRenderScene.h"
#include "core/renderer/RenderProxy.h"

#include <glm/glm.hpp>
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
 * (models), and separate maps for camera and light RenderProxies (ID returned to caller).
 */
class RenderScene : public IRenderScene {
public:
	uint32_t registerProxy(const RenderProxyUpdate& update) override;
	void updateProxy(uint32_t handle, const std::vector<MeshId>& meshIds, MaterialId materialId, const Transform& transform) override;
	void updateCameraProxy(uint32_t handle, const glm::mat4& view, const glm::mat4& projection, const glm::vec3& worldPos) override;

	/** Log the current number of render proxies to stdout. */
	void logProxyData() const;

	const std::unordered_map<uint32_t, RenderProxy>& cameras() const { return m_cameras; }
	const std::unordered_map<uint32_t, RenderProxy>& lights() const { return m_lights; }
	const std::unordered_map<uint32_t, RenderProxyEntry>& models() const { return m_models; }

private:
	std::unordered_map<uint32_t, RenderProxyEntry> m_models;
	std::unordered_map<uint32_t, RenderProxy> m_cameras;
	std::unordered_map<uint32_t, RenderProxy> m_lights;
	uint32_t m_nextHandle = 1;
	uint32_t m_nextCameraId = 1;
	uint32_t m_nextLightId = 1;
};

} // namespace core
