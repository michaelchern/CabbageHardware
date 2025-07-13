#version 450

layout(push_constant) uniform PushConsts
{
    mat4 model;
    mat4 view;
    mat4 proj;
    vec3 viewPos;
    vec3 lightColor;
    vec3 lightPos;
} pushConsts;


layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;   // New: Input for vertex normals
layout(location = 2) in vec3 inColor;

// Outputs to the fragment shader
layout(location = 0) out vec3 fragPos;   // Fragment position in world space
layout(location = 1) out vec3 fragNormal; // Fragment normal in world space
layout(location = 2) out vec3 fragColor;

void main() {
    // Transform vertex position to clip space
    gl_Position = pushConsts.proj * pushConsts.view * pushConsts.model * vec4(inPosition, 1.0);

    // Pass necessary values to the fragment shader for lighting calculations
    // Transform position to world space for fragment shader
    fragPos = vec3(pushConsts.model * vec4(inPosition, 1.0));

    // Transform normal to world space (using the normal matrix)
    // The normal matrix is the inverse transpose of the model matrix
    fragNormal = normalize(mat3(transpose(inverse(pushConsts.model))) * inNormal);

    // Pass the input color directly to the fragment shader
    fragColor = inColor;
}