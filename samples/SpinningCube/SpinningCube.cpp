#include <platform/Window.h>
#include <platform/Clock.h>
#include <engine/graphics/renderer/VulkanContext.h>
#include <engine/graphics/renderer/VulkanSwapchain.h>
#include <engine/graphics/renderer/VulkanRenderPass.h>
#include <engine/graphics/renderer/VulkanImage.h>
#include <engine/graphics/renderer/VulkanFramebuffer.h>
#include <engine/graphics/renderer/VulkanShaderModule.h>
#include <engine/graphics/renderer/VulkanDescriptorSetLayout.h>
#include <engine/graphics/renderer/VulkanPipelineLayout.h>
#include <engine/graphics/renderer/VulkanPipeline.h>
#include <engine/graphics/renderer/VulkanBuffer.h>
#include <engine/graphics/renderer/VulkanDescriptorPool.h>
#include <engine/graphics/renderer/VulkanDescriptorSet.h>
#include <engine/graphics/renderer/VulkanFrameManager.h>
#include <engine/assets/AssetManager.h>
#include <game/Entity.h>

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

    Window window({ "Spinning Cube Sample", 720, 720 });

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

    // Extract material paths from the MTL file
    std::vector<MaterialPaths> materialPaths = assetManager.extractMaterialPaths(loadedPath);
    std::cout << "Extracted " << materialPaths.size() << " material path(s) from MTL" << std::endl;

    // Load materials using StbLoader
    std::vector<MaterialId> materialIds = assetManager.loadMaterials(materialPaths);
    std::cout << "Loaded " << materialIds.size() << " material(s)" << std::endl;

    Entity entity;
    entity.renderComponent().meshIds = meshIds;
    entity.renderComponent().materialIds = materialIds;

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

    VulkanPipelineLayout pipelineLayout = VulkanPipelineLayout(ctx, descriptorLayout);

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
        3,
        { offsetof(Vertex, pos),
          offsetof(Vertex, normal),
          offsetof(Vertex, texCoord) }
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

    // -----------------------------------------
    // 10. Frame manager (command buffers + sync)
    // -----------------------------------------
    VulkanFrameManager frames(ctx, swapchain.imageCount(), swapchain);

    // -----------------------------------------
    // Main loop
    // -----------------------------------------
    Clock clock = Clock();
    float angle = 0.0f;

    while (!window.shouldClose()) {
        window.pollEvents();

        clock.tick();
        float dt = clock.deltaTime();
        angle += dt * 1.0f;

        // Update UBO
        glm::mat4 model = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0, 1, 0));

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
        u.model = model;
        u.view = view;
        u.proj = proj;
        cameraUBO.upload(&u, sizeof(u));

        DirectionalLightUBO light{};
        light.direction = glm::vec4(0.0f, -1.0f, -0.3f, 0.0f);
        light.color = glm::vec4(1.0f, 1.0f, 0.95f, 0.0f);
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
        cmd.bindDescriptorSet(pipelineLayout, descriptorSets[imageIndex]);

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
    return 0;
}
