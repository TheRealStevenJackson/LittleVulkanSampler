#pragma once

#include <core/event/InputManager.h>
#include <engine/input/Controller.h>
#include <engine/scene/SceneManager.h>
#include <platform/Window.h>
#include <platform/GamepadReader.h>
#include <platform/Clock.h>

#include <glm/glm.hpp>

#include <memory>
#include <vector>

class VulkanContext;
class AssetManager;
namespace core { class Renderer; }

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

	void loadScene(const std::string& filepath);

	/**
	 * Load a single entity from OBJ paths (tries paths in order) and update material descriptor sets for the engine's renderer.
	 * \param pathsToTry Paths to try for loading the entity mesh.
	 * \param initialTransform Initial model matrix for the entity (default: identity at origin).
	 * \return true if loading succeeded, false if all paths failed.
	 */
	bool loadEntityTemporary(const std::vector<std::string>& pathsToTry, const glm::mat4& initialTransform = glm::mat4(1.0f));

	/** Call each frame to pump gamepad state into the input manager. */
	void pollGamepads() { m_gamepadReader.poll(); }

	/**
	 * Run the main loop until the window requests close.
	 * Each frame: polls events, updates input, ticks the clock, updates the scene, logs proxy data, and renders.
	 * Call vkDeviceWaitIdle after the loop so the GPU is idle when run() returns.
	 */
	void run();

	/** Engine clock; updated each run() frame. Use for delta time inside onFrame. */
	Clock& clock() { return m_clock; }
	const Clock& clock() const { return m_clock; }

private:
	Window m_window;
	core::InputManager m_inputManager;
	GamepadReader m_gamepadReader;
	Controller m_controller;
	Clock m_clock;

	std::unique_ptr<VulkanContext> m_context;
	std::unique_ptr<AssetManager> m_assetManager;
	SceneManager m_sceneManager;
	std::unique_ptr<core::Renderer> m_renderer;
};

} // namespace engine
