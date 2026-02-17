#include <engine/Engine.h>
#include <engine/scene/object/Entity.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <filesystem>

int main() {
    engine::Engine engine({ "Spinning Cube Sample", 1440, 1440 });

    engine.loadScene("resources/scenes/SHC Classic Style Porch House EXPORT.glb");

    glm::mat4 view = glm::lookAt(
        glm::vec3(0.0f, 0.0f, 20.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );

    glm::mat4 proj = glm::perspective(
        glm::radians(60.0f),
        1.0f,
        0.1f,
        300.0f
    );
    proj[1][1] *= -1;

    engine.sceneManager().loadCameraTemporary(view, proj);
    if (engine.sceneManager().loadedCamera()) {
        engine.sceneManager().loadedCamera()->setController(&engine.controller());
        engine.sceneManager().loadedCamera()->setMoveSpeed(25.0f);
    }

    engine.sceneManager().loadLightTemporary(
        glm::vec4(0.0f, -1.0f, -0.3f, 0.0f),
        glm::vec4(0.8f, 0.8f, 0.75f, 0.0f)
    );

    engine.run();

    return 0;
}
