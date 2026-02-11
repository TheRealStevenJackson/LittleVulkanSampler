#include <engine/Engine.h>
#include <engine/scene/object/Entity.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <filesystem>

int main() {
    engine::Engine engine({ "Spinning Cube Sample", 1440, 1440 });

    std::vector<std::string> pathsToTry = {
        "resources/meshes/nes-controller/controller_wireless_1024.obj",
        "../../resources/meshes/nes-controller/controller_wireless_1024.obj",
        "../../../resources/meshes/nes-controller/controller_wireless_1024.obj"
    };

    if (!engine.loadEntityTemporary(pathsToTry)) {
        std::cerr << "Failed to load mesh. Tried paths:" << std::endl;
        for (const auto& path : pathsToTry) {
            std::cerr << "  - " << path << std::endl;
        }
        return -1;
    }

    glm::mat4 view = glm::lookAt(
        glm::vec3(0.1f, 0.1f, 0.1f),
        glm::vec3(0, 0, 0),
        glm::vec3(0, 1, 0)
    );

    glm::mat4 proj = glm::perspective(
        glm::radians(60.0f),
        1.0f,
        0.01f, 200.0f
    );
    proj[1][1] *= -1;

    engine.sceneManager().loadCameraTemporary(view, proj);

    engine.run();

    return 0;
}
