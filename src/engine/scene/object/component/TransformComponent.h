#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

/**
 * Component storing local transform (position, rotation, scale).
 * Used to build the model matrix for rendering and physics.
 */
struct TransformComponent {
	glm::vec3 position = glm::vec3(0.0f);
	glm::vec3 rotation = glm::vec3(0.0f);  // Euler angles (radians): X=pitch, Y=yaw, Z=roll
	glm::vec3 scale = glm::vec3(1.0f);

	/** Builds the local model matrix from position, rotation, scale. */
	glm::mat4 modelMatrix() const {
		glm::mat4 m = glm::mat4(1.0f);
		m = glm::translate(m, position);
		m = glm::rotate(m, rotation.y, glm::vec3(0, 1, 0));
		m = glm::rotate(m, rotation.x, glm::vec3(1, 0, 0));
		m = glm::rotate(m, rotation.z, glm::vec3(0, 0, 1));
		m = glm::scale(m, scale);
		return m;
	}
};
