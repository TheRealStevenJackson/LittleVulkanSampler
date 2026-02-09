#include "engine/scene/SceneLoader.h"
#include "core/asset/types/Mesh.h"
#include "core/asset/types/Material.h"
#include "platform/graphics/vulkan/VulkanContext.h"

#include <fastgltf/core.hpp>
#include <fastgltf/tools.hpp>
#include <glm/glm.hpp>
#include <filesystem>
#include <iostream>
#include <optional>
#include <vector>

namespace fs = std::filesystem;

// Vertex layout matching MeshLoader (pos, normal, texCoord, tangent, bitangent)
struct GltfVertex {
	glm::vec3 pos;
	glm::vec3 normal;
	glm::vec2 texCoord;
	glm::vec3 tangent;
	glm::vec3 bitangent;
};

static bool parseGltfFile(const std::string& filepath,
	fastgltf::Asset& outAsset)
{
	auto dataResult = fastgltf::GltfDataBuffer::FromPath(filepath);
	if (dataResult.error() != fastgltf::Error::None) {
		std::cerr << "fastgltf: failed to load file: " << filepath << std::endl;
		return false;
	}

	fastgltf::GltfDataBuffer& data = dataResult.get();
	fs::path path(filepath);
	fs::path basePath = path.has_parent_path() ? path.parent_path() : fs::path(".");

	constexpr auto options = fastgltf::Options::LoadGLBBuffers
		| fastgltf::Options::LoadExternalBuffers;

	fastgltf::Parser parser{};
	std::optional<fastgltf::Asset> assetResult;

	if (path.extension() == ".glb") {
		auto result = parser.loadBinaryGLTF(&data, basePath, options);
		if (result.error() != fastgltf::Error::None) {
			std::cerr << "fastgltf: failed to parse binary GLTF: " << filepath << std::endl;
			return false;
		}
		assetResult = std::move(result.get());
	} else {
		auto result = parser.loadGltf(&data, basePath, options);
		if (result.error() != fastgltf::Error::None) {
			std::cerr << "fastgltf: failed to parse GLTF: " << filepath << std::endl;
			return false;
		}
		assetResult = std::move(result.get());
	}

	if (!assetResult) return false;
	outAsset = std::move(*assetResult);
	return true;
}

static void computeTangentSpace(std::vector<GltfVertex>& vertices,
	const std::vector<uint32_t>& indices)
{
	if (indices.size() % 3 != 0) return;
	for (size_t i = 0; i < indices.size(); i += 3) {
		uint32_t i0 = indices[i + 0], i1 = indices[i + 1], i2 = indices[i + 2];
		GltfVertex& v0 = vertices[i0], & v1 = vertices[i1], & v2 = vertices[i2];
		glm::vec3 edge1 = v1.pos - v0.pos;
		glm::vec3 edge2 = v2.pos - v0.pos;
		glm::vec2 deltaUV1 = v1.texCoord - v0.texCoord;
		glm::vec2 deltaUV2 = v2.texCoord - v0.texCoord;
		float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
		if (std::abs(deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y) > 1e-6f) {
			glm::vec3 tangent(
				f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x),
				f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y),
				f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z));
			v0.tangent += tangent; v1.tangent += tangent; v2.tangent += tangent;
		}
	}
	for (auto& v : vertices) {
		if (glm::length(v.tangent) > 1e-6f) v.tangent = glm::normalize(v.tangent);
		else v.tangent = glm::vec3(1.0f, 0.0f, 0.0f);
		if (glm::length(v.bitangent) > 1e-6f) v.bitangent = glm::normalize(v.bitangent);
		else v.bitangent = glm::vec3(0.0f, 1.0f, 0.0f);
	}
}

