#pragma once

#include "game/RenderComponent.h"

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

private:
	RenderComponent m_renderComponent;
};
