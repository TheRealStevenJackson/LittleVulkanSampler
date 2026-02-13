#include "engine/Engine.h"
#include <core/renderer/Renderer.h>
#include <platform/graphics/vulkan/VulkanContext.h>
#include <core/asset/AssetManager.h>
#include <engine/scene/SceneManager.h>
#include <vulkan/vulkan.h>

namespace engine {

Engine::Engine(const WindowDesc& desc)
	: m_window(desc)
	, m_inputManager()
	, m_gamepadReader()
	, m_controller(m_inputManager)
	, m_context(std::make_unique<VulkanContext>(m_window.getHandle()))
	, m_assetManager(std::make_unique<AssetManager>(*m_context))
	, m_sceneManager(*m_assetManager)
{
	m_window.setInputReceiver(&m_inputManager);
	m_gamepadReader.setInputReceiver(&m_inputManager);
	m_renderer = std::make_unique<core::Renderer>(*m_context, m_window, *m_assetManager);
	m_sceneManager.setRenderScene(m_renderer->getRenderScene());
}

Engine::~Engine() = default;

void Engine::pollEvents() {
	m_window.pollEvents();
}

bool Engine::shouldClose() const {
	return m_window.shouldClose();
}

VulkanContext& Engine::context() {
	return *m_context;
}

const VulkanContext& Engine::context() const {
	return *m_context;
}

AssetManager& Engine::assetManager() {
	return *m_assetManager;
}

const AssetManager& Engine::assetManager() const {
	return *m_assetManager;
}

void Engine::run() {
	while (!shouldClose()) {
		pollEvents();
		pollGamepads();
		m_inputManager.update();
		m_clock.tick();
		const float dt = static_cast<float>(m_clock.deltaTime());
		sceneManager().update(dt);
		m_renderer->getRenderScene()->logProxyData();
		m_renderer->renderFrame();
	}
	vkDeviceWaitIdle(context().device());
}

bool Engine::loadEntityTemporary(const std::vector<std::string>& pathsToTry) {
	return m_sceneManager.loadEntityTemporary(pathsToTry, &m_controller,
		m_renderer ? m_renderer->getMaterialDescriptorLayout() : nullptr);
}

} // namespace engine
