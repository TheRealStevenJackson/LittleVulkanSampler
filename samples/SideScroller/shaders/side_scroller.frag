#version 450

layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main()
{
    // Simple lighting for side-scroller
    vec3 lightDir = normalize(vec3(0.5, 0.5, 1.0));
    float diff = max(dot(normalize(fragNormal), lightDir), 0.3);
    
    // Use texture coordinates for a simple pattern
    vec3 color = vec3(0.5 + 0.3 * sin(fragTexCoord.x * 10.0), 
                      0.5 + 0.3 * sin(fragTexCoord.y * 10.0), 
                      0.7);
    
    outColor = vec4(color * diff, 1.0);
}