bool SceneLoader::loadGltfFromPaths(const std::vector<std::string>& pathsToTry,
	std::vector<Entity>& outEntities,
	std::string& outLoadedPath)
{
	fastgltf::Asset asset;
	std::string loadedPath;
	bool loaded = false;
	for (const auto& path : pathsToTry) {
		if (parseGltfFile(path, asset)) {
			loadedPath = path;
			loaded = true;
			break;
		}
	}
	if (!loaded) {
		outLoadedPath.clear();
		return false;
	}

	VulkanContext& ctx = m_assetManager->context();
	std::vector<MeshId> meshIds;
	std::vector<MaterialId> materialIds;

	// One default material for all glTF meshes
	auto defaultMaterial = std::make_unique<Material>(ctx);
	MaterialId defaultMatId = m_assetManager->addMaterial(std::move(defaultMaterial));
	materialIds.push_back(defaultMatId);

	for (auto& mesh : asset.meshes) {
		for (auto& primitive : mesh.primitives) {
			std::vector<GltfVertex> vertices;
			std::vector<uint32_t> indices32;
			std::vector<uint16_t> indices16;

			// POSITION (required)
			auto posIt = primitive.attributes.find("POSITION");
			if (posIt == primitive.attributes.end()) continue;
			fastgltf::Accessor& posAccessor = asset.accessors[posIt->second];
			vertices.resize(posAccessor.count);
			fastgltf::iterateAccessor<glm::vec3>(asset, posAccessor,
				[&](glm::vec3 p, size_t i) { vertices[i].pos = p; });

			// NORMAL (optional)
			auto normIt = primitive.attributes.find("NORMAL");
			if (normIt != primitive.attributes.end()) {
				fastgltf::iterateAccessor<glm::vec3>(asset, asset.accessors[normIt->second],
					[&](glm::vec3 n, size_t i) { vertices[i].normal = n; });
			} else {
				for (auto& v : vertices) v.normal = glm::vec3(0.0f, 0.0f, 1.0f);
			}

			// TEXCOORD_0 (optional)
			auto uvIt = primitive.attributes.find("TEXCOORD_0");
			if (uvIt != primitive.attributes.end()) {
				fastgltf::iterateAccessor<glm::vec2>(asset, asset.accessors[uvIt->second],
					[&](glm::vec2 uv, size_t i) {
						vertices[i].texCoord = glm::vec2(uv.x, 1.0f - uv.y);
					});
			} else {
				for (auto& v : vertices) v.texCoord = glm::vec2(0.0f, 0.0f);
			}

			for (auto& v : vertices) {
				v.tangent = glm::vec3(0.0f, 0.0f, 0.0f);
				v.bitangent = glm::vec3(0.0f, 0.0f, 0.0f);
			}

			// Indices (optional)
			bool hasIndices = primitive.indicesAccessor.has_value();
			if (hasIndices) {
				fastgltf::Accessor& idxAccessor = asset.accessors[*primitive.indicesAccessor];
				if (idxAccessor.componentType == fastgltf::ComponentType::UnsignedInt) {
					fastgltf::iterateAccessor<uint32_t>(asset, idxAccessor,
						[&](uint32_t idx) { indices32.push_back(idx); });
				} else {
					fastgltf::iterateAccessor<uint16_t>(asset, idxAccessor,
						[&](uint16_t idx) { indices16.push_back(idx); });
				}
			}

			if (hasIndices && !indices32.empty())
				computeTangentSpace(vertices, indices32);
			else if (hasIndices && !indices16.empty()) {
				std::vector<uint32_t> up(indices16.begin(), indices16.end());
				computeTangentSpace(vertices, up);
			}

			std::unique_ptr<Mesh> meshPtr;
			if (hasIndices && !indices32.empty()) {
				meshPtr = std::make_unique<Mesh>(ctx,
					vertices.data(), vertices.size() * sizeof(GltfVertex), static_cast<uint32_t>(vertices.size()),
					indices32.data(), indices32.size() * sizeof(uint32_t), static_cast<uint32_t>(indices32.size()),
					VK_INDEX_TYPE_UINT32);
			} else if (hasIndices && !indices16.empty()) {
				meshPtr = std::make_unique<Mesh>(ctx,
					vertices.data(), vertices.size() * sizeof(GltfVertex), static_cast<uint32_t>(vertices.size()),
					indices16.data(), indices16.size() * sizeof(uint16_t), static_cast<uint32_t>(indices16.size()),
					VK_INDEX_TYPE_UINT16);
			} else {
				meshPtr = std::make_unique<Mesh>(ctx,
					vertices.data(), vertices.size() * sizeof(GltfVertex), static_cast<uint32_t>(vertices.size()));
			}
			MeshId id = m_assetManager->addMesh(std::move(meshPtr));
			meshIds.push_back(id);
		}
	}

	if (meshIds.empty()) {
		outLoadedPath.clear();
		return false;
	}

	Entity entity;
	entity.renderComponent().meshIds = std::move(meshIds);
	entity.renderComponent().materialIds = std::move(materialIds);
	outEntities.push_back(std::move(entity));
	outLoadedPath = loadedPath;
	return true;
}

std::optional<Entity> SceneLoader::loadOneGltfFromPaths(const std::vector<std::string>& pathsToTry,
	std::string* outLoadedPath)
{
	std::vector<Entity> entities;
	std::string loadedPath;
	if (!loadGltfFromPaths(pathsToTry, entities, loadedPath))
		return std::nullopt;
	if (outLoadedPath)
		*outLoadedPath = loadedPath;
	return std::move(entities.front());
}

SceneLoader::SceneLoader(AssetManager& assetManager)
	: m_assetManager(&assetManager)
{
}

bool SceneLoader::loadFromPaths(const std::vector<std::string>& pathsToTry,
	std::vector<Entity>& outEntities,
	std::string& outLoadedPath)
{
	std::vector<MeshId> meshIds;
	if (!m_assetManager->loadObjFromPaths(pathsToTry, meshIds, outLoadedPath))
		return false;

	std::vector<MaterialId> materialIds = m_assetManager->loadMaterials(outLoadedPath);

	Entity entity;
	entity.renderComponent().meshIds = std::move(meshIds);
	entity.renderComponent().materialIds = std::move(materialIds);
	outEntities.push_back(std::move(entity));
	return true;
}

std::optional<Entity> SceneLoader::loadOneFromPaths(const std::vector<std::string>& pathsToTry,
	std::string* outLoadedPath)
{
	std::vector<Entity> entities;
	std::string loadedPath;
	if (!loadFromPaths(pathsToTry, entities, loadedPath))
		return std::nullopt;
	if (outLoadedPath)
		*outLoadedPath = loadedPath;
	return std::move(entities.front());
}
