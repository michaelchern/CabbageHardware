#version 450

layout(push_constant) uniform PushConsts
{
    mat4 model;
    mat4 view;
    mat4 proj;
} pushConsts;

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

void main() {
    gl_Position = pushConsts.proj * pushConsts.view * pushConsts.model * vec4(inPosition, 0.0, 1.0);
    fragColor = inColor;
    fragTexCoord = inTexCoord;
}