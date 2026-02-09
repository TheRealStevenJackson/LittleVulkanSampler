#include "core/asset/AssetManager.h"
#include "core/asset/types/Material.h"
#include "platform/graphics/vulkan/VulkanDescriptorSet.h"

#include <vulkan/vulkan.h>
#include <iostream>
#include <vector>

AssetManager::AssetManager(VulkanContext& context)
	: mContext(context)
	, mMeshLoader(context)
	, mMaterialLoader(context)
	, mShaderLoader(context)
{
}

AssetManager::~AssetManager()
{
	// Destroy materials (and their descriptor sets) first, while the pool is still alive.
	// Member destruction order would otherwise destroy m_materialDescriptorPool before m_materialMap.
	m_materialMap.clear();
	if (m_defaultSampler != VK_NULL_HANDLE) {
		vkDestroySampler(mContext.device(), m_defaultSampler, nullptr);
		m_defaultSampler = VK_NULL_HANDLE;
	}
}

MeshId AssetManager::nextMeshId() {
	return m_nextMeshId++;
}

MaterialId AssetManager::nextMaterialId() {
	return m_nextMaterialId++;
}

ShaderId AssetManager::nextShaderId() {
	return m_nextShaderId++;
}

std::vector<MeshId> AssetManager::loadObj(const std::string& filepath) {
	std::vector<MeshId> ids;
	auto meshes = mMeshLoader.loadFromFile(filepath);
	for (auto& mesh : meshes) {
		MeshId id = nextMeshId();
		m_meshMap[id] = std::move(mesh);
		ids.push_back(id);
	}
	return ids;
}

MeshId AssetManager::loadObjCombined(const std::string& filepath) {
	auto mesh = mMeshLoader.loadFromFileCombined(filepath);
	if (!mesh)
		return InvalidMeshId;
	MeshId id = nextMeshId();
	m_meshMap[id] = std::move(mesh);
	return id;
}

bool AssetManager::loadObjFromPaths(const std::vector<std::string>& pathsToTry,
	std::vector<MeshId>& outMeshIds,
	std::string& outLoadedPath)
{
	for (const auto& path : pathsToTry) {
		auto ids = loadObj(path);
		if (!ids.empty()) {
			outMeshIds = std::move(ids);
			outLoadedPath = path;
			return true;
		}
	}
	outMeshIds.clear();
	outLoadedPath.clear();
	return false;
}

MeshId AssetManager::addMesh(std::unique_ptr<Mesh> mesh) {
	if (!mesh)
		return InvalidMeshId;
	MeshId id = nextMeshId();
	m_meshMap[id] = std::move(mesh);
	return id;
}

MaterialId AssetManager::addMaterial(std::unique_ptr<Material> material) {
	if (!material)
		return InvalidMaterialId;
	MaterialId id = nextMaterialId();
	m_materialMap[id] = std::move(material);
	return id;
}

Mesh* AssetManager::getMesh(MeshId id) {
	auto it = m_meshMap.find(id);
	return it != m_meshMap.end() ? it->second.get() : nullptr;
}

const Mesh* AssetManager::getMesh(MeshId id) const {
	auto it = m_meshMap.find(id);
	return it != m_meshMap.end() ? it->second.get() : nullptr;
}

Material* AssetManager::getMaterial(MaterialId id) {
	auto it = m_materialMap.find(id);
	return it != m_materialMap.end() ? it->second.get() : nullptr;
}

const Material* AssetManager::getMaterial(MaterialId id) const {
	auto it = m_materialMap.find(id);
	return it != m_materialMap.end() ? it->second.get() : nullptr;
}

Shader* AssetManager::getShader(ShaderId id) {
	auto it = m_shaderMap.find(id);
	return it != m_shaderMap.end() ? it->second.get() : nullptr;
}

const Shader* AssetManager::getShader(ShaderId id) const {
	auto it = m_shaderMap.find(id);
	return it != m_shaderMap.end() ? it->second.get() : nullptr;
}

ShaderId AssetManager::loadShader(const std::string& filepath) {
	auto shader = mShaderLoader.loadShader(filepath);
	if (!shader)
		return InvalidShaderId;
	ShaderId id = nextShaderId();
	m_shaderMap[id] = std::move(shader);
	return id;
}

std::vector<MaterialId> AssetManager::loadMaterials(const std::string& filepath)
{
	std::vector<MaterialId> materialIds;
	std::vector<MaterialPaths> paths = mMeshLoader.extractMaterialPaths(filepath);

	for (const auto& path : paths) {
		auto material = mMaterialLoader.loadMaterial(path);
		if (!material)
			continue;
		MaterialId id = nextMaterialId();
		m_materialMap[id] = std::move(material);
		materialIds.push_back(id);
	}

	return materialIds;
}

void AssetManager::updateMaterialDescriptorSets(VulkanDescriptorSetLayout& materialDescriptorLayout)
{
	// Create or replace default sampler
	if (m_defaultSampler != VK_NULL_HANDLE) {
		vkDestroySampler(mContext.device(), m_defaultSampler, nullptr);
		m_defaultSampler = VK_NULL_HANDLE;
	}
	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_FALSE;
	samplerInfo.maxAnisotropy = 1.0f;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;
	if (vkCreateSampler(mContext.device(), &samplerInfo, nullptr, &m_defaultSampler) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create default sampler.");
	}

	const uint32_t size = static_cast<uint32_t>(m_materialMap.size());
	std::vector<VkDescriptorPoolSize> poolSizes(2);
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[0].descriptorCount = 5u * size;
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[1].descriptorCount = size;
	m_materialDescriptorPool = std::make_unique<VulkanDescriptorPool>(mContext, poolSizes, size);

	for (auto& [id, materialPtr] : m_materialMap) {
		Material* material = materialPtr.get();
		VulkanDescriptorSet set(mContext, *m_materialDescriptorPool, materialDescriptorLayout);

		if (material->hasAlbedoMap() && material->albedoMap()) {
			set.writeCombinedImageSampler(*material->albedoMap()->image(), m_defaultSampler, 0);
		}
		if (material->hasNormalMap() && material->normalMap()) {
			set.writeCombinedImageSampler(*material->normalMap()->image(), m_defaultSampler, 1);
		}
		if (material->hasMetallicMap() && material->metallicMap()) {
			set.writeCombinedImageSampler(*material->metallicMap()->image(), m_defaultSampler, 2);
		}
		if (material->hasRoughnessMap() && material->roughnessMap()) {
			set.writeCombinedImageSampler(*material->roughnessMap()->image(), m_defaultSampler, 3);
		}
		if (material->hasAoMap() && material->aoMap()) {
			set.writeCombinedImageSampler(*material->aoMap()->image(), m_defaultSampler, 4);
		}
		set.writeUniformBuffer(material->materialUBO(), sizeof(MaterialUBO), 5);

		material->setDescriptorSet(std::make_unique<VulkanDescriptorSet>(std::move(set)));

		std::cout << "setPtr: " << material->descriptorSet() << std::endl;
	}
}