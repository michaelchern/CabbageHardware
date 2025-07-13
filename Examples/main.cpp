#include <fstream>
#include <regex>
#include <filesystem>

#include <ktm/ktm.h>

#include <GLFW/glfw3.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include "CabbageFramework.h"
#include "PipelineManager/ComputePipeline.h"
#include "PipelineManager/RasterizerPipeline.h"

std::string readStringFile(const std::string_view file_path)
{
	std::ifstream file(file_path.data());
	if (!file.is_open())
	{
		throw std::runtime_error("Could not open the file.");
	}

	std::stringstream buffer;
	buffer << file.rdbuf();

	file.close();
	return buffer.str();
}

std::string shaderPath = [] {
	std::string resultPath = "";
	std::string runtimePath = std::filesystem::current_path().string();
	// std::replace(runtimePath.begin(), runtimePath.end(), '\\', '/');
	std::regex pattern(R"((.*)CabbageDisplay\b)");
	std::smatch matches;
	if (std::regex_search(runtimePath, matches, pattern))
	{
		if (matches.size() > 1)
		{
			resultPath = matches[1].str() + "CabbageDisplay";
		}
		else
		{
			throw std::runtime_error("Failed to resolve source path.");
		}
	}
	std::replace(resultPath.begin(), resultPath.end(), '\\', '/');
	return resultPath + "/Examples";
	}();

std::vector<float> pos = {
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

std::vector<float> normal = {
	// Back face (Z-negative)
	0.0f, 0.0f, -1.0f,  0.0f, 0.0f, -1.0f,  0.0f, 0.0f, -1.0f,  // Triangle 1
	0.0f, 0.0f, -1.0f,  0.0f, 0.0f, -1.0f,  0.0f, 0.0f, -1.0f,  // Triangle 2

	// Front face (Z-positive)
	0.0f, 0.0f, 1.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, 1.0f,   // Triangle 1
	0.0f, 0.0f, 1.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, 1.0f,   // Triangle 2

	// Left face (X-negative)
	-1.0f, 0.0f, 0.0f,  -1.0f, 0.0f, 0.0f,  -1.0f, 0.0f, 0.0f,  // Triangle 1
	-1.0f, 0.0f, 0.0f,  -1.0f, 0.0f, 0.0f,  -1.0f, 0.0f, 0.0f,  // Triangle 2

	// Right face (X-positive)
	1.0f, 0.0f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 0.0f, 0.0f,   // Triangle 1
	1.0f, 0.0f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 0.0f, 0.0f,   // Triangle 2

	// Bottom face (Y-negative)
	0.0f, -1.0f, 0.0f,  0.0f, -1.0f, 0.0f,  0.0f, -1.0f, 0.0f,  // Triangle 1
	0.0f, -1.0f, 0.0f,  0.0f, -1.0f, 0.0f,  0.0f, -1.0f, 0.0f,  // Triangle 2

	// Top face (Y-positive)
	0.0f, 1.0f, 0.0f,   0.0f, 1.0f, 0.0f,   0.0f, 1.0f, 0.0f,   // Triangle 1
	0.0f, 1.0f, 0.0f,   0.0f, 1.0f, 0.0f,   0.0f, 1.0f, 0.0f    // Triangle 2
};

std::vector<float> color = {
	// Back face (Red)
	1.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,
	// Front face (Green)
	0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f,
	// Left face (Blue)
	0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 1.0f,
	// Right face (Yellow)
	1.0f, 1.0f, 0.0f,  1.0f, 1.0f, 0.0f,  1.0f, 1.0f, 0.0f,  1.0f, 1.0f, 0.0f,  1.0f, 1.0f, 0.0f,  1.0f, 1.0f, 0.0f,
	// Bottom face (Cyan)
	0.0f, 1.0f, 1.0f,  0.0f, 1.0f, 1.0f,  0.0f, 1.0f, 1.0f,  0.0f, 1.0f, 1.0f,  0.0f, 1.0f, 1.0f,  0.0f, 1.0f, 1.0f,
	// Top face (Magenta)
	1.0f, 0.0f, 1.0f,  1.0f, 0.0f, 1.0f,  1.0f, 0.0f, 1.0f,  1.0f, 0.0f, 1.0f,  1.0f, 0.0f, 1.0f,  1.0f, 0.0f, 1.0f,
};

std::vector<uint32_t> indices =
{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35 };


int main()
{
	HardwareBuffer postionBuffer = HardwareBuffer(pos, BufferUsage::VertexBuffer);
	HardwareBuffer normalBuffer = HardwareBuffer(normal, BufferUsage::VertexBuffer);
	HardwareBuffer colorBuffer = HardwareBuffer(color, BufferUsage::VertexBuffer);

	HardwareBuffer indexBuffer = HardwareBuffer(indices, BufferUsage::IndexBuffer);

	globalHardwareContext.displayManagers.resize(1);

	HardwareImage finalOutputImage(ktm::uvec2(800, 800), ImageFormat::RGBA16_FLOAT, ImageUsage::StorageImage);

	RasterizerPipeline rasterizer(readStringFile(shaderPath + "/vert.glsl"), readStringFile(shaderPath + "/frag.glsl"));

	ComputePipeline computer(readStringFile(shaderPath + "/compute.glsl"));

	auto startTime = std::chrono::high_resolution_clock::now();

	if (glfwInit() >= 0)
	{
		GLFWwindow* window;
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		window = glfwCreateWindow(800, 800, "Cabbage Engine", nullptr, nullptr);

		globalHardwareContext.displayManagers[0].initDisplayManager(glfwGetWin32Window(window));

		while (!glfwWindowShouldClose(window))
		{
			glfwPollEvents();

			auto currentTime = std::chrono::high_resolution_clock::now();
			float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

			rasterizer["pushConsts.model"] = ktm::rotate3d_axis(time * ktm::radians(90.0f), ktm::fvec3(0.0f, 0.0f, 1.0f));
			rasterizer["pushConsts.view"] = ktm::look_at_lh(ktm::fvec3(2.0f, 2.0f, 2.0f), ktm::fvec3(0.0f, 0.0f, 0.0f), ktm::fvec3(0.0f, 0.0f, 1.0f));
			rasterizer["pushConsts.proj"] = ktm::perspective_lh(ktm::radians(45.0f), 800.0f / 800.0f, 0.1f, 10.0f);
			rasterizer["pushConsts.viewPos"] = ktm::fvec3(2.0f, 2.0f, 2.0f);
			rasterizer["pushConsts.lightPos"] = ktm::fvec3(1.0f, 1.0f, 1.0f);
			rasterizer["pushConsts.lightColor"] = ktm::fvec3(10.0f, 10.0f, 10.0f);
			rasterizer["inPosition"] = postionBuffer;
			rasterizer["inColor"] = colorBuffer;
			rasterizer["inNormal"] = normalBuffer;
			rasterizer["outColor"] = finalOutputImage;
			rasterizer.recordGeomMesh(indexBuffer);
			rasterizer.executePipeline(ktm::uvec2(800, 800));

			computer["pushConsts.imageID"] = finalOutputImage.storeDescriptor();
			computer.executePipeline(ktm::uvec3(800 / 8, 800 / 8, 1));

			globalHardwareContext.displayManagers[0].displayFrame(glfwGetWin32Window(window), finalOutputImage.image);
		}

		glfwDestroyWindow(window);

		glfwTerminate();
	}

	return 0;
}