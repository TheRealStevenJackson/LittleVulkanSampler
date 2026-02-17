#include "engine/scene/SceneManager.h"
#include "core/common/RenderDataTypes.h"
#include "core/math/Transform.h"

#include <glm/gtc/quaternion.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>

#include <iostream>

namespace {

void logNode(const Node& node, int depth) {
	const std::string indent(depth * 2, ' ');
	const glm::vec3 translation(node.transform[3][0], node.transform[3][1], node.transform[3][2]);
	std::cout << indent << "node: translation=(" << translation.x << ", " << translation.y << ", " << translation.z
		<< "), meshes=" << node.meshIds.size()
		<< ", materials=" << node.materialIds.size()
		<< ", children=" << node.children.size() << std::endl;
	for (size_t i = 0; i < node.children.size(); ++i)
		logNode(node.children[i], depth + 1);
}

} // namespace

SceneManager::SceneManager(AssetManager& assetManager)
	: m_assetManager(&assetManager)
{
}

void SceneManager::loadScene(const std::string& filepath) {
	auto scene = m_assetManager->loadScene(filepath);
	// if (!scene)
	// 	return;
	// std::cout << "Scene: " << scene->rootNodes.size() << " root node(s)" << std::endl;
	// for (size_t i = 0; i < scene->rootNodes.size(); ++i) {
	// 	std::cout << "Root node " << i << ":" << std::endl;
	// 	logNode(scene->rootNodes[i], 1);
	// }
	m_assetManager->logAssets();
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
	// TODO: Uncomment when Entity::update() is fixed
	// for (Camera& camera : m_cameras)
	// 	camera.update(dt);
}
