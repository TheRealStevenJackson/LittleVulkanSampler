#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inTangent;
layout(location = 4) in vec3 inBitangent;

layout(location = 0) out vec3 fragNormal;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out mat3 fragTBN;

// Set = 0, Binding = 0: ViewProjUBO (view, proj)
layout(set = 0, binding = 0) uniform ViewProjUBO {
    mat4 view;
    mat4 proj;
} viewProj;

// Set = 0, Binding = 1: DirectionalLightUBO
layout(set = 0, binding = 1) uniform DirectionalLightUBO {
    vec4 direction;  // xyz = direction toward light
    vec4 color;      // rgb = light color
} light;

// Set = 2, Binding = 0: Model matrix
layout(set = 2, binding = 0) uniform ModelUBO {
    mat4 model;
} modelUbo;

void main()
{
    mat4 mvp = viewProj.proj * viewProj.view * modelUbo.model;
    gl_Position = mvp * vec4(inPosition, 1.0);
    
    // Transform normal, tangent, and bitangent to world space
    mat3 normalMatrix = mat3(transpose(inverse(modelUbo.model)));
    fragNormal = normalize(normalMatrix * inNormal);
    vec3 T = normalize(normalMatrix * inTangent);
    vec3 B = normalize(normalMatrix * inBitangent);
    vec3 N = fragNormal;
    
    // TBN matrix for normal mapping (Tangent-Bitangent-Normal)
    fragTBN = mat3(T, B, N);
    
    fragTexCoord = inTexCoord;
}
