#pragma once

#include "engine/graphics/renderer/VulkanContext.h"
#include "engine/graphics/renderer/VulkanRenderPass.h"
#include "engine/graphics/renderer/VulkanPipelineLayout.h"
#include "engine/graphics/renderer/VulkanShaderModule.h"

class VulkanPipeline {
public:
	VulkanPipeline(VulkanContext&, VulkanRenderPass&, VulkanPipelineLayout&,
		VulkanShaderModule&, VulkanShaderModule&, uint32_t, uint32_t, std::vector<uint32_t>);
	~VulkanPipeline();

	VkPipeline pipeline() const { return mPipeline; }

private:
	void createPipeline();

	VulkanContext& mContext;
	VulkanRenderPass& mRenderPass;
	VulkanPipelineLayout& mPipelineLayout;
	VulkanShaderModule& mVert;
	VulkanShaderModule& mFrag;
	uint32_t mStride;
	uint32_t mOffsetCount;
	std::vector<uint32_t> mOffsets;

	VkPipeline mPipeline = VK_NULL_HANDLE;
};