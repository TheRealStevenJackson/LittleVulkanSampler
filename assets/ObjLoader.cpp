#define TINYOBJLOADER_IMPLEMENTATION
#include "assets/ObjLoader.h"
#include "tiny_obj_loader.h"

#include <glm/glm.hpp>
#include <iostream>
#include <unordered_map>

// Simple vertex structure with position, normal, and texcoord
struct Vertex {
	glm::vec3 pos;
	glm::vec3 normal;
	glm::vec2 texCoord;
};

ObjLoader::ObjLoader(VulkanContext& context)
	: mContext(context)
{
}

ObjLoader::~ObjLoader() = default;

std::vector<std::unique_ptr<Mesh>> ObjLoader::loadFromFile(const std::string& filepath) {
	std::vector<std::unique_ptr<Mesh>> meshes;

	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn, err;

	// Load the OBJ file
	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.c_str());

	if (!warn.empty()) {
		std::cout << "Warning loading OBJ: " << warn << std::endl;
	}

	if (!err.empty()) {
		std::cerr << "Error loading OBJ: " << err << std::endl;
	}

	if (!ret) {
		std::cerr << "Failed to load OBJ file: " << filepath << std::endl;
		return meshes;
	}

	// Process each shape in the OBJ file
	for (const auto& shape : shapes) {
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
		std::unordered_map<std::string, uint32_t> uniqueVertices;

		// Process each face
		for (size_t f = 0; f < shape.mesh.indices.size() / 3; f++) {
			tinyobj::index_t idx0 = shape.mesh.indices[3 * f + 0];
			tinyobj::index_t idx1 = shape.mesh.indices[3 * f + 1];
			tinyobj::index_t idx2 = shape.mesh.indices[3 * f + 2];

			// Process three vertices of the triangle
			for (int v = 0; v < 3; v++) {
				tinyobj::index_t idx = shape.mesh.indices[3 * f + v];

				// Create a unique key for this vertex
				std::string vertexKey = std::to_string(idx.vertex_index) + "/" +
					(idx.normal_index >= 0 ? std::to_string(idx.normal_index) : "x") + "/" +
					(idx.texcoord_index >= 0 ? std::to_string(idx.texcoord_index) : "x");

				// Check if we've seen this vertex before
				auto it = uniqueVertices.find(vertexKey);
				if (it != uniqueVertices.end()) {
					// Reuse existing vertex
					indices.push_back(it->second);
				} else {
					// Create new vertex
					Vertex vertex{};

					// Position
					if (idx.vertex_index >= 0) {
						vertex.pos = glm::vec3(
							attrib.vertices[3 * idx.vertex_index + 0],
							attrib.vertices[3 * idx.vertex_index + 1],
							attrib.vertices[3 * idx.vertex_index + 2]
						);
					}

					// Normal
					if (idx.normal_index >= 0) {
						vertex.normal = glm::vec3(
							attrib.normals[3 * idx.normal_index + 0],
							attrib.normals[3 * idx.normal_index + 1],
							attrib.normals[3 * idx.normal_index + 2]
						);
					} else {
						vertex.normal = glm::vec3(0.0f, 0.0f, 1.0f); // Default normal
					}

					// TexCoord
					if (idx.texcoord_index >= 0) {
						vertex.texCoord = glm::vec2(
							attrib.texcoords[2 * idx.texcoord_index + 0],
							1.0f - attrib.texcoords[2 * idx.texcoord_index + 1] // Flip V coordinate
						);
					} else {
						vertex.texCoord = glm::vec2(0.0f, 0.0f);
					}

					uint32_t vertexIndex = static_cast<uint32_t>(vertices.size());
					vertices.push_back(vertex);
					uniqueVertices[vertexKey] = vertexIndex;
					indices.push_back(vertexIndex);
				}
			}
		}

		// Create mesh for this shape (RAII - buffers initialized in constructor)
		if (!vertices.empty()) {
			std::unique_ptr<Mesh> mesh;
			if (!indices.empty()) {
				// Create mesh with vertices and indices
				mesh = std::make_unique<Mesh>(
					mContext,
					vertices.data(),
					vertices.size() * sizeof(Vertex),
					static_cast<uint32_t>(vertices.size()),
					indices.data(),
					indices.size() * sizeof(uint32_t),
					static_cast<uint32_t>(indices.size()),
					VK_INDEX_TYPE_UINT32
				);
			} else {
				// Create mesh with vertices only
				mesh = std::make_unique<Mesh>(
					mContext,
					vertices.data(),
					vertices.size() * sizeof(Vertex),
					static_cast<uint32_t>(vertices.size())
				);
			}
			meshes.push_back(std::move(mesh));
		}
	}

	return meshes;
}

std::unique_ptr<Mesh> ObjLoader::loadFromFileCombined(const std::string& filepath) {
	auto meshes = loadFromFile(filepath);
	
	if (meshes.empty()) {
		return nullptr;
	}

	// If only one mesh, return it
	if (meshes.size() == 1) {
		return std::move(meshes[0]);
	}

	// Combine all meshes into one
	// For simplicity, we'll just return the first mesh
	// A more complete implementation would merge all vertices and indices
	std::cout << "Warning: Multiple shapes found, returning first mesh only" << std::endl;
	return std::move(meshes[0]);
}
