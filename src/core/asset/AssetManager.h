#pragma once

#include "core/asset/loader/MeshLoader.h"
#include "core/asset/loader/MaterialLoader.h"
#include "core/asset/loader/ShaderLoader.h"
#include "core/asset/types/Mesh.h"
#include "core/asset/types/Material.h"
#include "core/asset/types/Shader.h"
#include "platform/graphics/vulkan/VulkanContext.h"

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

using MeshId = uint32_t;
static constexpr MeshId InvalidMeshId = 0;

using MaterialId = uint32_t;
static constexpr MaterialId InvalidMaterialId = 0;

using ShaderId = uint32_t;
static constexpr ShaderId InvalidShaderId = 0;

/**
 * AssetManager handles loading of .obj files that include .mtl (material) files.
 * OBJ and MTL are loaded via MeshLoader; MTL base path is resolved from the OBJ
 * directory so referenced materials and textures are found correctly.
 * Meshes are stored internally; only meshIDs are handed out. Use getMesh() to
 * access a mesh by ID for rendering.
 */
class AssetManager {
public:
	explicit AssetManager(VulkanContext& context);
	~AssetManager() = default;

	// Non-copyable
	AssetManager(const AssetManager&) = delete;
	AssetManager& operator=(const AssetManager&) = delete;

	/**
	 * Load an OBJ file (and its referenced .mtl). Returns one meshID per shape.
	 * Returns empty vector on failure.
	 */
	std::vector<MeshId> loadObj(const std::string& filepath);

	/**
	 * Load an OBJ file and combine all shapes into a single mesh.
	 * Returns InvalidMeshId on failure.
	 */
	MeshId loadObjCombined(const std::string& filepath);

	/**
	 * Try loading from multiple paths (e.g. relative to executable vs build dir).
	 * Fills outMeshIds and outLoadedPath on success; clears them on failure.
	 * Returns true if any path succeeded, false otherwise.
	 */
	bool loadObjFromPaths(const std::vector<std::string>& pathsToTry,
		std::vector<MeshId>& outMeshIds,
		std::string& outLoadedPath);

	/**
	 * Get mesh by ID for rendering. Returns nullptr if ID is invalid or not found.
	 */
	Mesh* getMesh(MeshId id);
	const Mesh* getMesh(MeshId id) const;

	/**
	 * Get material by ID. Returns nullptr if ID is invalid or not found.
	 */
	Material* getMaterial(MaterialId id);
	const Material* getMaterial(MaterialId id) const;

	/**
	 * Get shader by ID. Returns nullptr if ID is invalid or not found.
	 */
	Shader* getShader(ShaderId id);
	const Shader* getShader(ShaderId id) const;

	/**
	 * Load a shader from file. Returns InvalidShaderId on failure.
	 */
	ShaderId loadShader(const std::string& filepath);

	/**
	 * Load materials from an OBJ file's MTL using MaterialLoader.
	 * Extracts material paths from the OBJ file and loads them.
	 * Returns a vector of MaterialIds, one per material loaded.
	 * Returns empty vector on failure.
	 */
	std::vector<MaterialId> loadMaterials(const std::string& filepath);

private:
	MeshId nextMeshId();
	MaterialId nextMaterialId();
	ShaderId nextShaderId();

	VulkanContext& mContext;
	MeshLoader mMeshLoader;
	MaterialLoader mMaterialLoader;
	ShaderLoader mShaderLoader;
	std::unordered_map<MeshId, std::unique_ptr<Mesh>> m_meshMap;
	std::unordered_map<MaterialId, std::unique_ptr<Material>> m_materialMap;
	std::unordered_map<ShaderId, std::unique_ptr<Shader>> m_shaderMap;
	MeshId m_nextMeshId{ 1 };
	MaterialId m_nextMaterialId{ 1 };
	ShaderId m_nextShaderId{ 1 };
};
