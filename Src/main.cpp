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

			globalHardwareContext.displayManagers[0].displayFrame(glfwGetWin32Window(window), finalOutputImage.image);
		}

		glfwDestroyWindow(window);

		glfwTerminate();
	}

    return 0;
}