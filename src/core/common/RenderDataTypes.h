#pragma once

#include <glm/glm.hpp>
#include <cstdint>

// Using namespace-like naming or an actual namespace helps prevent
// naming collisions in a "Common" directory.
namespace core {

    enum class ProxyType : uint32_t {
        Model,
        Camera,
        DirectionalLight,
        PointLight
    };

    // --- RENDERER DATA PACKETS ---

    struct CameraData {
        alignas(16) glm::mat4 view;
        alignas(16) glm::mat4 projection;
        alignas(16) glm::vec3 worldPos;
        float padding; // Ensure 16-byte alignment
    };

    struct DirectionalLightData {
        alignas(16) glm::vec4 direction; // w component could be intensity
        alignas(16) glm::vec4 color;
    };

    struct ModelData {
        alignas(16) glm::mat4 modelMatrix;
        uint32_t materialID;
        uint32_t meshID;
        float padding[2]; // Manual padding to keep it 16-byte aligned
    };

    // --- COMPILE TIME SAFETY ---
    static_assert(sizeof(CameraData) % 16 == 0, "CameraData is not 16-byte aligned!");
    static_assert(sizeof(ModelData) % 16 == 0, "ModelData is not 16-byte aligned!");

    enum class RenderDirtyFlags : uint32_t {
        None         = 0,
        Transform    = 1u << 0,
        Type         = 1u << 1,
        CameraData   = 1u << 2,
        LightData    = 1u << 3,
        ModelData    = 1u << 4,
    };

    inline RenderDirtyFlags operator|(RenderDirtyFlags a, RenderDirtyFlags b) {
        return static_cast<RenderDirtyFlags>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
    }
    inline RenderDirtyFlags operator&(RenderDirtyFlags a, RenderDirtyFlags b) {
        return static_cast<RenderDirtyFlags>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
    }
    inline RenderDirtyFlags& operator|=(RenderDirtyFlags& a, RenderDirtyFlags b) {
        a = a | b;
        return a;
    }
    inline bool hasFlag(RenderDirtyFlags flags, RenderDirtyFlags f) {
        return (static_cast<uint32_t>(flags) & static_cast<uint32_t>(f)) != 0;
    }

    struct RenderProxyUpdate {
        ProxyType type;          // The Tag
        uint32_t proxyID;        // Which object are we updating? (0 for new registration)
        glm::mat4 transform;     // OUTSIDE: Every proxy has a location

        union {
            CameraData camera;
            DirectionalLightData light;
            ModelData model;
        } data;
    };

} // namespace core
