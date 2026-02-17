#pragma once

#include "core/asset/types/Node.h"

#include <vector>

/**
 * Scene containing a forest of root nodes. Each root node may have its own
 * transform, mesh/material refs, and child nodes.
 */
struct Scene {
	std::vector<Node> rootNodes;
};
