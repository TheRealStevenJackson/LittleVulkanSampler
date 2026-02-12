#pragma once

#include "core/math/Transform.h"
#include "core/asset/AssetManager.h"

namespace core {

/**
 * Lightweight render proxy: transform plus mesh and material IDs.
 * Used to represent a single drawable instance (e.g. one mesh with one material at one transform).
 */
class RenderProxy {
public:
	RenderProxy() = default;

	RenderProxy(const Transform& transform, MeshId meshId, MaterialId materialId)
		: m_transform(transform)
		, m_meshId(meshId)
		, m_materialId(materialId)
	{}

	Transform& transform() { return m_transform; }
	const Transform& transform() const { return m_transform; }

	MeshId meshID() const { return m_meshId; }
	void setMeshID(MeshId id) { m_meshId = id; }

	MaterialId materialID() const { return m_materialId; }
	void setMaterialID(MaterialId id) { m_materialId = id; }

private:
	Transform m_transform;
	MeshId m_meshId = InvalidMeshId;
	MaterialId m_materialId = InvalidMaterialId;
};

} // namespace core
