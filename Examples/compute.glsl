#version 450
#extension GL_EXT_nonuniform_qualifier : enable
layout(local_size_x = 8, local_size_y = 8) in;
layout(set = 0, binding = 3, rgba16) uniform image2D inputImageRGBA16[];
layout(push_constant) uniform PushConsts
{
    uint imageID;
} pushConsts;
void main()
{
    vec4 color = imageLoad(inputImageRGBA16[pushConsts.imageID], ivec2(gl_GlobalInvocationID.xy));
    imageStore(inputImageRGBA16[pushConsts.imageID], ivec2(gl_GlobalInvocationID.xy), color * 1.2);
}