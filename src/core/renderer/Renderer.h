#pragma once

#include <engine/Engine.h>
#include <core/asset/AssetManager.h>
#include <platform/Window.h>
#include <platform/graphics/vulkan/VulkanContext.h>
#include <platform/graphics/vulkan/VulkanSwapchain.h>
#include <platform/graphics/vulkan/VulkanRenderPass.h>
#include <platform/graphics/vulkan/VulkanImage.h>
#include <platform/graphics/vulkan/VulkanFramebuffer.h>
#include <platform/graphics/vulkan/VulkanDescriptorSetLayout.h>
#include <platform/graphics/vulkan/VulkanPipelineLayout.h>
#include <platform/graphics/vulkan/VulkanPipeline.h>
#include <platform/graphics/vulkan/VulkanBuffer.h>
#include <platform/graphics/vulkan/VulkanDescriptorPool.h>
#include <platform/graphics/vulkan/VulkanDescriptorSet.h>
#include <platform/graphics/vulkan/VulkanFrameManager.h>
#include "core/renderer/RenderScene.h"

#include <memory>
#include <vector>

class AssetManager;

namespace core {

/**
 * Owns swapchain, render pass, framebuffers, pipeline, descriptor sets,
 * camera/light UBOs, and frame synchronization. Builds the default PBR
 * pipeline (spinning_cube shaders, camera + directional light + material sets).
 */
class Renderer {
public:
	Renderer(VulkanContext& ctx, Window& window, AssetManager& assetManager);
	~Renderer();

	Renderer(const Renderer&) = delete;
	Renderer& operator=(const Renderer&) = delete;

	/** Performs one frame of rendering. */
	void renderFrame();

	/** Material descriptor layout; used to update material descriptor sets after loading entities. */
	VulkanDescriptorSetLayout* getMaterialDescriptorLayout() { return m_materialDescriptorLayout.get(); }
	const VulkanDescriptorSetLayout* getMaterialDescriptorLayout() const { return m_materialDescriptorLayout.get(); }

	/** Scene that holds render proxies; created in the constructor. */
	RenderScene* getRenderScene() { return m_renderScene.get(); }
	const RenderScene* getRenderScene() const { return m_renderScene.get(); }

private:
	void createDepthResources();
	void createFramebuffers();
	void createGlobalDescriptorLayout();
	void createMaterialDescriptorLayout();
	void createModelDescriptorLayout();
	void createPipelineLayout();
	void createPipeline();
	void createUniformBuffers();
	void createGlobalDescriptorPoolAndSets();

	VulkanContext& m_ctx;
	Window* m_window;
	AssetManager* m_assetManager;

	std::unique_ptr<VulkanSwapchain> m_swapchain;
	std::vector<VulkanImage> m_depthImages;
	std::unique_ptr<VulkanRenderPass> m_renderPass;
	std::vector<VulkanFramebuffer> m_framebuffers;

	ShaderId m_vertShaderId = InvalidShaderId;
	ShaderId m_fragShaderId = InvalidShaderId;

	std::unique_ptr<VulkanDescriptorSetLayout> m_descriptorLayout;
	std::unique_ptr<VulkanDescriptorSetLayout> m_materialDescriptorLayout;
	std::unique_ptr<VulkanDescriptorSetLayout> m_modelDescriptorLayout;
	std::unique_ptr<VulkanPipelineLayout> m_pipelineLayout;
	std::unique_ptr<VulkanPipeline> m_pipeline;

	std::unique_ptr<VulkanBuffer> m_cameraUBO;
	std::unique_ptr<VulkanBuffer> m_directionalLightUBO;
	std::unique_ptr<VulkanBuffer> m_modelUBO;

	std::unique_ptr<VulkanDescriptorPool> m_descriptorPool;
	std::vector<VulkanDescriptorSet> m_descriptorSets;
	std::vector<VulkanDescriptorSet> m_modelDescriptorSets;

	std::unique_ptr<VulkanFrameManager> m_frames;
	std::unique_ptr<RenderScene> m_renderScene;
};

} // namespace core
