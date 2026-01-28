#version 450

layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in mat3 fragTBN;

layout(location = 0) out vec4 outColor;

// Set = 0, Binding = 1: DirectionalLightUBO
layout(set = 0, binding = 1) uniform DirectionalLightUBO {
    vec4 direction;  // xyz = direction toward light
    vec4 color;      // rgb = light color
} light;

// Set 1: Material textures
layout(set = 1, binding = 0) uniform sampler2D albedoMap;
layout(set = 1, binding = 1) uniform sampler2D normalMap;
layout(set = 1, binding = 2) uniform sampler2D metallicMap;
layout(set = 1, binding = 3) uniform sampler2D roughnessMap;
layout(set = 1, binding = 4) uniform sampler2D aoMap;

// Set 1: Material properties (if textures are not used)
layout(set = 1, binding = 5) uniform Material {
    vec4 albedo;
    float metallic;
    float roughness;
    float ao;
    int useAlbedoMap;
    int useNormalMap;
    int useMetallicMap;
    int useRoughnessMap;
    int useAoMap;
} material;

void main()
{
    // Sample material properties from textures or use uniform values
    vec3 albedoColor = material.useAlbedoMap > 0 
        ? pow(texture(albedoMap, fragTexCoord).rgb, vec3(2.2)) // Gamma correction
        : material.albedo.rgb;
    
    float metallicValue = material.useMetallicMap > 0
        ? texture(metallicMap, fragTexCoord).r
        : material.metallic;
    
    float roughnessValue = material.useRoughnessMap > 0
        ? texture(roughnessMap, fragTexCoord).r
        : material.roughness;
    
    float aoValue = material.useAoMap > 0
        ? texture(aoMap, fragTexCoord).r
        : material.ao;
    
    // Calculate normal - use normal map if available, otherwise use vertex normal
    vec3 N = normalize(fragNormal);
    if (material.useNormalMap > 0) {
        vec3 normal = texture(normalMap, fragTexCoord).rgb * 2.0 - 1.0; // Transform from [0,1] to [-1,1]
        N = normalize(fragTBN * normal);
    }
    
    // Light direction (negate because direction points toward light)
    vec3 L = normalize(-light.direction.xyz);
    
    // Calculate diffuse lighting (N dot L)
    float NdotL = max(dot(N, L), 0.0);
    
    // Simple specular highlight based on roughness
    vec3 V = vec3(0.0, 0.0, 1.0); // View direction (simplified - could use camera position)
    vec3 H = normalize(L + V);
    float NdotH = max(dot(N, H), 0.0);
    float specularPower = (1.0 - roughnessValue) * 32.0; // Higher roughness = lower specular power
    float specular = pow(NdotH, specularPower);
    
    // Mix diffuse and specular based on metallic value
    vec3 diffuseColor = albedoColor * (1.0 - metallicValue);
    vec3 F0 = mix(vec3(0.04), albedoColor, metallicValue);
    vec3 specularColor = F0 * specular * (1.0 - roughnessValue * 0.5); // Tone down specular
    
    // Ambient + diffuse + specular lighting
    vec3 ambient = vec3(0.1) * albedoColor * aoValue; // Reduced ambient from 0.2 to 0.1
    vec3 diffuse = light.color.rgb * diffuseColor * NdotL;
    vec3 spec = light.color.rgb * specularColor * NdotL * 0.5; // Reduce specular intensity by 50%
    
    vec3 finalColor = ambient + diffuse + spec;
    
    // Gamma correction
    finalColor = pow(finalColor, vec3(1.0 / 2.2));
    
    outColor = vec4(finalColor, 1.0);
}
