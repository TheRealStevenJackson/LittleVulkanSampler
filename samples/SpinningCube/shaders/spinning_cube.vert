#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragNormal;
layout(location = 1) out vec2 fragTexCoord;

// Set = 0, Binding = 0: CameraUBO (model, view, proj)
layout(set = 0, binding = 0) uniform CameraUBO {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

// Set = 0, Binding = 1: DirectionalLightUBO
layout(set = 0, binding = 1) uniform DirectionalLightUBO {
    vec4 direction;  // xyz = direction toward light
    vec4 color;      // rgb = light color
} light;

void main()
{
    mat4 mvp = ubo.proj * ubo.view * ubo.model;
    gl_Position = mvp * vec4(inPosition, 1.0);
    
    // Transform normal to world space (using normal matrix from model)
    // For simplicity, we'll use the model matrix's upper-left 3x3
    mat3 normalMatrix = mat3(transpose(inverse(ubo.model)));
    fragNormal = normalize(normalMatrix * inNormal);
    fragTexCoord = inTexCoord;
}
