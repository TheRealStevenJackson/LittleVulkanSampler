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

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <filesystem>

struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
};

static const Vertex cubeVertices[] = {
    {{-1, -1, -1}, {1,0,0}},
    {{ 1, -1, -1}, {0,1,0}},
    {{ 1,  1, -1}, {0,0,1}},
    {{-1,  1, -1}, {1,1,0}},
    {{-1, -1,  1}, {1,0,1}},
    {{ 1, -1,  1}, {0,1,1}},
    {{ 1,  1,  1}, {1,1,1}},
    {{-1,  1,  1}, {0,0,0}},
};

static const uint16_t cubeIndices[] = {
    0,1,2,2,3,0,
    4,5,6,6,7,4,
    0,4,7,7,3,0,
    1,5,6,6,2,1,
    3,2,6,6,7,3,
    0,1,5,5,4,0,
};

struct CameraUBO {
    glm::mat4 viewProj;
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
    // 3. Swapchain + RenderPass
    // -----------------------------------------
    VulkanSwapchain swapchain(ctx, window.getHandle());

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
        2,
        { {offsetof(Vertex, pos)},
        offsetof(Vertex, color) }
    );

    // -----------------------------------------
    // 8. Buffers (vertex, index, uniform)
    // -----------------------------------------
    VulkanBuffer vertexBuffer = VulkanBuffer(
        ctx,
        cubeVertices,
        sizeof(cubeVertices),
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT
    );

   VulkanBuffer indexBuffer = VulkanBuffer(
        ctx,
        cubeIndices,
        sizeof(cubeIndices),
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT
    );

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
    float angle = 0.0f;

    while (!window.shouldClose()) {
        window.pollEvents();

        clock.tick();
        float dt = clock.deltaTime();
        angle += dt * 1.0f;

        // Update UBO
        glm::mat4 model = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0, 1, 0));

        glm::mat4 view = glm::lookAt(
            glm::vec3(3, 3, 3),
            glm::vec3(0, 0, 0),
            glm::vec3(0, 1, 0)
        );

        glm::mat4 proj = glm::perspective(
            glm::radians(60.0f),
            swapchain.getAspectRatio(),
            0.1f, 100.f
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

        cmd.beginRenderPass(renderPass, framebuffers[imageIndex]);

        cmd.bindPipeline(pipeline);
        cmd.setViewport(swapchain.getExtent());
        cmd.setScissor(swapchain.getExtent());
        cmd.bindDescriptorSet(pipelineLayout, descriptorSets[imageIndex]);

        cmd.bindVertexBuffer(vertexBuffer);
        cmd.bindIndexBuffer(indexBuffer);

        cmd.drawIndexed(std::size(cubeIndices));

        cmd.endRenderPass();

        frames.endFrame(cmd.getHandle(), imageIndex);
    }

    vkDeviceWaitIdle(ctx.device());
    return 0;
}
