#pragma once

#include "core/asset/Mesh.h"
#include "core/asset/Material.h"
#include "platform/graphics/vulkan/VulkanContext.h"

#include <string>
#include <vector>
#include <memory>

class ObjLoader {
public:
	ObjLoader(VulkanContext& context);
	~ObjLoader();

	// Load an OBJ file and return a vector of meshes (one per shape in the OBJ)
	// Returns empty vector on failure
	std::vector<std::unique_ptr<Mesh>> loadFromFile(const std::string& filepath);

	// Load an OBJ file and combine all shapes into a single mesh
	// Returns nullptr on failure
	std::unique_ptr<Mesh> loadFromFileCombined(const std::string& filepath);

	// Extract MaterialPaths from loaded materials
	// Returns a vector of MaterialPaths, one per material found in the MTL file
	// baseDir is the directory containing the OBJ/MTL files (for resolving relative texture paths)
	std::vector<MaterialPaths> extractMaterialPaths(const std::string& filepath);

private:
	VulkanContext& mContext;
};
