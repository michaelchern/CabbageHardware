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
	return resultPath + "/Examples/TestCase";
	}();

const std::vector<ktm::fvec3> pos = {
	{-0.5f, -0.5f, 0.0f},
	{0.5f, -0.5f, 0.0f},
	{0.5f, 0.5f, 0.0f},
	{-0.5f, 0.5f, 0.0f},
	{-0.5f, -0.5f, -0.5f},
	{0.5f, -0.5f, -0.5f},
	{0.5f, 0.5f, -0.5f},
	{-0.5f, 0.5f, -0.5f},
};

const std::vector<ktm::fvec3> color = {
	{1.0f, 0.0f, 0.0f},
	{0.0f, 1.0f, 0.0f},
	{0.0f, 0.0f, 1.0f},
	{1.0f, 1.0f, 1.0f},
 {1.0f, 0.0f, 0.0f},
 {0.0f, 1.0f, 0.0f},
{0.0f, 0.0f, 1.0f},
{1.0f, 1.0f, 1.0f},
};

const std::vector<uint32_t> indices = {
	0, 1, 2, 2, 3, 0,
	4, 5, 6, 6, 7, 4
};

int main()
{
	HardwareBuffer postionBuffer = HardwareBuffer(pos, BufferUsage::VertexBuffer);
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
			rasterizer["inPosition"] = postionBuffer;
			rasterizer["inColor"] = colorBuffer;
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