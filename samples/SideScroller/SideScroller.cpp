#include <platform/Window.h>
#include <platform/Clock.h>
#include <vulkanbackend/VulkanContext.h>
#include <VulkanBackend/VulkanSwapchain.h>
#include <VulkanBackend/VulkanRenderPass.h>
#include <vulkanbackend/VulkanImage.h>
#include <vulkanbackend/VulkanFramebuffer.h>
#include <vulkanbackend/VulkanShaderModule.h>
#include <vulkanbackend/VulkanDescriptorSetLayout.h>
#include <vulkanbackend/VulkanPipelineLayout.h>
#include <vulkanbackend/VulkanPipeline.h>
#include <vulkanbackend/VulkanBuffer.h>
#include <vulkanbackend/VulkanDescriptorPool.h>
#include <vulkanbackend/VulkanDescriptorSet.h>
#include <vulkanbackend/VulkanFrameManager.h>
#include <assets/ObjLoader.h>
#include <renderer/Mesh.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <filesystem>

// Vertex structure matching the loaded mesh format (pos, normal, texCoord)
struct Vertex {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 texCoord;
};

struct CameraUBO {
    glm::mat4 viewProj;
};

int main() {
    // -----------------------------------------
    // 1. Window
    // -----------------------------------------

    Window window({ "Side Scroller Sample", 1280, 720 });

    // -----------------------------------------
    // 2. Vulkan context
    // -----------------------------------------
    VulkanContext ctx(window.getHandle());

    // -----------------------------------------
    // 2.5. Object loader
    // -----------------------------------------
    ObjLoader objLoader(ctx);
    
    // Load mesh from resources directory - try multiple paths
    std::vector<std::string> pathsToTry = {
        "resources/Meshes/nes-controller/controller_wireless_1024.obj",  // Relative to executable
        "../../resources/Meshes/nes-controller/controller_wireless_1024.obj",  // Relative from build subdirectory
        "../../../resources/Meshes/nes-controller/controller_wireless_1024.obj"  // Relative from deeper build subdirectory
    };
    
    std::vector<std::unique_ptr<Mesh>> meshes;
    std::string loadedPath;
    for (const auto& path : pathsToTry) {
        meshes = objLoader.loadFromFile(path);
        if (!meshes.empty()) {
            loadedPath = path;
            break;
        }
    }
    
    if (meshes.empty()) {
        std::cerr << "Failed to load mesh. Tried paths:" << std::endl;
        for (const auto& path : pathsToTry) {
            std::cerr << "  - " << path << std::endl;
        }
        return -1;
    }
    std::cout << "Successfully loaded " << meshes.size() << " mesh(es) from " << loadedPath << std::endl;
    
    // Use the first loaded mesh
    auto& loadedMesh = meshes[0];

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
    VulkanShaderModule vert = VulkanShaderModule(ctx, "spv/side_scroller.vert.spv");
    VulkanShaderModule frag = VulkanShaderModule(ctx, "spv/side_scroller.frag.spv");

    // -----------------------------------------
    // 6. Pipeline layout + descriptor set layout
    // -----------------------------------------
    VulkanDescriptorSetLayout descriptorLayout = VulkanDescriptorSetLayout(
        ctx,
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        VK_SHADER_STAGE_VERTEX_BIT
    );

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
    glm::mat4 model = glm::mat4(1.0f);   // Identity
    VulkanBuffer cameraUBO = VulkanBuffer(
        ctx,
        glm::value_ptr(model),
        sizeof(CameraUBO),
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT
    );

    std::cout << "cameraUBO successfully created" << std::endl;

    // -----------------------------------------
    // 9. Descriptor set
    // -----------------------------------------
    VulkanDescriptorPool descriptorPool = VulkanDescriptorPool(
        ctx,
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
    );

    std::vector<VulkanDescriptorSet> descriptorSets;
    descriptorSets.reserve(swapchain.imageCount());
    for (uint32_t i = 0; i < swapchain.imageCount(); i++) {
        descriptorSets.emplace_back(
            ctx,
            descriptorPool,
            descriptorLayout
        );
        descriptorSets.back().writeUniformBuffer(cameraUBO, sizeof(CameraUBO));
    }

    // -----------------------------------------
    // 10. Frame manager (command buffers + sync)
    // -----------------------------------------
    VulkanFrameManager frames(ctx, swapchain.imageCount(), swapchain);

    // -----------------------------------------
    // Main loop
    // -----------------------------------------
    Clock clock = Clock();
    float cameraX = 0.0f;  // Camera position for side-scrolling

    while (!window.shouldClose()) {
        window.pollEvents();

        clock.tick();
        float dt = clock.deltaTime();
        cameraX += dt * 2.0f;  // Scroll speed

        // Update UBO - side-scrolling camera
        glm::mat4 model = glm::mat4(1.0f);

        // Orthographic projection for 2D side-scrolling
        glm::mat4 view = glm::lookAt(
            glm::vec3(cameraX, 0.0f, 1.0f),  // Camera follows X position
            glm::vec3(cameraX, 0.0f, 0.0f),  // Look at same X position
            glm::vec3(0, 1, 0)
        );

        // Orthographic projection for 2D
        float aspect = swapchain.getAspectRatio();
        float viewHeight = 10.0f;
        float viewWidth = viewHeight * aspect;
        glm::mat4 proj = glm::ortho(
            -viewWidth / 2.0f, viewWidth / 2.0f,
            -viewHeight / 2.0f, viewHeight / 2.0f,
            0.01f, 100.0f
        );
        proj[1][1] *= -1;

        CameraUBO u;
        u.viewProj = proj * view * model;
        cameraUBO.upload(&u, sizeof(u));

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

        // Use the loaded mesh instead of hardcoded buffers
        loadedMesh->bindVertexBuffer(cmd.getHandle());
        if (loadedMesh->hasIndices()) {
            loadedMesh->bindIndexBuffer(cmd.getHandle());
            loadedMesh->draw(cmd.getHandle());
        } else {
            loadedMesh->draw(cmd.getHandle());
        }

        cmd.endRenderPass();

        frames.endFrame(cmd.getHandle(), imageIndex);
    }

    vkDeviceWaitIdle(ctx.device());
    return 0;
}
