#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace core {

/**
 * Wraps a glm 4x4 transformation matrix.
 * Supports identity, construction from mat4, and chainable translate/rotate/scale.
 */
class Transform {
public:
	Transform() : m_matrix(1.0f) {}
	explicit Transform(const glm::mat4& matrix) : m_matrix(matrix) {}

	/** Read-only access to the underlying 4x4 matrix. */
	const glm::mat4& matrix() const { return m_matrix; }

	/** Mutable access to the underlying 4x4 matrix. */
	glm::mat4& matrix() { return m_matrix; }

	/** Implicit conversion to glm::mat4 for use in GLM APIs. */
	operator glm::mat4() const { return m_matrix; }

	/** Reset to identity. */
	Transform& setIdentity() {
		m_matrix = glm::mat4(1.0f);
		return *this;
	}

	/** Multiply by translation; returns *this for chaining. */
	Transform& translate(const glm::vec3& v) {
		m_matrix = glm::translate(m_matrix, v);
		return *this;
	}

	/** Multiply by rotation (angle in radians around axis); returns *this for chaining. */
	Transform& rotate(float angleRad, const glm::vec3& axis) {
		m_matrix = glm::rotate(m_matrix, angleRad, axis);
		return *this;
	}

	/** Multiply by scale; returns *this for chaining. */
	Transform& scale(const glm::vec3& s) {
		m_matrix = glm::scale(m_matrix, s);
		return *this;
	}

	/** Compose with another transform: this = this * other. */
	Transform& combine(const Transform& other) {
		m_matrix = m_matrix * other.m_matrix;
		return *this;
	}

	/** Compose with another transform: this = other * this. */
	Transform& combinePre(const Transform& other) {
		m_matrix = other.m_matrix * m_matrix;
		return *this;
	}

	/** Return inverse transform (new instance). */
	Transform inverted() const {
		return Transform(glm::inverse(m_matrix));
	}

	/** Create an identity transform. */
	static Transform identity() {
		return Transform(glm::mat4(1.0f));
	}

	/** Create transform from position, Euler rotation (radians: X=pitch, Y=yaw, Z=roll), and scale. */
	static Transform fromPositionRotationScale(
		const glm::vec3& position,
		const glm::vec3& rotationRadians,
		const glm::vec3& scaleVec)
	{
		Transform t;
		t.translate(position);
		t.rotate(rotationRadians.y, glm::vec3(0, 1, 0));
		t.rotate(rotationRadians.x, glm::vec3(1, 0, 0));
		t.rotate(rotationRadians.z, glm::vec3(0, 0, 1));
		t.scale(scaleVec);
		return t;
	}

private:
	glm::mat4 m_matrix;
};

} // namespace core
