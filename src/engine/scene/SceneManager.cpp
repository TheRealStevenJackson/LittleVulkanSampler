#include "engine/scene/SceneManager.h"
#include "core/common/RenderDataTypes.h"
#include "core/math/Transform.h"

#include <algorithm>
#include <cctype>

static bool pathLooksLikeGltf(const std::string& path) {
	std::string ext;
	auto pos = path.find_last_of('.');
	if (pos != std::string::npos && pos + 1 < path.size()) {
		ext = path.substr(pos + 1);
		for (auto& c : ext) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
	}
	return ext == "gltf" || ext == "glb";
}

SceneManager::SceneManager(AssetManager& assetManager)
	: m_assetManager(&assetManager)
	, m_loader(assetManager)
{
}

bool SceneManager::loadScene(const std::string& sceneName,
	const std::vector<std::string>& pathsToTry)
{
	auto resource = loadScene(pathsToTry);
	if (!resource.has_value())
		return false;
	m_scenes[sceneName] = std::move(*resource);
	return true;
}

std::optional<SceneResource> SceneManager::loadScene(const std::vector<std::string>& pathsToTry)
{
	if (pathsToTry.empty())
		return std::nullopt;

	// If any path looks like glTF/GLB, try glTF first; otherwise try OBJ first.
	bool tryGltfFirst = std::any_of(pathsToTry.begin(), pathsToTry.end(), pathLooksLikeGltf);

	SceneResource out;
	std::string loadedPath;

	if (tryGltfFirst) {
		if (m_loader.loadGltfFromPaths(pathsToTry, out.entities(), loadedPath)) {
			out.setLoadedPath(loadedPath);
			return out;
		}
		if (m_loader.loadFromPaths(pathsToTry, out.entities(), loadedPath)) {
			out.setLoadedPath(loadedPath);
			return out;
		}
	} else {
		if (m_loader.loadFromPaths(pathsToTry, out.entities(), loadedPath)) {
			out.setLoadedPath(loadedPath);
			return out;
		}
		if (m_loader.loadGltfFromPaths(pathsToTry, out.entities(), loadedPath)) {
			out.setLoadedPath(loadedPath);
			return out;
		}
	}

	return std::nullopt;
}

SceneResource* SceneManager::getScene(const std::string& sceneName) {
	auto it = m_scenes.find(sceneName);
	return it != m_scenes.end() ? &it->second : nullptr;
}

const SceneResource* SceneManager::getScene(const std::string& sceneName) const {
	auto it = m_scenes.find(sceneName);
	return it != m_scenes.end() ? &it->second : nullptr;
}

void SceneManager::setCurrentScene(const std::string& sceneName) {
	SceneResource* r = getScene(sceneName);
	m_currentScene = r;
}

void SceneManager::unloadScene(const std::string& sceneName) {
	if (m_currentScene && m_scenes.count(sceneName)) {
		// If we're unloading the current scene, clear current pointer
		if (getScene(sceneName) == m_currentScene)
			m_currentScene = nullptr;
	}
	m_scenes.erase(sceneName);
}

bool SceneManager::loadEntityTemporary(const std::vector<std::string>& pathsToTry,
	engine::Controller* controller,
	VulkanDescriptorSetLayout* materialDescriptorLayout)
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

	m_loadedEntities.clear();
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
