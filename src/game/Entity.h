#pragma once

#include "game/RenderComponent.h"
#include "engine/input/Controller.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

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

	void setController(engine::Controller* controller) { m_controller = controller; }
	engine::Controller* controller() { return m_controller; }
	const engine::Controller* controller() const { return m_controller; }

	glm::mat4& model() { return m_model; }
	const glm::mat4& model() const { return m_model; }

	void update(float dt) {
		const float rotationSpeedScale = 1.0f;
		if (m_controller) {
			// LeftStickX -> rotation around Y axis (yaw)
			// LeftStickY -> rotation around X axis (pitch)
			m_angleY += dt * m_controller->leftStickX.load() * rotationSpeedScale;
			m_angleX += dt * m_controller->leftStickY.load() * rotationSpeedScale;
		} else {
			m_angleY += dt * 1.0f;
		}
		// Apply Y rotation first, then X rotation
		m_model = glm::mat4(1.0f);
		m_model = glm::rotate(m_model, m_angleY, glm::vec3(0, 1, 0));
		m_model = glm::rotate(m_model, m_angleX, glm::vec3(1, 0, 0));
	}

private:
	RenderComponent m_renderComponent;
	engine::Controller* m_controller = nullptr;
	glm::mat4 m_model = glm::mat4(1.0f);
	float m_angleX = 0.0f;  // rotation around X (pitch), from LeftStickY
	float m_angleY = 0.0f;  // rotation around Y (yaw), from LeftStickX
};
