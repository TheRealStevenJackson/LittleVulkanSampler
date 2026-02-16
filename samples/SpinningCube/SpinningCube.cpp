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

    glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-0.2f, -0.2f, -0.2f));

    if (!engine.loadEntityTemporary(pathsToTry, modelMatrix)) {
        std::cerr << "Failed to load mesh. Tried paths:" << std::endl;
        for (const auto& path : pathsToTry) {
            std::cerr << "  - " << path << std::endl;
        }
        return -1;
    }

    glm::mat4 modelMatrix2 = glm::translate(glm::mat4(1.0f), glm::vec3(-0.1f, -0.15f, -0.1f));

    if (!engine.loadEntityTemporary(pathsToTry, modelMatrix2)) {
        std::cerr << "Failed to load mesh. Tried paths:" << std::endl;
        for (const auto& path : pathsToTry) {
            std::cerr << "  - " << path << std::endl;
        }
        return -1;
    }

    glm::mat4 modelMatrix3 = glm::translate(glm::mat4(1.0f), glm::vec3(-0.15f, -0.1f, -0.15f));

    if (!engine.loadEntityTemporary(pathsToTry, modelMatrix3)) {
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

    engine.sceneManager().loadLightTemporary(
        glm::vec4(0.0f, -1.0f, -0.3f, 0.0f),
        glm::vec4(0.8f, 0.8f, 0.75f, 0.0f)
    );

    engine.run();

    return 0;
}
