#pragma once

#include "Entity.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

/**
 * Camera entity. Extends Entity to use its transform (position, rotation)
 * for the camera in world space. Provides view and projection matrices.
 * When a controller is set, the current view is decomposed into position/rotation
 * and the camera is driven from the transform (controller updates rotation each frame).
 */
class Camera : public Entity {
public:
	Camera() = default;
	~Camera() = default;

	Camera(const Camera&) = delete;
	Camera& operator=(const Camera&) = delete;
	Camera(Camera&&) noexcept = default;
	Camera& operator=(Camera&&) noexcept = default;

	/** Set controller; if non-null and we have an explicit view, sync transform from it and drive view from transform. */
	void setController(engine::Controller* controller) override {
		Entity::setController(controller);
		if (controller && m_useExplicitView) {
			const glm::mat4 invView = glm::inverse(m_view);
			transformComponent().position = glm::vec3(invView[3]);
			const glm::mat3 R(invView);
			transformComponent().rotation = glm::eulerAngles(glm::quat_cast(R));
			m_useExplicitView = false;
		}
	}

	/** Update rotation from right stick and position from left stick (when controller is set). */
	void update(float dt) override {
		engine::Controller* ctrl = controller();
		if (ctrl) {
			const float rotationSpeedScale = 1.0f;
			transformComponent().rotation.y -= dt * ctrl->rightStickX.load() * rotationSpeedScale;
			transformComponent().rotation.x -= dt * ctrl->rightStickY.load() * rotationSpeedScale;
			const glm::mat4 M = transformComponent().modelMatrix();
			const glm::vec3 right = glm::normalize(glm::vec3(M[0][0], M[0][1], M[0][2]));
			const glm::vec3 forward = -glm::normalize(glm::vec3(M[2][0], M[2][1], M[2][2]));
			const float sx = ctrl->leftStickX.load() * m_moveSpeed * dt;
			const float sy = -ctrl->leftStickY.load() * m_moveSpeed * dt;
			transformComponent().position += right * sx + forward * sy;
		}
	}

	/** Movement speed (units per second per stick axis). Default 2. Use higher values for large scenes (e.g. house). */
	void setMoveSpeed(float speed) { m_moveSpeed = speed; }
	float moveSpeed() const { return m_moveSpeed; }

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
	float m_moveSpeed = 2.0f;
	float m_fovY = glm::radians(60.0f);
	float m_aspectRatio = 16.0f / 9.0f;
	float m_nearZ = 0.01f;
	float m_farZ = 200.0f;
	glm::mat4 m_view = glm::mat4(1.0f);
	glm::mat4 m_proj = glm::mat4(1.0f);
	bool m_useExplicitView = false;
	bool m_useExplicitProj = false;
};
