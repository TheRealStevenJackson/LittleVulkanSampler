#include <platform/Window.h>
#include <platform/Clock.h>
#include <platform/GamepadReader.h>
#include <core/event/InputManager.h>
#include <platform/graphics/vulkan/VulkanContext.h>
#include <platform/graphics/vulkan/VulkanSwapchain.h>
#include <platform/graphics/vulkan/VulkanRenderPass.h>
#include <platform/graphics/vulkan/VulkanImage.h>
#include <platform/graphics/vulkan/VulkanFramebuffer.h>
#include <platform/graphics/vulkan/VulkanShaderModule.h>
#include <platform/graphics/vulkan/VulkanDescriptorSetLayout.h>
#include <platform/graphics/vulkan/VulkanPipelineLayout.h>
#include <platform/graphics/vulkan/VulkanPipeline.h>
#include <platform/graphics/vulkan/VulkanBuffer.h>
#include <platform/graphics/vulkan/VulkanDescriptorPool.h>
#include <platform/graphics/vulkan/VulkanDescriptorSet.h>
#include <platform/graphics/vulkan/VulkanFrameManager.h>
#include <core/asset/AssetManager.h>
#include <core/asset/Material.h>
#include <game/Entity.h>
#include <engine/input/Controller.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vulkan/vulkan.h>
#include <iostream>
#include <filesystem>

// Vertex structure matching the loaded mesh format (pos, normal, texCoord)
struct Vertex {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 texCoord;
    glm::vec3 tangent;
	glm::vec3 bitangent;
};

