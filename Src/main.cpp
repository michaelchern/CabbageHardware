#include <iostream>


#include<thread>

#include <GLFW/glfw3.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>


#include <vector>
#include <cstdint>
#include <iostream>
#include <filesystem>
#include <string>
#include <fstream>
#include <sstream>

#include <ktm/ktm.h>

#include "CabbageFramework.h"
#include "PipelineManager/ComputePipeline.h"
#include "PipelineManager/RasterizerPipeline.h"


//#include"Compiler/SourceFilesPath.h"


std::string vertexShader = R"( 
#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

void main() {
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 0.0, 1.0);
    fragColor = inColor;
    fragTexCoord = inTexCoord;
})";

std::string fragShader = R"( 
#version 450

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
)";


int main()
{
	void* surface = nullptr;
    std::thread([&]() {
        if (glfwInit() >= 0)
        {
            GLFWwindow *window;
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            window = glfwCreateWindow(800, 800, "Cabbage Engine", nullptr, nullptr);
            surface = glfwGetWin32Window(window);

            while (!glfwWindowShouldClose(window))
            {
                glfwPollEvents();
            }

            glfwDestroyWindow(window);

            glfwTerminate();
        }
    }).detach();

	RasterizerPipeline(vertexShader, fragShader);

    //CabbageGlobalContext app;
    while (true)
    {

        //app.updateEngine();
    }

    return 0;
}