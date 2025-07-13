#version 450

layout(push_constant) uniform PushConsts
{
    mat4 model;
    mat4 view;
    mat4 proj;
    vec3 viewPos; // Camera position in world space
} pushConsts;

layout(location = 0) in vec3 inPosition; // Input vertex position
layout(location = 1) in vec3 inNormal;   // Input vertex normal
layout(location = 2) in vec3 inColor;    // Input vertex color

layout(location = 0) out vec4 outColor;

void main()
{
    // Transform normal to world space
    vec3 norm = normalize(mat3(transpose(inverse(pushConsts.model))) * inNormal);
    // Transform fragment position to world space
    vec3 fragPos = vec3(pushConsts.model * vec4(inPosition, 1.0));

    // --- Ambient light ---
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * inColor;

    // --- Diffuse light (example: directional light) ---
    vec3 lightDir = normalize(vec3(0.5, 1.0, 0.3)); // Example light direction
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * inColor;

    // --- Specular light (example: point light) ---
    vec3 lightPos = vec3(1.0, 1.0, 1.0); // Example light position in world space
    vec3 lightVec = normalize(lightPos - fragPos);
    vec3 viewDir = normalize(pushConsts.viewPos - fragPos);
    vec3 reflectDir = reflect(-lightVec, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32); // 32 is the shininess factor
    vec3 specular = spec * vec3(1.0); // White specular highlight

    // Combine all light components
    outColor = vec4(ambient + diffuse + specular, 1.0);
}