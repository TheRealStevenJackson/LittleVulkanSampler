#pragma once

#include "core/common/RenderDataTypes.h"
#include "core/math/Transform.h"
#include "core/asset/AssetManager.h"

namespace core {

/**
 * Lightweight render proxy: holds one of CameraData, DirectionalLightData, or ModelData,
 * tagged by ProxyType. Used to represent a camera, light, or drawable instance for the renderer.
 * Single constructor: pass type and a pointer to the corresponding data (or nullptr to zero-init).
 */
class RenderProxy {
public:
	RenderProxy() : m_type(ProxyType::Model) { m_data.model = ModelData{}; }

	RenderProxy(ProxyType type, const void* data) : m_type(type) {
		if (data) {
			switch (type) {
				case ProxyType::Camera:
					m_data.camera = *static_cast<const CameraData*>(data);
					break;
				case ProxyType::DirectionalLight:
					m_data.directionalLight = *static_cast<const DirectionalLightData*>(data);
					break;
				case ProxyType::Model:
					m_data.model = *static_cast<const ModelData*>(data);
					break;
				case ProxyType::PointLight:
					// Store as directional light data until PointLightData exists
					m_data.directionalLight = *static_cast<const DirectionalLightData*>(data);
					break;
			}
		} else {
			m_data.model = ModelData{};
		}
	}

	ProxyType type() const { return m_type; }

	CameraData& cameraData() { return m_data.camera; }
	const CameraData& cameraData() const { return m_data.camera; }

	DirectionalLightData& directionalLightData() { return m_data.directionalLight; }
	const DirectionalLightData& directionalLightData() const { return m_data.directionalLight; }

	ModelData& modelData() { return m_data.model; }
	const ModelData& modelData() const { return m_data.model; }

	Transform transform() const { return Transform(m_data.model.modelMatrix); }
	MeshId meshID() const { return m_data.model.meshID; }
	MaterialId materialID() const { return m_data.model.materialID; }
	void setMeshID(MeshId id) { m_data.model.meshID = id; }
	void setMaterialID(MaterialId id) { m_data.model.materialID = id; }

private:
	ProxyType m_type;
	union {
		CameraData camera;
		DirectionalLightData directionalLight;
		ModelData model;
	} m_data;
};

} // namespace core
