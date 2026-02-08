#pragma once

#include "platform/graphics/vulkan/VulkanContext.h"
#include "platform/graphics/vulkan/VulkanBuffer.h"

#include <vulkan/vulkan.h>
#include <cstdint>
#include <memory>
#include <vector>

class Mesh {
public:
	// Constructor for mesh with vertices only (non-indexed)
	Mesh(VulkanContext& context, 
		 const void* vertexData, 
		 VkDeviceSize vertexSize, 
		 uint32_t vertexCount);

	// Constructor for mesh with vertices and indices
	Mesh(VulkanContext& context,
		 const void* vertexData,
		 VkDeviceSize vertexSize,
		 uint32_t vertexCount,
		 const void* indexData,
		 VkDeviceSize indexSize,
		 uint32_t indexCount,
		 VkIndexType indexType = VK_INDEX_TYPE_UINT32);

	// Delete copy constructor and assignment (RAII - non-copyable)
	Mesh(const Mesh&) = delete;
	Mesh& operator=(const Mesh&) = delete;

	// Allow move semantics
	Mesh(Mesh&&) noexcept = default;
	Mesh& operator=(Mesh&&) noexcept = default;

	~Mesh();

	// Bind buffers for rendering
	void bindVertexBuffer(VkCommandBuffer commandBuffer) const;
	void bindIndexBuffer(VkCommandBuffer commandBuffer) const;

	// Draw the mesh
	void draw(VkCommandBuffer commandBuffer) const;

	// Getters
	uint32_t indexCount() const { return mIndexCount; }
	uint32_t vertexCount() const { return mVertexCount; }
	bool hasIndices() const { return mIndexCount > 0; }

private:
	VulkanContext& mContext;
	
	std::unique_ptr<VulkanBuffer> mVertexBuffer;
	std::unique_ptr<VulkanBuffer> mIndexBuffer;
	
	uint32_t mVertexCount;
	uint32_t mIndexCount;
	VkIndexType mIndexType;
};
