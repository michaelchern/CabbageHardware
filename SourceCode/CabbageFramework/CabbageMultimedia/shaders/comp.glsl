#version 450
#extension GL_EXT_nonuniform_qualifier : enable

layout (local_size_x = 8, local_size_y = 8) in;

layout(set = 0, binding = 1) uniform sampler2D textures[];

layout(set = 0, binding = 3, r32ui) uniform uimage2D inputImageRGBA8[];
layout(set = 0, binding = 3, rgba16) uniform image2D inputImageRGBA16[];

layout(set = 0, binding = 3, rgba16) uniform readonly image2DArray inputImage2DArray[];

layout(push_constant) uniform PushConsts
{
    vec3 lightColor;
	uint finalOutputImage;
} pushConsts;


void main()
{
    imageStore(inputImageRGBA16[pushConsts.finalOutputImage], ivec2(gl_GlobalInvocationID.xy), vec4(pushConsts.lightColor, 1.0f));
}