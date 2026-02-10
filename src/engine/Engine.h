#pragma once

#include <core/event/InputManager.h>
#include <engine/input/Controller.h>
#include <engine/scene/SceneManager.h>
#include <platform/Window.h>
#include <platform/GamepadReader.h>

#include <memory>

class VulkanContext;
class AssetManager;

namespace engine {

/**
 * Central engine facade: owns window, input, Vulkan context, assets, and scene management.
 * Construct with a WindowDesc; then use context(), assetManager(), sceneManager(), and
 * controller() for rendering and game logic. Call pollEvents() / shouldClose() each frame.
 */
class Engine {
public:
	explicit Engine(const WindowDesc& desc);
	~Engine();

	Engine(const Engine&) = delete;
	Engine& operator=(const Engine&) = delete;

	/** Call each frame to process window and input events. */
	void pollEvents();

	/** True when the window has been requested to close. */
	bool shouldClose() const;

	Window& window() { return m_window; }
	const Window& window() const { return m_window; }

	core::InputManager& inputManager() { return m_inputManager; }
	const core::InputManager& inputManager() const { return m_inputManager; }

	Controller& controller() { return m_controller; }
	const Controller& controller() const { return m_controller; }

	VulkanContext& context();
	const VulkanContext& context() const;

	AssetManager& assetManager();
	const AssetManager& assetManager() const;

	SceneManager& sceneManager() { return m_sceneManager; }
	const SceneManager& sceneManager() const { return m_sceneManager; }

	/** Call each frame to pump gamepad state into the input manager. */
	void pollGamepads() { m_gamepadReader.poll(); }

private:
	Window m_window;
	core::InputManager m_inputManager;
	GamepadReader m_gamepadReader;
	Controller m_controller;

	std::unique_ptr<VulkanContext> m_context;
	std::unique_ptr<AssetManager> m_assetManager;
	SceneManager m_sceneManager;
};

} // namespace engine
