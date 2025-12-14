#include "vulkanbackend/VulkanPipeline.h"

#include <iostream>
#include <array>

VulkanPipeline::VulkanPipeline(VulkanContext& context, VulkanRenderPass& renderPass,
	VulkanPipelineLayout& pipelineLayout, VulkanShaderModule& vert, VulkanShaderModule& frag,
	uint32_t stride, uint32_t offsetCount, std::vector<uint32_t> offsets)
	:mContext(context),
	mRenderPass(renderPass),
	mPipelineLayout(pipelineLayout),
	mVert(vert),
	mFrag(frag),
	mStride(stride),
	mOffsetCount(offsetCount),
	mOffsets(offsets)
{
	createPipeline();

	std::cout << "Created pipeline." << std::endl;
}

VulkanPipeline::~VulkanPipeline()
{
	std::cout << "Deleting pipeline." << std::endl;

	if (mPipeline) {
		vkDestroyPipeline(mContext.device(), mPipeline, nullptr);
	}
}

void VulkanPipeline::createPipeline()
{
	VkPipelineShaderStageCreateInfo vertexStageInfo{};
	vertexStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertexStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertexStageInfo.module = mVert.module();
	vertexStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragmentStageInfo{};
	fragmentStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragmentStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragmentStageInfo.module = mFrag.module();
	fragmentStageInfo.pName = "main";

	std::vector<VkPipelineShaderStageCreateInfo> shaderStages{};
	shaderStages.push_back(vertexStageInfo);
	shaderStages.push_back(fragmentStageInfo);

	VkVertexInputBindingDescription vertexBindingDescription{};
	vertexBindingDescription.binding = 0;
	vertexBindingDescription.stride = mStride;
	vertexBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	std::array<VkVertexInputAttributeDescription, 2> ad{};
	ad[0].location = 0;
	ad[0].binding = 0;
	ad[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	ad[0].offset = mOffsets[0];

	ad[1].location = 1;
	ad[1].binding = 0;
	ad[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	ad[1].offset = mOffsets[1];

	//std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
	//for (int i = 0; i < mOffsetCount; i++) {
	//	VkVertexInputAttributeDescription vertexAttributeDescription{};
	//	vertexAttributeDescription.location = i;
	//	vertexAttributeDescription.binding = 0;
	//	vertexAttributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
	//	vertexAttributeDescription.offset = mOffsets[i];

	//	attributeDescriptions.push_back(vertexAttributeDescription);
	//}

	VkPipelineVertexInputStateCreateInfo vertexInfo{};
	vertexInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInfo.vertexBindingDescriptionCount = 1;
	vertexInfo.pVertexBindingDescriptions = &vertexBindingDescription;
	vertexInfo.vertexAttributeDescriptionCount = mOffsetCount;
	vertexInfo.pVertexAttributeDescriptions = ad.data();

	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;

	VkPipelineRasterizationStateCreateInfo rasterState{};
	rasterState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterState.depthClampEnable = VK_FALSE;
	rasterState.rasterizerDiscardEnable = VK_FALSE;
	rasterState.polygonMode = VK_POLYGON_MODE_FILL;
	rasterState.cullMode = VK_CULL_MODE_NONE;
	rasterState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterState.depthBiasEnable = VK_FALSE;
	rasterState.lineWidth = 1.0;

	VkPipelineMultisampleStateCreateInfo multisampleState{};
	multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampleState.sampleShadingEnable = VK_FALSE;

	VkPipelineDepthStencilStateCreateInfo depthStencilState{};
	depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilState.depthTestEnable = VK_TRUE;
	depthStencilState.depthWriteEnable = VK_TRUE;
	depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS;

	VkPipelineColorBlendAttachmentState blendAttachment{};
	blendAttachment.blendEnable = VK_FALSE;
	blendAttachment.colorWriteMask =
		VK_COLOR_COMPONENT_R_BIT |
		VK_COLOR_COMPONENT_G_BIT |
		VK_COLOR_COMPONENT_B_BIT |
		VK_COLOR_COMPONENT_A_BIT;

	VkPipelineColorBlendStateCreateInfo colorBlendState{};
	colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendState.logicOpEnable = VK_FALSE;
	colorBlendState.attachmentCount = 1;
	colorBlendState.pAttachments = &blendAttachment;

	std::vector<VkDynamicState> dynamicStates{
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
	dynamicState.pDynamicStates = dynamicStates.data();

	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = shaderStages.size();
	pipelineInfo.pStages = shaderStages.data();
	pipelineInfo.pVertexInputState = &vertexInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterState;
	pipelineInfo.pMultisampleState = &multisampleState;
	pipelineInfo.pDepthStencilState = &depthStencilState;
	pipelineInfo.pColorBlendState = &colorBlendState;
	pipelineInfo.layout = mPipelineLayout.pipelineLayout();
	pipelineInfo.renderPass = mRenderPass.renderPass();
	pipelineInfo.pDynamicState = &dynamicState;

	if (vkCreateGraphicsPipelines(mContext.device(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &mPipeline) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create graphics pipeline.");
	}
}