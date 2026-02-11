#pragma once

#include "Entity.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

/**
 * Camera entity. Extends Entity to use its transform (position, rotation)
 * for the camera in world space. Provides view and projection matrices.
 */
class Camera : public Entity {
public:
	Camera() = default;
	~Camera() = default;

	Camera(const Camera&) = delete;
	Camera& operator=(const Camera&) = delete;
	Camera(Camera&&) noexcept = default;
	Camera& operator=(Camera&&) noexcept = default;

	/** View matrix (world space -> camera space). Uses set value or inverse of the camera's model matrix. */
	glm::mat4 viewMatrix() const {
		if (m_useExplicitView)
			return m_view;
		return glm::inverse(transformComponent().modelMatrix());
	}

	/** Perspective projection matrix. Uses set value or built from setPerspective() params. */
	glm::mat4 projectionMatrix() const {
		if (m_useExplicitProj)
			return m_proj;
		glm::mat4 p = glm::perspective(m_fovY, m_aspectRatio, m_nearZ, m_farZ);
		p[1][1] *= -1.0f;  // Flip Y for Vulkan NDC
		return p;
	}

	/** Set initial view matrix (used when provided to loadCameraTemporary). */
	void setViewMatrix(const glm::mat4& view) {
		m_view = view;
		m_useExplicitView = true;
	}

	/** Set initial projection matrix (used when provided to loadCameraTemporary). */
	void setProjectionMatrix(const glm::mat4& proj) {
		m_proj = proj;
		m_useExplicitProj = true;
	}

	/** Set perspective projection (fovY in radians, aspect ratio, near/far planes). */
	void setPerspective(float fovYRadians, float aspectRatio, float nearZ, float farZ) {
		m_fovY = fovYRadians;
		m_aspectRatio = aspectRatio;
		m_nearZ = nearZ;
		m_farZ = farZ;
	}

	float fovY() const { return m_fovY; }
	float aspectRatio() const { return m_aspectRatio; }
	float nearZ() const { return m_nearZ; }
	float farZ() const { return m_farZ; }

private:
	float m_fovY = glm::radians(60.0f);
	float m_aspectRatio = 16.0f / 9.0f;
	float m_nearZ = 0.01f;
	float m_farZ = 200.0f;
	glm::mat4 m_view = glm::mat4(1.0f);
	glm::mat4 m_proj = glm::mat4(1.0f);
	bool m_useExplicitView = false;
	bool m_useExplicitProj = false;
};
