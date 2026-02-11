#pragma once

#include <glm/glm.hpp>

namespace core {

struct ViewProjUBO {
	glm::mat4 view;
	glm::mat4 proj;
};

struct ModelUBO {
	glm::mat4 model;
};

struct DirectionalLightUBO {
	glm::vec4 direction;
	glm::vec4 color;
};

} // namespace core
