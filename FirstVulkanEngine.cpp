// FirstVulkanEngine.cpp : Defines the entry point for the application.
//

#include "FirstVulkanEngine.h"
#include "platform/Window.h"
#include "renderengine/RenderEngine.h"


int main() {
    WindowDesc desc;
    desc.title = "LittleVulkanEngine - GLFW Window";
    std::cout << desc.title << std::endl;
    Window window(desc);
    RenderEngine engine(window);


    window.setResizeCallback([](uint32_t w, uint32_t h) {
        std::cout << "Resized to " << w << "x" << h << std::endl;
    });

    while (!window.shouldClose()) {
        window.pollEvents();
        engine.drawFrame();
    }

    return 0;
}
