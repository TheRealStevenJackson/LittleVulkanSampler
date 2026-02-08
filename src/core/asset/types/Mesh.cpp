#include "core/asset/types/Mesh.h"

Mesh::Mesh(VulkanContext& context,
		   const void* vertexData,
		   VkDeviceSize vertexSize,
		   uint32_t vertexCount)
	: mContext(context)
	, mVertexCount(vertexCount)
	, mIndexCount(0)
	, mIndexType(VK_INDEX_TYPE_UINT32)
{
	// Initialize vertex buffer in constructor (RAII)
	mVertexBuffer = std::make_unique<VulkanBuffer>(
		mContext,
		vertexData,
		vertexSize,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT
	);
}

Mesh::Mesh(VulkanContext& context,
		   const void* vertexData,
		   VkDeviceSize vertexSize,
		   uint32_t vertexCount,
		   const void* indexData,
		   VkDeviceSize indexSize,
		   uint32_t indexCount,
		   VkIndexType indexType)
	: mContext(context)
	, mVertexCount(vertexCount)
	, mIndexCount(indexCount)
	, mIndexType(indexType)
{
	// Initialize vertex buffer in constructor (RAII)
	mVertexBuffer = std::make_unique<VulkanBuffer>(
		mContext,
		vertexData,
		vertexSize,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT
	);

	// Initialize index buffer in constructor (RAII)
	mIndexBuffer = std::make_unique<VulkanBuffer>(
		mContext,
		indexData,
		indexSize,
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT
	);
}

Mesh::~Mesh() {
	// Buffers are automatically destroyed via unique_ptr
	// VulkanBuffer destructor handles cleanup
}

void Mesh::bindVertexBuffer(VkCommandBuffer commandBuffer) const {
	if (mVertexBuffer) {
		VkBuffer buffer = mVertexBuffer->buffer();
		VkDeviceSize offset = 0;
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &buffer, &offset);
	}
}

void Mesh::bindIndexBuffer(VkCommandBuffer commandBuffer) const {
	if (mIndexBuffer) {
		vkCmdBindIndexBuffer(
			commandBuffer,
			mIndexBuffer->buffer(),
			0,
			mIndexType
		);
	}
}

void Mesh::draw(VkCommandBuffer commandBuffer) const {
	if (hasIndices()) {
		vkCmdDrawIndexed(commandBuffer, mIndexCount, 1, 0, 0, 0);
	} else {
		vkCmdDraw(commandBuffer, mVertexCount, 1, 0, 0);
	}
}
