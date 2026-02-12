#include "engine/Engine.h"
#include <core/renderer/Renderer.h>
#include <core/renderer/RenderTypes.h>
#include <platform/graphics/vulkan/VulkanContext.h>
#include <platform/graphics/vulkan/VulkanSwapchain.h>
#include <platform/graphics/vulkan/VulkanRenderPass.h>
#include <platform/graphics/vulkan/VulkanFramebuffer.h>
#include <platform/graphics/vulkan/VulkanPipeline.h>
#include <platform/graphics/vulkan/VulkanPipelineLayout.h>
#include <platform/graphics/vulkan/VulkanBuffer.h>
#include <platform/graphics/vulkan/VulkanDescriptorSet.h>
#include <platform/graphics/vulkan/VulkanFrameManager.h>
#include <platform/graphics/vulkan/VulkanCommandBuffer.h>
#include <core/asset/AssetManager.h>
#include <core/asset/types/Material.h>
#include <core/asset/types/Mesh.h>
#include <engine/scene/SceneManager.h>
#include <engine/scene/object/Entity.h>
#include <engine/scene/object/Light.h>
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

namespace engine {

Engine::Engine(const WindowDesc& desc)
	: m_window(desc)
	, m_inputManager()
	, m_gamepadReader()
	, m_controller(m_inputManager)
	, m_context(std::make_unique<VulkanContext>(m_window.getHandle()))
	, m_assetManager(std::make_unique<AssetManager>(*m_context))
	, m_sceneManager(*m_assetManager)
{
	m_window.setInputReceiver(&m_inputManager);
	m_gamepadReader.setInputReceiver(&m_inputManager);
	m_renderer = std::make_unique<core::Renderer>(*m_context, m_window, *m_assetManager);
	m_sceneManager.setRenderScene(m_renderer->getRenderScene());
}

Engine::~Engine() = default;

void Engine::pollEvents() {
	m_window.pollEvents();
}

bool Engine::shouldClose() const {
	return m_window.shouldClose();
}

VulkanContext& Engine::context() {
	return *m_context;
}

const VulkanContext& Engine::context() const {
	return *m_context;
}

AssetManager& Engine::assetManager() {
	return *m_assetManager;
}

const AssetManager& Engine::assetManager() const {
	return *m_assetManager;
}

void Engine::run() {
	while (!shouldClose()) {
		pollEvents();
		pollGamepads();
		m_inputManager.update();
		m_clock.tick();
		renderFrame(static_cast<float>(m_clock.deltaTime()));
	}
	vkDeviceWaitIdle(context().device());
}

void Engine::renderFrame(float dt) {
	renderFrame(dt, m_renderer->getRenderFrameParams());
}

void Engine::renderFrame(float dt, const RenderFrameParams& params) {
	if (!params.swapchain || !params.renderPass || !params.framebuffers || !params.pipeline ||
	    !params.pipelineLayout || !params.descriptorSets || !params.modelDescriptorSets ||
	    !params.cameraUBO || !params.modelUBO || !params.directionalLightUBO || !params.frames)
		return;

	sceneManager().update(dt);

	Entity* entity = sceneManager().loadedEntity();
	if (!entity)
		return;

	Camera* camera = sceneManager().loadedCamera();
	if (!camera)
		return;

	Light* light = sceneManager().loadedLight();
	if (!light)
		return;

	core::ViewProjUBO viewProj;
	viewProj.view = camera->viewMatrix();
	viewProj.proj = camera->projectionMatrix();
	params.cameraUBO->upload(&viewProj, sizeof(viewProj));

	core::ModelUBO modelUbo;
	modelUbo.model = entity->model();
	params.modelUBO->upload(&modelUbo, sizeof(modelUbo));

	core::DirectionalLightUBO lightUbo{};
	lightUbo.direction = glm::vec4(light->direction(), 0.0f);
	lightUbo.color = glm::vec4(light->color(), 1.0f);
	params.directionalLightUBO->upload(&lightUbo, sizeof(lightUbo));

	uint32_t imageIndex = params.frames->beginFrame();

	VulkanCommandBuffer cmd(params.frames->getCommandBuffer());
	cmd.begin();

	cmd.beginRenderPass(*params.renderPass, (*params.framebuffers)[imageIndex], params.swapchain->getExtent());

	cmd.bindPipeline(*params.pipeline);
	cmd.setViewport(params.swapchain->getExtent());
	cmd.setScissor(params.swapchain->getExtent());

	const auto& matIds = entity->renderComponent().materialIds;
	Material* material = (!matIds.empty()) ? assetManager().getMaterial(matIds[0]) : nullptr;
	if (material && material->descriptorSet()) {
		std::vector<VulkanDescriptorSet*> descriptorSetsToBind = {
			&(*params.descriptorSets)[imageIndex],
			material->descriptorSet(),
			&(*params.modelDescriptorSets)[imageIndex]
		};
		cmd.bindDescriptorSets(*params.pipelineLayout, descriptorSetsToBind);
	}

	for (MeshId meshId : entity->renderComponent().meshIds) {
		const Mesh* mesh = assetManager().getMesh(meshId);
		if (mesh) {
			mesh->bindVertexBuffer(cmd.getHandle());
			if (mesh->hasIndices()) {
				mesh->bindIndexBuffer(cmd.getHandle());
				mesh->draw(cmd.getHandle());
			} else {
				mesh->draw(cmd.getHandle());
			}
		}
	}

	cmd.endRenderPass();

	params.frames->endFrame(cmd.getHandle(), imageIndex);
}

bool Engine::loadEntityTemporary(const std::vector<std::string>& pathsToTry) {
	return m_sceneManager.loadEntityTemporary(pathsToTry, &m_controller,
		m_renderer ? m_renderer->getMaterialDescriptorLayout() : nullptr);
}

} // namespace engine
