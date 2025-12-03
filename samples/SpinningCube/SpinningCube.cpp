#include <platform/Window.h>
#include <vulkanbackend/VulkanContext.h>
#include <VulkanBackend/VulkanSwapchain.h>
#include <VulkanBackend/VulkanRenderPass.h>
#include <vulkanbackend/VulkanImage.h>
#include <vulkanbackend/VulkanFramebuffer.h>
#include <vulkanbackend/VulkanShaderModule.h>
//#include <VulkanBackend/Pipeline.h>
//#include <VulkanBackend/DescriptorSet.h>
//#include <VulkanBackend/Buffer.h>
//#include <VulkanBackend/Commands.h>
//#include <VulkanBackend/Framebuffer.h>
//#include <VulkanBackend/FrameManager.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <filesystem>

//struct Vertex {
//    glm::vec3 pos;
//    glm::vec3 color;
//};
//
//static const Vertex cubeVertices[] = {
//    {{-1, -1, -1}, {1,0,0}},
//    {{ 1, -1, -1}, {0,1,0}},
//    {{ 1,  1, -1}, {0,0,1}},
//    {{-1,  1, -1}, {1,1,0}},
//    {{-1, -1,  1}, {1,0,1}},
//    {{ 1, -1,  1}, {0,1,1}},
//    {{ 1,  1,  1}, {1,1,1}},
//    {{-1,  1,  1}, {0,0,0}},
//};
//
//static const uint16_t cubeIndices[] = {
//    0,1,2,2,3,0,
//    4,5,6,6,7,4,
//    0,4,7,7,3,0,
//    1,5,6,6,2,1,
//    3,2,6,6,7,3,
//    0,1,5,5,4,0,
//};
//
//struct CameraUBO {
//    glm::mat4 viewProj;
//};

int main() {
    // -----------------------------------------
    // 1. Window
    // -----------------------------------------

    Window window({"Spinning Cube Sample", 1080, 720});

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

    std::cout << "End of program." << std::endl;
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

//    // -----------------------------------------
//    // 5. Load shaders
//    // -----------------------------------------
    std::cout << "CWD: " << std::filesystem::current_path() << std::endl;
    VulkanShaderModule vert = VulkanShaderModule(ctx, "spv/spinning_cube.vert.spv");
    VulkanShaderModule frag = VulkanShaderModule(ctx, "spv/spinning_cube.frag.spv");
//
//    // -----------------------------------------
//    // 6. Pipeline layout + descriptor set layout
//    // -----------------------------------------
//    DescriptorSetLayout descriptorLayout = ctx.createDescriptorSetLayout({
//        {0, DescriptorType::UniformBuffer, ShaderStage::Vertex}
//        });
//
//    PipelineLayout pipelineLayout =
//        ctx.createPipelineLayout(&descriptorLayout);
//
//    // -----------------------------------------
//    // 7. Pipeline
//    // -----------------------------------------
//    Pipeline pipeline = PipelineBuilder()
//        .setRenderPass(renderPass)
//        .setPipelineLayout(pipelineLayout)
//        .setShaders(vert, frag)
//        .addVertexBinding<Vertex>()
//        .addVertexAttribute(0, Format::RGB32F, offsetof(Vertex, pos))
//        .addVertexAttribute(1, Format::RGB32F, offsetof(Vertex, color))
//        .enableDepthTest(true)
//        .build(ctx);
//
//    // -----------------------------------------
//    // 8. Buffers (vertex, index, uniform)
//    // -----------------------------------------
//    Buffer vertexBuffer = ctx.createBuffer(
//        BufferUsage::Vertex,
//        cubeVertices, sizeof(cubeVertices)
//    );
//
//    Buffer indexBuffer = ctx.createBuffer(
//        BufferUsage::Index,
//        cubeIndices, sizeof(cubeIndices)
//    );
//
//    Buffer cameraUBO = ctx.createUniformBuffer(sizeof(CameraUBO));
//
//    // -----------------------------------------
//    // 9. Descriptor set
//    // -----------------------------------------
//    DescriptorSet descriptorSet =
//        ctx.allocateDescriptorSet(descriptorLayout);
//
//    descriptorSet.writeUniformBuffer(0, cameraUBO);
//
//    // -----------------------------------------
//    // 10. Frame manager (command buffers + sync)
//    // -----------------------------------------
//    FrameManager frames(ctx, swapchain.getImageCount());
//
//    // -----------------------------------------
//    // Main loop
//    // -----------------------------------------
//    float angle = 0.0f;
//
//    while (!window.shouldClose()) {
//        window.pollEvents();
//
//        float dt = window.getDeltaTime();
//        angle += dt * 1.0f;
//
//        // Update UBO
//        glm::mat4 model = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0, 1, 0));
//
//        glm::mat4 view = glm::lookAt(
//            glm::vec3(3, 3, 3),
//            glm::vec3(0, 0, 0),
//            glm::vec3(0, 1, 0)
//        );
//
//        glm::mat4 proj = glm::perspective(
//            glm::radians(60.0f),
//            swapchain.getAspectRatio(),
//            0.1f, 100.f
//        );
//        proj[1][1] *= -1;
//
//        CameraUBO u;
//        u.viewProj = proj * view * model;
//        cameraUBO.upload(&u, sizeof(u));
//
//        // Acquire frame
//        uint32_t imageIndex = frames.beginFrame(swapchain);
//
//        CommandBuffer& cmd = frames.getCommandBuffer();
//
//        // Begin render pass
//        cmd.begin(renderPass, framebuffers[imageIndex]);
//
//        cmd.bindPipeline(pipeline);
//        cmd.bindDescriptorSet(pipelineLayout, descriptorSet);
//
//        cmd.bindVertexBuffer(vertexBuffer);
//        cmd.bindIndexBuffer(indexBuffer);
//
//        cmd.drawIndexed(std::size(cubeIndices));
//
//        cmd.endRenderPass();
//
//        frames.endFrame(cmd, swapchain, imageIndex);
//    }
//
//    ctx.deviceWaitIdle();
    return 0;
}
