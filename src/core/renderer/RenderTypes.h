#pragma once

#include <glm/glm.hpp>

namespace core {

// Vertex layout matching loaded mesh (pos, normal, texCoord, tangent, bitangent)
struct Vertex {
	float pos[3];
	float normal[3];
	float texCoord[2];
	float tangent[3];
	float bitangent[3];
};

// Set 0: view + proj (model moved to set 2)
struct ViewProjUBO {
	float view[16];
	float proj[16];
};

struct DirectionalLightUBO {
	float direction[4];
	float color[4];
};

struct ModelUBO {
	glm::mat4 model;
};



} // namespace core
