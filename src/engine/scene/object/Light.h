#pragma once

#include "Entity.h"
#include <glm/glm.hpp>

/**
 * Light entity. Extends Entity to use its transform (position, rotation)
 * for the light in world space. Supports point, directional, and spot lights.
 */
class Light : public Entity {
public:
	enum class Type {
		Point,
		Directional,
		Spot
	};

	Light() = default;
	~Light() = default;

	Light(const Light&) = delete;
	Light& operator=(const Light&) = delete;
	Light(Light&&) noexcept = default;
	Light& operator=(Light&&) noexcept = default;

	Type type() const { return m_type; }
	void setType(Type type) { m_type = type; }

	/** Light color (RGB). */
	const glm::vec3& color() const { return m_color; }
	void setColor(const glm::vec3& color) { m_color = color; }

	/** Light intensity (multiplier). */
	float intensity() const { return m_intensity; }
	void setIntensity(float intensity) { m_intensity = intensity; }

	/** World-space direction (for directional/spot). Uses explicit direction if set, else from transform. */
	glm::vec3 direction() const {
		if (m_useExplicitDirection)
			return glm::normalize(m_explicitDirection);
		glm::mat4 m = transformComponent().modelMatrix();
		return glm::normalize(glm::vec3(m[2][0], m[2][1], m[2][2]));
	}
	/** Set explicit direction (e.g. for directional lights). When set, direction() returns this instead of transform. */
	void setDirection(const glm::vec3& d) {
		m_explicitDirection = d;
		m_useExplicitDirection = (glm::length(d) > 0.0f);
	}

	/** World-space position (from transform). */
	glm::vec3 position() const {
		return transformComponent().position;
	}

	// --- Point / spot ---
	/** Attenuation range (distance at which influence effectively ends). */
	float range() const { return m_range; }
	void setRange(float range) { m_range = range; }

	// --- Spot ---
	float innerConeAngle() const { return m_innerConeAngle; }
	void setInnerConeAngle(float radians) { m_innerConeAngle = radians; }
	float outerConeAngle() const { return m_outerConeAngle; }
	void setOuterConeAngle(float radians) { m_outerConeAngle = radians; }

private:
	Type m_type = Type::Point;
	glm::vec3 m_color = glm::vec3(1.0f);
	float m_intensity = 1.0f;
	float m_range = 10.0f;
	float m_innerConeAngle = 0.0f;
	float m_outerConeAngle = glm::radians(15.0f);
	bool m_useExplicitDirection = false;
	glm::vec3 m_explicitDirection = glm::vec3(0.0f, 0.0f, -1.0f);
};