struct CameraUBO {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

struct DirectionalLightUBO {
    glm::vec4 direction;  // xyz = direction toward light, w = unused
    glm::vec4 color;      // rgb = light color, w = unused
};

int main() {
    // -----------------------------------------
    // 1. Window
    // -----------------------------------------

    core::InputManager inputManager;
    Window window({ "Spinning Cube Sample", 1440, 1440 });
    window.setInputReceiver(&inputManager);

    GamepadReader gamepadReader;
    gamepadReader.setInputReceiver(&inputManager);

    engine::Controller controller(inputManager);

    // -----------------------------------------
    // 2. Vulkan context
    // -----------------------------------------
    VulkanContext ctx(window.getHandle());

    // -----------------------------------------
    // 2.5. Asset manager (OBJ + MTL loading)
    // -----------------------------------------
    //TODO: Move to SceneLoader
    AssetManager assetManager(ctx);

    std::vector<std::string> pathsToTry = {
        "resources/meshes/nes-controller/controller_wireless_1024.obj",  // Relative to executable
        "../../resources/meshes/nes-controller/controller_wireless_1024.obj",  // Relative from build subdirectory
        "../../../resources/meshes/nes-controller/controller_wireless_1024.obj"  // Relative from deeper build subdirectory
    };

    std::vector<MeshId> meshIds;
    std::string loadedPath;
    if (!assetManager.loadObjFromPaths(pathsToTry, meshIds, loadedPath)) {
        std::cerr << "Failed to load mesh. Tried paths:" << std::endl;
        for (const auto& path : pathsToTry) {
            std::cerr << "  - " << path << std::endl;
        }
        return -1;
    }
    std::cout << "Successfully loaded " << meshIds.size() << " mesh(es) from " << loadedPath << std::endl;

    // Load materials from the OBJ file's MTL using StbLoader
    std::vector<MaterialId> materialIds = assetManager.loadMaterials(loadedPath);
    std::cout << "Loaded " << materialIds.size() << " material(s)" << std::endl;

    Entity entity;
    entity.renderComponent().meshIds = meshIds;
    entity.renderComponent().materialIds = materialIds;
    entity.setController(&controller);

    // -----------------------------------------
    // 3. Swapchain + RenderPass
    // -----------------------------------------
    VulkanSwapchain swapchain(ctx, window.getHandle());

    // TODO: Move depth image and view to swapchain
    std::vector<VulkanImage> depthImages;
    depthImages.reserve(swapchain.imageCount());
    for (uint32_t i = 0; i < swapchain.imageCount(); i++) {
        depthImages.emplace_back(
            ctx,
            VK_FORMAT_D32_SFLOAT,
            swapchain.getExtent()
        );
    }

    VulkanRenderPass renderPass(
        ctx,
        swapchain.getImageFormat(),
        depthImages[0].format()
    );

    // -----------------------------------------
    // 4. Framebuffers
    // -----------------------------------------
    // TODO: Move framebuffer to swapchain
    std::vector<VulkanFramebuffer> framebuffers;
    framebuffers.reserve(swapchain.imageCount());
    for (uint32_t i = 0; i < swapchain.imageCount(); i++) {
        framebuffers.emplace_back(
            ctx,
            renderPass.renderPass(),
            swapchain.getImageViews()[i],
            depthImages[i].imageView(),
            swapchain.getExtent()
        );
    }

    // -----------------------------------------
    // 5. Load shaders
    // -----------------------------------------
    VulkanShaderModule vert = VulkanShaderModule(ctx, "spv/spinning_cube.vert.spv");
    VulkanShaderModule frag = VulkanShaderModule(ctx, "spv/spinning_cube.frag.spv");

    // -----------------------------------------
    // 6. Pipeline layout + descriptor set layout
    // -----------------------------------------
    // Set 0: Global uniforms (camera, light)
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

    VulkanDescriptorSetLayout descriptorLayout(ctx, layoutBindings);

    // Set 1: PBR Material textures and UBO
    std::vector<VkDescriptorSetLayoutBinding> materialLayoutBindings(6);
    // Albedo map
    materialLayoutBindings[0].binding = 0;
    materialLayoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    materialLayoutBindings[0].descriptorCount = 1;
    materialLayoutBindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    materialLayoutBindings[0].pImmutableSamplers = nullptr;
    // Normal map
    materialLayoutBindings[1].binding = 1;
    materialLayoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    materialLayoutBindings[1].descriptorCount = 1;
    materialLayoutBindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    materialLayoutBindings[1].pImmutableSamplers = nullptr;
    // Metallic map
    materialLayoutBindings[2].binding = 2;
    materialLayoutBindings[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    materialLayoutBindings[2].descriptorCount = 1;
    materialLayoutBindings[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    materialLayoutBindings[2].pImmutableSamplers = nullptr;
    // Roughness map
    materialLayoutBindings[3].binding = 3;
    materialLayoutBindings[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    materialLayoutBindings[3].descriptorCount = 1;
    materialLayoutBindings[3].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    materialLayoutBindings[3].pImmutableSamplers = nullptr;
    // AO map
    materialLayoutBindings[4].binding = 4;
    materialLayoutBindings[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    materialLayoutBindings[4].descriptorCount = 1;
    materialLayoutBindings[4].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    materialLayoutBindings[4].pImmutableSamplers = nullptr;
    // Material UBO
    materialLayoutBindings[5].binding = 5;
    materialLayoutBindings[5].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    materialLayoutBindings[5].descriptorCount = 1;
    materialLayoutBindings[5].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    materialLayoutBindings[5].pImmutableSamplers = nullptr;

    VulkanDescriptorSetLayout materialDescriptorLayout(ctx, materialLayoutBindings);

    std::vector<VulkanDescriptorSetLayout*> pipelineLayouts = { &descriptorLayout, &materialDescriptorLayout };
    VulkanPipelineLayout pipelineLayout = VulkanPipelineLayout(ctx, pipelineLayouts);

    // -----------------------------------------
    // 7. Pipeline
    // -----------------------------------------
    VulkanPipeline pipeline = VulkanPipeline(
        ctx,
        renderPass,
        pipelineLayout,
        vert,
        frag,
        sizeof(Vertex),
        5,
        { offsetof(Vertex, pos),
          offsetof(Vertex, normal),
          offsetof(Vertex, texCoord),
          offsetof(Vertex, tangent),
          offsetof(Vertex, bitangent) }
    );

    // -----------------------------------------
    // 8. Uniform buffer
    // -----------------------------------------
    CameraUBO initUBO{};
    initUBO.model = glm::mat4(1.0f);
    initUBO.view = glm::mat4(1.0f);
    initUBO.proj = glm::mat4(1.0f);
    VulkanBuffer cameraUBO = VulkanBuffer(
        ctx,
        &initUBO,
        sizeof(CameraUBO),
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT
    );

    std::cout << "cameraUBO successfully created" << std::endl;

    DirectionalLightUBO initLight{};
    initLight.direction = glm::vec4(0.0f, -1.0f, 0.0f, 0.0f);
    initLight.color = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
    VulkanBuffer directionalLightUBO = VulkanBuffer(
        ctx,
        &initLight,
        sizeof(DirectionalLightUBO),
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT
    );

    // -----------------------------------------
    // 9. Descriptor set
    // -----------------------------------------
    VulkanDescriptorPool descriptorPool(
        ctx,
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        2u * swapchain.imageCount()
    );

    std::vector<VulkanDescriptorSet> descriptorSets;
    descriptorSets.reserve(swapchain.imageCount());
    for (uint32_t i = 0; i < swapchain.imageCount(); i++) {
        descriptorSets.emplace_back(
            ctx,
            descriptorPool,
            descriptorLayout
        );
        descriptorSets.back().writeUniformBuffer(cameraUBO, sizeof(CameraUBO), 0);
        descriptorSets.back().writeUniformBuffer(directionalLightUBO, sizeof(DirectionalLightUBO), 1);
    }

    // Create default sampler for textures
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy = 1.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    VkSampler defaultSampler = VK_NULL_HANDLE;
    if (vkCreateSampler(ctx.device(), &samplerInfo, nullptr, &defaultSampler) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create default sampler.");
    }

    // Create material descriptor pool
    std::vector<VkDescriptorPoolSize> materialPoolSizes(2);
    materialPoolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    materialPoolSizes[0].descriptorCount = 5u * swapchain.imageCount(); // 5 textures per material
    materialPoolSizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    materialPoolSizes[1].descriptorCount = 1u * swapchain.imageCount(); // 1 UBO per material

    VulkanDescriptorPool materialDescriptorPool(ctx, materialPoolSizes, swapchain.imageCount());

    // Get the first material (assuming we have at least one)
    Material* material = nullptr;
    if (!materialIds.empty()) {
        material = assetManager.getMaterial(materialIds[0]);
    }

    // Create material descriptor sets
    std::vector<VulkanDescriptorSet> materialDescriptorSets;
    materialDescriptorSets.reserve(swapchain.imageCount());
    for (uint32_t i = 0; i < swapchain.imageCount(); i++) {
        materialDescriptorSets.emplace_back(
            ctx,
            materialDescriptorPool,
            materialDescriptorLayout
        );

        if (material) {
            // Write textures (use default/placeholder if not available)
            if (material->hasAlbedoMap() && material->albedoMap()) {
                materialDescriptorSets.back().writeCombinedImageSampler(*material->albedoMap(), defaultSampler, 0);
            }
            if (material->hasNormalMap() && material->normalMap()) {
                materialDescriptorSets.back().writeCombinedImageSampler(*material->normalMap(), defaultSampler, 1);
            }
            if (material->hasMetallicMap() && material->metallicMap()) {
                materialDescriptorSets.back().writeCombinedImageSampler(*material->metallicMap(), defaultSampler, 2);
            }
            if (material->hasRoughnessMap() && material->roughnessMap()) {
                materialDescriptorSets.back().writeCombinedImageSampler(*material->roughnessMap(), defaultSampler, 3);
            }
            if (material->hasAoMap() && material->aoMap()) {
                materialDescriptorSets.back().writeCombinedImageSampler(*material->aoMap(), defaultSampler, 4);
            }
            // Write material UBO
            materialDescriptorSets.back().writeUniformBuffer(material->materialUBO(), sizeof(MaterialUBO), 5);
        }
    }

    // -----------------------------------------
    // 10. Frame manager (command buffers + sync)
    // -----------------------------------------
    VulkanFrameManager frames(ctx, swapchain.imageCount(), swapchain);

    // -----------------------------------------
    // Main loop
    // -----------------------------------------
    Clock clock = Clock();

    while (!window.shouldClose()) {
        window.pollEvents();
        gamepadReader.poll();
        inputManager.update();

        clock.tick();
        float dt = clock.deltaTime();

        // Update entity (updates model matrix based on controller input)
        entity.update(dt);

        glm::mat4 view = glm::lookAt(
            glm::vec3(0.1, 0.1, 0.1),
            glm::vec3(0, 0, 0),
            glm::vec3(0, 1, 0)
        );

        glm::mat4 proj = glm::perspective(
            glm::radians(60.0f),
            swapchain.getAspectRatio(),
            0.01f, 200.0f
        );
        proj[1][1] *= -1;

        CameraUBO u;
        u.model = entity.model();
        u.view = view;
        u.proj = proj;
        cameraUBO.upload(&u, sizeof(u));

        DirectionalLightUBO light{};
        light.direction = glm::vec4(0.0f, -1.0f, -0.3f, 0.0f);
        light.color = glm::vec4(0.8f, 0.8f, 0.75f, 0.0f); // Reduced brightness from 1.0 to 0.8
        directionalLightUBO.upload(&light, sizeof(light));

        // Acquire frame
        uint32_t imageIndex = frames.beginFrame();

        VulkanCommandBuffer cmd = VulkanCommandBuffer(frames.getCommandBuffer());
        cmd.begin();

        // Begin render pass

        cmd.beginRenderPass(renderPass, framebuffers[imageIndex], swapchain.getExtent());

        cmd.bindPipeline(pipeline);
        cmd.setViewport(swapchain.getExtent());
        cmd.setScissor(swapchain.getExtent());
        std::vector<VulkanDescriptorSet*> descriptorSetsToBind = { &descriptorSets[imageIndex], &materialDescriptorSets[imageIndex] };
        cmd.bindDescriptorSets(pipelineLayout, descriptorSetsToBind);

        // Use the meshes from the entity's RenderComponent
        for (MeshId meshId : entity.renderComponent().meshIds) {
            const Mesh* mesh = assetManager.getMesh(meshId);
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

        frames.endFrame(cmd.getHandle(), imageIndex);
    }

    vkDeviceWaitIdle(ctx.device());
    
    // Cleanup sampler
    if (defaultSampler != VK_NULL_HANDLE) {
        vkDestroySampler(ctx.device(), defaultSampler, nullptr);
    }
    
    return 0;
}
