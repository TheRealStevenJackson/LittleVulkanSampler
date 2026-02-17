#include "engine/scene/SceneManager.h"
#include "core/common/RenderDataTypes.h"
#include "core/math/Transform.h"

#include <glm/gtc/quaternion.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>

#include <iostream>

namespace {

void registerNodeProxies(core::IRenderScene* renderScene, const Node& node, const glm::mat4& parentWorld) {
	const glm::mat4 world = parentWorld * node.transform;
	for (size_t i = 0; i < node.meshIds.size(); ++i) {
		MaterialId materialId = (i < node.materialIds.size()) ? node.materialIds[i] : InvalidMaterialId;
		core::RenderProxyUpdate update{};
		update.type = core::ProxyType::Model;
		update.proxyID = 0;
		update.transform = world;
		update.data.model = { world, materialId, node.meshIds[i], {} };
		renderScene->registerProxy(update);
	}
	for (const Node& child : node.children)
		registerNodeProxies(renderScene, child, world);
}

} // namespace

SceneManager::SceneManager(AssetManager& assetManager)
	: m_assetManager(&assetManager)
{
}

void SceneManager::loadScene(const std::string& filepath, VulkanDescriptorSetLayout* materialDescriptorLayout) {
	auto scene = m_assetManager->loadScene(filepath);

	if (m_renderScene) {
		const glm::mat4 rootWorld(1.0f);
		for (const Node& rootNode : scene->rootNodes)
			registerNodeProxies(m_renderScene, rootNode, rootWorld);
	}

	m_assetManager->updateMaterialDescriptorSets(*materialDescriptorLayout);
}

bool SceneManager::loadEntityTemporary(const std::vector<std::string>& pathsToTry,
	engine::Controller* controller,
	VulkanDescriptorSetLayout* materialDescriptorLayout,
	const glm::mat4& initialTransform)
{
	std::vector<MeshId> meshIds;
	std::string loadedPath;
	if (!m_assetManager->loadObjFromPaths(pathsToTry, meshIds, loadedPath))
		return false;

	std::vector<MaterialId> materialIds = m_assetManager->loadMaterials(loadedPath);

	Entity entity;
	entity.renderComponent().meshIds = std::move(meshIds);
	entity.renderComponent().materialIds = std::move(materialIds);
	entity.setController(controller);

	// Apply initial transform by decomposing into position, rotation, scale
	glm::vec3 scale;
	glm::quat orientation;
	glm::vec3 translation;
	glm::vec3 skew;
	glm::vec4 perspective;
	if (glm::decompose(initialTransform, scale, orientation, translation, skew, perspective)) {
		entity.transformComponent().position = translation;
		entity.transformComponent().rotation = glm::eulerAngles(orientation);
		entity.transformComponent().scale = scale;
	}

	if (m_renderScene) {
		const glm::mat4 transform = entity.model();
		const auto& meshIds = entity.renderComponent().meshIds;
		MaterialId materialId = entity.renderComponent().materialIds.empty()
			? InvalidMaterialId
			: entity.renderComponent().materialIds.front();
		entity.renderComponent().renderProxyIds.clear();
		for (MeshId meshId : meshIds) {
			core::RenderProxyUpdate update{};
			update.type = core::ProxyType::Model;
			update.proxyID = 0;
			update.transform = transform;
			update.data.model = { transform, materialId, meshId, {} };
			entity.renderComponent().renderProxyIds.push_back(
				m_renderScene->registerProxy(update));
		}
	}

	//m_loadedEntities.clear();
	m_loadedEntities.push_back(std::move(entity));

	if (materialDescriptorLayout)
		m_assetManager->updateMaterialDescriptorSets(*materialDescriptorLayout);

	return true;
}

bool SceneManager::loadCameraTemporary(const glm::mat4& view, const glm::mat4& proj)
{
	Camera camera;
	camera.setViewMatrix(view);
	camera.setProjectionMatrix(proj);
	if (m_renderScene) {
		core::RenderProxyUpdate update{};
		update.type = core::ProxyType::Camera;
		update.proxyID = 0;
		update.transform = glm::mat4(1.0f);
		update.data.camera.view = view;
		update.data.camera.projection = proj;
		update.data.camera.worldPos = glm::vec3(0.0f);
		update.data.camera.padding = 0.0f;
		camera.renderComponent().renderProxyIds.clear();
		camera.renderComponent().renderProxyIds.push_back(m_renderScene->registerProxy(update));
	}
	m_cameras.clear();
	m_cameras.push_back(std::move(camera));
	return true;
}

bool SceneManager::loadLightTemporary(const glm::vec4& direction, const glm::vec4& color)
{
	Light light;
	light.setType(Light::Type::Directional);
	light.setDirection(glm::vec3(direction));
	light.setColor(glm::vec3(color));
	if (m_renderScene) {
		core::RenderProxyUpdate update{};
		update.type = core::ProxyType::DirectionalLight;
		update.proxyID = 0;
		update.transform = glm::mat4(1.0f);
		update.data.light.direction = direction;
		update.data.light.color = color;
		light.renderComponent().renderProxyIds.clear();
		light.renderComponent().renderProxyIds.push_back(m_renderScene->registerProxy(update));
	}
	m_lights.push_back(std::move(light));
	return true;
}

void SceneManager::update(float dt) {
	for (Entity& entity : m_loadedEntities) {
		entity.update(dt);
		if (m_renderScene) {
			const auto& rc = entity.renderComponent();
			MaterialId materialId = rc.materialIds.empty() ? InvalidMaterialId : rc.materialIds.front();
			core::Transform transform(entity.model());
			for (size_t i = 0; i < rc.renderProxyIds.size() && i < rc.meshIds.size(); ++i)
				m_renderScene->updateProxy(rc.renderProxyIds[i], { rc.meshIds[i] }, materialId, transform);
		}
	}
	for (Camera& camera : m_cameras) {
		camera.update(dt);
		if (m_renderScene) {
			const auto& proxyIds = camera.renderComponent().renderProxyIds;
			if (!proxyIds.empty()) {
				const uint32_t handle = proxyIds.front();
				const glm::vec3 worldPos = camera.transformComponent().position;
				m_renderScene->updateCameraProxy(handle,
					camera.viewMatrix(), camera.projectionMatrix(), worldPos);
			}
		}
	}
}
