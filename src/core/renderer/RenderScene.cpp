#include "core/renderer/RenderScene.h"
#include "core/math/Transform.h"
#include <glm/glm.hpp>
#include <iostream>

namespace core {

uint32_t RenderScene::registerProxy(const RenderProxyUpdate& update) {
	switch (update.type) {
		case ProxyType::Camera: {
			const uint32_t id = m_nextCameraId++;
			m_cameras.emplace(id, RenderProxy(ProxyType::Camera, &update.data.camera));
			return id;
		}
		case ProxyType::DirectionalLight:
		case ProxyType::PointLight: {
			const uint32_t id = m_nextLightId++;
			m_lights.emplace(id, RenderProxy(update.type, &update.data.light));
			return id;
		}
		case ProxyType::Model: {
			const uint32_t handle = m_nextHandle++;
			RenderProxyEntry entry;
			entry.transform = Transform(update.transform);
			entry.meshIds = { update.data.model.meshID };
			entry.materialId = update.data.model.materialID;
			m_models.emplace(handle, std::move(entry));
			return handle;
		}
	}
	return 0;
}

void RenderScene::updateProxy(uint32_t handle, const std::vector<MeshId>& meshIds, MaterialId materialId, const Transform& transform) {
	auto it = m_models.find(handle);
	if (it != m_models.end()) {
		it->second.transform = transform;
		it->second.meshIds = meshIds;
		it->second.materialId = materialId;
	}
}

void RenderScene::updateCameraProxy(uint32_t handle, const glm::mat4& view, const glm::mat4& projection, const glm::vec3& worldPos) {
	auto it = m_cameras.find(handle);
	if (it != m_cameras.end()) {
		CameraData& cam = it->second.cameraData();
		cam.view = view;
		cam.projection = projection;
		cam.worldPos = worldPos;
	}
}

void RenderScene::logProxyData() const {
	std::cout << "RenderScene: " << m_cameras.size() << " camera(s), " << m_lights.size() << " light(s), " << m_models.size() << " model(s)" << std::endl;
	for (const auto& [id, proxy] : m_cameras) {
		const auto& cam = proxy.cameraData();
		std::cout << "  [camera " << id << "] worldPos(" << cam.worldPos.x << "," << cam.worldPos.y << "," << cam.worldPos.z << ")" << std::endl;
		std::cout << "    view:\n";
		for (int row = 0; row < 4; ++row) {
			std::cout << "      ";
			for (int col = 0; col < 4; ++col)
				std::cout << cam.view[col][row] << (col < 3 ? " " : "\n");
		}
		std::cout << "    proj:\n";
		for (int row = 0; row < 4; ++row) {
			std::cout << "      ";
			for (int col = 0; col < 4; ++col)
				std::cout << cam.projection[col][row] << (col < 3 ? " " : "\n");
		}
	}
	for (const auto& [id, proxy] : m_lights) {
		const auto& light = proxy.directionalLightData();
		std::cout << "  [light " << id << "] type=" << static_cast<uint32_t>(proxy.type())
			<< " direction(" << light.direction.x << "," << light.direction.y << "," << light.direction.z << ")"
			<< " color(" << light.color.x << "," << light.color.y << "," << light.color.z << ")" << std::endl;
		std::cout << "    direction (transform-like): " << light.direction.x << " " << light.direction.y << " " << light.direction.z << " " << light.direction.w << std::endl;
	}
	for (const auto& [handle, entry] : m_models) {
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
