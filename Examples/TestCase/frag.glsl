#version 450

layout(push_constant) uniform PushConsts
{
    mat4 model;
    mat4 view;
    mat4 proj;
} pushConsts;


layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;



const float lineNumber = 19.6332;
const float obliquity = 0.1009;
const float screenWidth = 1440.0;
const float screenHeight = 2560.0;


const float pitch  = (screenWidth * 3. ) / lineNumber;
const float slope  = -obliquity * (screenHeight / screenWidth);
const float center =  -0.43532609939575195;
const float invView = 0.0;

const vec2 quiltSize = vec2(8., 5.);
const float numViews = quiltSize.x * quiltSize.y;
// ------------------------------------------------------------------------

// 1 to render tiled quilt, 0 to render for LKG
#define RENDER_AS_QUILT 0
// There are a few more configuration knobs in other tabs.
#define TMOD (mod(2560, 10.0))
#define USE_PERSPECTIVE_X (TMOD >= 3.0 && TMOD < 5.0)
#define USE_PERSPECTIVE_Y (TMOD < 8.0)
// ------------------------------------------------------------------------

// Helper for quilt
vec2 texArr(vec3 uvz) {
    float z = floor(uvz.z * numViews);
    float x = (mod(z, quiltSize.x) + uvz.x) / quiltSize.x;
    float y = (floor(z / quiltSize.x) + uvz.y) / quiltSize.y;
    return vec2(x, y);
}

void main()
{
    // Width of a subpixel
    float subp = 1.0;

#if RENDER_AS_QUILT
    outColor = texture(texSampler, fragTexCoord);
#else
    vec3 rgb;
    vec2 uv = fragTexCoord;
    for (int chan = 0; chan < 3; ++chan) {
        float z = (uv.x + float(chan) * subp + uv.y * slope) * pitch - center;
        z = mod(z + ceil(abs(z)), 1.0);
        z = (1.0 - invView) * z + invView * (1.0 - z);
        
        vec2 iuv = texArr(vec3(uv, z));
        rgb[chan] = texture(texSampler, iuv)[chan];
    }
    outColor = vec4(rgb, 1.0);
#endif

    // if (fragTexCoord.x < 60.0 && fragTexCoord.y < 60.0) {
    //     outColor = vec4(
    //         float(USE_PERSPECTIVE_X),
    //         float(USE_PERSPECTIVE_Y), 0, 1);
    // }
}
