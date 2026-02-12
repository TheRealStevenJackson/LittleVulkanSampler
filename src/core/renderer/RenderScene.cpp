#include "core/renderer/RenderScene.h"
#include "core/math/Transform.h"
#include <glm/glm.hpp>
#include <iostream>

namespace core {

uint32_t RenderScene::registerProxy(const std::vector<MeshId>& meshIds, MaterialId materialId, const Transform& transform) {
	const uint32_t handle = m_nextHandle++;
	RenderProxyEntry entry;
	entry.transform = transform;
	entry.meshIds = meshIds;
	entry.materialId = materialId;
	m_proxies.emplace(handle, std::move(entry));
	return handle;
}

void RenderScene::updateProxy(uint32_t handle, const std::vector<MeshId>& meshIds, MaterialId materialId, const Transform& transform) {
	auto it = m_proxies.find(handle);
	if (it != m_proxies.end()) {
		it->second.transform = transform;
		it->second.meshIds = meshIds;
		it->second.materialId = materialId;
	}
}

void RenderScene::logProxyCount() const {
	std::cout << "RenderScene: " << m_proxies.size() << " proxy(ies)" << std::endl;
	for (const auto& [handle, entry] : m_proxies) {
		std::cout << "  [handle " << handle << "] meshIds(" << entry.meshIds.size() << "):";
		for (MeshId id : entry.meshIds)
			std::cout << " " << id;
		std::cout << ", materialId: " << entry.materialId;
		const glm::mat4& m = entry.transform.matrix();
		std::cout << ", transform:\n";
		for (int row = 0; row < 4; ++row) {
			std::cout << "    ";
			for (int col = 0; col < 4; ++col)
				std::cout << m[col][row] << (col < 3 ? " " : "\n");
		}
	}
}

} // namespace core
