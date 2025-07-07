#include <ktm/ktm.h>

#include <GLFW/glfw3.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include "CabbageFramework.h"
#include "PipelineManager/ComputePipeline.h"
#include "PipelineManager/RasterizerPipeline.h"


std::string vertexShader = 
R"( 
#version 450
layout(location = 0) in vec2 inPosition;

void main() {
    gl_Position = vec4(inPosition, 0.0, 1.0);
}
)";

std::string fragShader = 
R"( 
#version 450
layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(0.5, 0.5, 0.5, 1.0);
}
)";

std::string computeShader =
R"( 
#version 450
#extension GL_EXT_nonuniform_qualifier : enable
layout (local_size_x = 8, local_size_y = 8) in;
layout (set = 0, binding = 3, rgba16) uniform image2D inputImageRGBA16[];
layout(push_constant) uniform PushConsts
{
    uint imageID;
} pushConsts;
void main()
{
    vec4 color = imageLoad(inputImageRGBA16[pushConsts.imageID], ivec2(gl_GlobalInvocationID.xy));
	imageStore(inputImageRGBA16[pushConsts.imageID], ivec2(gl_GlobalInvocationID.xy), color * 1.2);
}
)";

std::vector<float> vertices= {
    -0.5f, -0.5f, -0.5f, 
     0.5f, -0.5f, -0.5f, 
     0.5f,  0.5f, -0.5f, 
     0.5f,  0.5f, -0.5f, 
    -0.5f,  0.5f, -0.5f, 
    -0.5f, -0.5f, -0.5f,

    -0.5f, -0.5f,  0.5f,
     0.5f, -0.5f,  0.5f, 
     0.5f,  0.5f,  0.5f, 
     0.5f,  0.5f,  0.5f, 
    -0.5f,  0.5f,  0.5f,
    -0.5f, -0.5f,  0.5f,

    -0.5f,  0.5f,  0.5f, 
    -0.5f,  0.5f, -0.5f, 
    -0.5f, -0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f,
    -0.5f, -0.5f,  0.5f, 
    -0.5f,  0.5f,  0.5f,

     0.5f,  0.5f,  0.5f,
     0.5f,  0.5f, -0.5f, 
     0.5f, -0.5f, -0.5f, 
     0.5f, -0.5f, -0.5f,  
     0.5f, -0.5f,  0.5f, 
     0.5f,  0.5f,  0.5f,

    -0.5f, -0.5f, -0.5f, 
     0.5f, -0.5f, -0.5f, 
     0.5f, -0.5f,  0.5f, 
     0.5f, -0.5f,  0.5f, 
    -0.5f, -0.5f,  0.5f, 
    -0.5f, -0.5f, -0.5f, 

    -0.5f,  0.5f, -0.5f, 
     0.5f,  0.5f, -0.5f, 
     0.5f,  0.5f,  0.5f, 
     0.5f,  0.5f,  0.5f, 
    -0.5f,  0.5f,  0.5f,  
    -0.5f,  0.5f, -0.5f,  
};

std::vector<uint32_t> indices =
{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35 };

int main()
{
	HardwareBuffer postionBuffer = HardwareBuffer(vertices, BufferUsage::VertexBuffer);
    HardwareBuffer indexBuffer = HardwareBuffer(indices, BufferUsage::IndexBuffer);

	globalHardwareContext.displayManagers.resize(1);

	HardwareImage finalOutputImage(ktm::uvec2(800,800), ImageFormat::RGBA16_FLOAT, ImageUsage::StorageImage);

	RasterizerPipeline rasterizer(vertexShader, fragShader);
    
    ComputePipeline computer(computeShader);

	if (glfwInit() >= 0)
	{
		GLFWwindow* window;
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		window = glfwCreateWindow(800, 800, "Cabbage Engine", nullptr, nullptr);

		globalHardwareContext.displayManagers[0].initDisplayManager(glfwGetWin32Window(window));

		while (!glfwWindowShouldClose(window))
		{
			glfwPollEvents();

            rasterizer["inPosition"] = postionBuffer;
            rasterizer["outColor"] = finalOutputImage;
            rasterizer.recordGeomMesh(indexBuffer);
            rasterizer.executePipeline(ktm::uvec2(800, 800));

            computer["pushConsts.imageID"] = finalOutputImage.storeDescriptor();
            computer.executePipeline(ktm::uvec3(800/ 8, 800/ 8, 1));

			globalHardwareContext.displayManagers[0].displayFrame(glfwGetWin32Window(window), finalOutputImage.image);
		}

		glfwDestroyWindow(window);

		glfwTerminate();
	}

    return 0;
}