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

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 fragColor;

void main() {
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 0.0, 1.0);
    fragColor = inColor;
}
)";

std::string fragShader = 
R"( 
#version 450

layout(location = 0) in vec3 fragColor;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(fragColor, 1.0);
}
)";


int main()
{
	globalHardwareContext.displayManagers.resize(1);

	HardwareImage finalOutputImage(ktm::uvec2(800,800), ImageFormat::RGBA16_FLOAT, ImageUsage::StorageImage);

	RasterizerPipeline(vertexShader, fragShader);

	if (glfwInit() >= 0)
	{
		GLFWwindow* window;
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		window = glfwCreateWindow(800, 800, "Cabbage Engine", nullptr, nullptr);

		globalHardwareContext.displayManagers[0].initDisplayManager(glfwGetWin32Window(window));

		while (!glfwWindowShouldClose(window))
		{
			glfwPollEvents();
			globalHardwareContext.displayManagers[0].displayFrame(glfwGetWin32Window(window), finalOutputImage.image);
		}

		glfwDestroyWindow(window);

		glfwTerminate();
	}

    return 0;
}