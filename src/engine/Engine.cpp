#include "engine/Engine.h"
#include <platform/graphics/vulkan/VulkanContext.h>
#include <core/asset/AssetManager.h>
#include <engine/scene/SceneManager.h>

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

} // namespace engine
