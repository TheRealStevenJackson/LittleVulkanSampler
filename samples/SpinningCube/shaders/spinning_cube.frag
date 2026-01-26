#version 450

layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

// Set = 0, Binding = 1: DirectionalLightUBO
layout(set = 0, binding = 1) uniform DirectionalLightUBO {
    vec4 direction;  // xyz = direction toward light
    vec4 color;      // rgb = light color
} light;

void main()
{
    // Simple Lambertian diffuse lighting
    vec3 N = normalize(fragNormal);
    vec3 L = normalize(-light.direction.xyz);  // Light direction points toward light, so negate for direction from surface
    
    // Calculate diffuse lighting (N dot L)
    float NdotL = max(dot(N, L), 0.0);
    
    // Base color (could use texture, but for now use a simple color)
    vec3 baseColor = vec3(0.8, 0.6, 0.4);  // Warm beige color
    
    // Ambient + diffuse lighting
    vec3 ambient = vec3(0.2) * baseColor;
    vec3 diffuse = light.color.rgb * baseColor * NdotL;
    
    vec3 finalColor = ambient + diffuse;
    
    outColor = vec4(finalColor, 1.0);
}
