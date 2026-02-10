#include "core/renderer/Renderer.h"
#include <platform/graphics/vulkan/VulkanShaderModule.h>
#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>
#include <iostream>

namespace core {

namespace {

// Vertex layout matching loaded mesh (pos, normal, texCoord, tangent, bitangent)
struct Vertex {
    float pos[3];
    float normal[3];
    float texCoord[2];
    float tangent[3];
    float bitangent[3];
};

struct CameraUBO {
    float model[16];
    float view[16];
    float proj[16];
};

struct DirectionalLightUBO {
    float direction[4];
    float color[4];
};

} // namespace

Renderer::Renderer(VulkanContext& ctx, Window& window, AssetManager& assetManager)
    : m_ctx(ctx)
    , m_window(&window)
    , m_assetManager(&assetManager)
{
    m_swapchain = std::make_unique<VulkanSwapchain>(ctx, window.getHandle());
    createDepthResources();
    m_renderPass = std::make_unique<VulkanRenderPass>(
        ctx,
        m_swapchain->getImageFormat(),
        m_depthImages[0].format()
    );
    createFramebuffers();

    m_vertShaderId = assetManager.loadShader("spv/spinning_cube.vert.spv");
    m_fragShaderId = assetManager.loadShader("spv/spinning_cube.frag.spv");
    if (m_vertShaderId == InvalidShaderId || m_fragShaderId == InvalidShaderId) {
        throw std::runtime_error("Renderer: failed to load spinning_cube shaders.");
    }

    createGlobalDescriptorLayout();
    createMaterialDescriptorLayout();
    createPipelineLayout();
    createPipeline();
    createUniformBuffers();
    createGlobalDescriptorPoolAndSets();

    m_frames = std::make_unique<VulkanFrameManager>(ctx, m_swapchain->imageCount(), *m_swapchain);

    std::cout << "Renderer initialized." << std::endl;
}

Renderer::~Renderer() = default;

void Renderer::createDepthResources() {
    m_depthImages.clear();
    m_depthImages.reserve(m_swapchain->imageCount());
    for (uint32_t i = 0; i < m_swapchain->imageCount(); i++) {
        m_depthImages.emplace_back(m_ctx, VK_FORMAT_D32_SFLOAT, m_swapchain->getExtent());
    }
}

void Renderer::createFramebuffers() {
    m_framebuffers.clear();
    m_framebuffers.reserve(m_swapchain->imageCount());
    for (uint32_t i = 0; i < m_swapchain->imageCount(); i++) {
        m_framebuffers.emplace_back(
            m_ctx,
            m_renderPass->renderPass(),
            m_swapchain->getImageViews()[i],
            m_depthImages[i].imageView(),
            m_swapchain->getExtent()
        );
    }
}

void Renderer::createGlobalDescriptorLayout() {
    std::vector<VkDescriptorSetLayoutBinding> layoutBindings(2);
    layoutBindings[0].binding = 0;
    layoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    layoutBindings[0].descriptorCount = 1;
    layoutBindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    layoutBindings[0].pImmutableSamplers = nullptr;
    layoutBindings[1].binding = 1;
    layoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    layoutBindings[1].descriptorCount = 1;
    layoutBindings[1].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    layoutBindings[1].pImmutableSamplers = nullptr;

    m_descriptorLayout = std::make_unique<VulkanDescriptorSetLayout>(m_ctx, layoutBindings);
}

void Renderer::createMaterialDescriptorLayout() {
    std::vector<VkDescriptorSetLayoutBinding> materialLayoutBindings(6);
    materialLayoutBindings[0].binding = 0;
    materialLayoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    materialLayoutBindings[0].descriptorCount = 1;
    materialLayoutBindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    materialLayoutBindings[0].pImmutableSamplers = nullptr;
    materialLayoutBindings[1].binding = 1;
    materialLayoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    materialLayoutBindings[1].descriptorCount = 1;
    materialLayoutBindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    materialLayoutBindings[1].pImmutableSamplers = nullptr;
    materialLayoutBindings[2].binding = 2;
    materialLayoutBindings[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    materialLayoutBindings[2].descriptorCount = 1;
    materialLayoutBindings[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    materialLayoutBindings[2].pImmutableSamplers = nullptr;
    materialLayoutBindings[3].binding = 3;
    materialLayoutBindings[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    materialLayoutBindings[3].descriptorCount = 1;
    materialLayoutBindings[3].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    materialLayoutBindings[3].pImmutableSamplers = nullptr;
    materialLayoutBindings[4].binding = 4;
    materialLayoutBindings[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    materialLayoutBindings[4].descriptorCount = 1;
    materialLayoutBindings[4].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    materialLayoutBindings[4].pImmutableSamplers = nullptr;
    materialLayoutBindings[5].binding = 5;
    materialLayoutBindings[5].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    materialLayoutBindings[5].descriptorCount = 1;
    materialLayoutBindings[5].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    materialLayoutBindings[5].pImmutableSamplers = nullptr;

    m_materialDescriptorLayout = std::make_unique<VulkanDescriptorSetLayout>(m_ctx, materialLayoutBindings);
}

void Renderer::createPipelineLayout() {
    std::vector<VulkanDescriptorSetLayout*> layouts = { m_descriptorLayout.get(), m_materialDescriptorLayout.get() };
    m_pipelineLayout = std::make_unique<VulkanPipelineLayout>(m_ctx, layouts);
}

void Renderer::createPipeline() {
    Shader* vertShader = m_assetManager->getShader(m_vertShaderId);
    Shader* fragShader = m_assetManager->getShader(m_fragShaderId);
    if (!vertShader || !fragShader || !vertShader->module() || !fragShader->module()) {
        throw std::runtime_error("Renderer: shader or module missing.");
    }

    m_pipeline = std::make_unique<VulkanPipeline>(
        m_ctx,
        *m_renderPass,
        *m_pipelineLayout,
        *vertShader->module(),
        *fragShader->module(),
        sizeof(Vertex),
        5,
        std::vector<uint32_t>{
            offsetof(Vertex, pos),
            offsetof(Vertex, normal),
            offsetof(Vertex, texCoord),
            offsetof(Vertex, tangent),
            offsetof(Vertex, bitangent)
        }
    );
}

void Renderer::createUniformBuffers() {
    CameraUBO initUBO{};
    for (int i = 0; i < 16; i++) {
        initUBO.model[i] = (i % 5 == 0) ? 1.0f : 0.0f;
        initUBO.view[i] = (i % 5 == 0) ? 1.0f : 0.0f;
        initUBO.proj[i] = (i % 5 == 0) ? 1.0f : 0.0f;
    }
    m_cameraUBO = std::make_unique<VulkanBuffer>(
        m_ctx,
        &initUBO,
        sizeof(CameraUBO),
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT
    );

    DirectionalLightUBO initLight{};
    initLight.direction[0] = 0.0f;
    initLight.direction[1] = -1.0f;
    initLight.direction[2] = 0.0f;
    initLight.direction[3] = 0.0f;
    initLight.color[0] = 1.0f;
    initLight.color[1] = 1.0f;
    initLight.color[2] = 1.0f;
    initLight.color[3] = 0.0f;
    m_directionalLightUBO = std::make_unique<VulkanBuffer>(
        m_ctx,
        &initLight,
        sizeof(DirectionalLightUBO),
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT
    );
}

void Renderer::createGlobalDescriptorPoolAndSets() {
    m_descriptorPool = std::make_unique<VulkanDescriptorPool>(
        m_ctx,
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        2u * m_swapchain->imageCount()
    );

    m_descriptorSets.clear();
    m_descriptorSets.reserve(m_swapchain->imageCount());
    for (uint32_t i = 0; i < m_swapchain->imageCount(); i++) {
        m_descriptorSets.emplace_back(m_ctx, *m_descriptorPool, *m_descriptorLayout);
        m_descriptorSets.back().writeUniformBuffer(*m_cameraUBO, sizeof(CameraUBO), 0);
        m_descriptorSets.back().writeUniformBuffer(*m_directionalLightUBO, sizeof(DirectionalLightUBO), 1);
    }
}

engine::Engine::RenderFrameParams Renderer::getRenderFrameParams() {
    engine::Engine::RenderFrameParams params;
    params.swapchain = m_swapchain.get();
    params.renderPass = m_renderPass.get();
    params.framebuffers = &m_framebuffers;
    params.pipeline = m_pipeline.get();
    params.pipelineLayout = m_pipelineLayout.get();
    params.descriptorSets = &m_descriptorSets;
    params.cameraUBO = m_cameraUBO.get();
    params.directionalLightUBO = m_directionalLightUBO.get();
    params.frames = m_frames.get();
    return params;
}

} // namespace core
