#include <engine/Engine.h>
#include <game/Entity.h>

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
    const Entity* entity = engine.sceneManager().loadedEntity();

    engine.run([&](float dt) {
        engine.renderFrame(dt);
    });

    return 0;
}
