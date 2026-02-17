#pragma once

#include "engine/scene/object/component/RenderComponent.h"
#include "engine/scene/object/component/TransformComponent.h"
#include "engine/input/Controller.h"
#include <glm/glm.hpp>

/**
 * Entity in the game layer.
 * Owns components like RenderComponent for rendering.
 */
class Entity {
public:
	Entity() = default;
	~Entity() = default;

	// Non-copyable (components may contain non-copyable resources)
	Entity(const Entity&) = delete;
	Entity& operator=(const Entity&) = delete;

	// Allow move semantics
	Entity(Entity&&) noexcept = default;
	Entity& operator=(Entity&&) noexcept = default;

	// Component access
	RenderComponent& renderComponent() { return m_renderComponent; }
	const RenderComponent& renderComponent() const { return m_renderComponent; }

	TransformComponent& transformComponent() { return m_transform; }
	const TransformComponent& transformComponent() const { return m_transform; }

	virtual void setController(engine::Controller* controller) { m_controller = controller; }
	engine::Controller* controller() { return m_controller; }
	const engine::Controller* controller() const { return m_controller; }

	/** Model matrix from transform component (position, rotation, scale). */
	glm::mat4 model() const { return m_transform.modelMatrix(); }

	virtual void update(float dt) {
		const float rotationSpeedScale = 1.0f;
		if (m_controller) {
			// LeftStickX -> rotation around Y axis (yaw)
			// LeftStickY -> rotation around X axis (pitch)
			m_transform.rotation.y += dt * m_controller->leftStickX.load() * rotationSpeedScale;
			m_transform.rotation.x += dt * m_controller->leftStickY.load() * rotationSpeedScale;
		} else {
			m_transform.rotation.y += dt * 1.0f;
		}
	}

private:
	RenderComponent m_renderComponent;
	TransformComponent m_transform;
	engine::Controller* m_controller = nullptr;
};
