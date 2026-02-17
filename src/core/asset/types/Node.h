#pragma once

#include <cstdint>
#include <glm/glm.hpp>
#include <vector>

/**
 * Hierarchy node that can reference meshes and materials and contain child nodes.
 * Used to represent scene graphs or glTF node trees.
 */
struct Node {
	glm::mat4 transform = glm::mat4(1.0f);
	std::vector<uint32_t> meshIds;
	std::vector<uint32_t> materialIds;
	std::vector<Node> children;
};
