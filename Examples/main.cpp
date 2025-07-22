#include <filesystem>
#include <fstream>
#include <regex>
#include <sstream>

#include <ktm/ktm.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "CabbageDisplayer.h"
#include "Pipeline/ComputePipeline.h"
#include "Pipeline/RasterizerPipeline.h"
#include <Hardware/GlobalContext.h>

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
    std::regex pattern(R"((.*)CabbageDisplayer\b)");
    std::smatch matches;
    if (std::regex_search(runtimePath, matches, pattern))
    {
        if (matches.size() > 1)
        {
            resultPath = matches[1].str() + "CabbageDisplayer";
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
    0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, // Triangle 1
    0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, // Triangle 2

    // Front face (Z-positive)
    0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // Triangle 1
    0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // Triangle 2

    // Left face (X-negative)
    -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // Triangle 1
    -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // Triangle 2

    // Right face (X-positive)
    1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // Triangle 1
    1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // Triangle 2

    // Bottom face (Y-negative)
    0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, // Triangle 1
    0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, // Triangle 2

    // Top face (Y-positive)
    0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // Triangle 1
    0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f  // Triangle 2
};

std::vector<float> textureUV = {
	// Back face
	0.0f, 0.0f,
	1.0f, 0.0f,
	1.0f, 1.0f,
	1.0f, 1.0f,
	0.0f, 1.0f,
	0.0f, 0.0f,

	// Front face
	0.0f, 0.0f,
	1.0f, 0.0f,
	1.0f, 1.0f,
	1.0f, 1.0f,
	0.0f, 1.0f,
	0.0f, 0.0f,

	// Left face
	0.0f, 0.0f,
	1.0f, 0.0f,
	1.0f, 1.0f,
	1.0f, 1.0f,
	0.0f, 1.0f,
	0.0f, 0.0f,

	// Right face
	0.0f, 0.0f,
	1.0f, 0.0f,
	1.0f, 1.0f,
	1.0f, 1.0f,
	0.0f, 1.0f,
	0.0f, 0.0f,

	// Bottom face
	0.0f, 0.0f,
	1.0f, 0.0f,
	1.0f, 1.0f,
	1.0f, 1.0f,
	0.0f, 1.0f,
	0.0f, 0.0f,

	// Top face
	0.0f, 0.0f,
	1.0f, 0.0f,
	1.0f, 1.0f,
	1.0f, 1.0f,
	0.0f, 1.0f,
	0.0f, 0.0f,
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
    {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35};

int main()
{
    HardwarePushConstant pushConstant;
    pushConstant = 1;
    HardwarePushConstant pushConstant3 = (16,0,nullptr);
    HardwarePushConstant pushConstant2 = (4,0,&pushConstant3);
    pushConstant2 = 1;
    HardwarePushConstant pushConstant4 = 1;
    HardwarePushConstant pushConstant5 = pushConstant4;
    HardwarePushConstant pushConstant6;
    pushConstant6 = pushConstant2;
    HardwarePushConstant pushConstant7;
    pushConstant7 = pushConstant3;

    HardwareBuffer postionBuffer = HardwareBuffer(pos, BufferUsage::VertexBuffer);
    HardwareBuffer normalBuffer = HardwareBuffer(normal, BufferUsage::VertexBuffer);
    HardwareBuffer uvBuffer = HardwareBuffer(textureUV, BufferUsage::VertexBuffer);
    HardwareBuffer colorBuffer = HardwareBuffer(color, BufferUsage::VertexBuffer);

    HardwareBuffer indexBuffer = HardwareBuffer(indices, BufferUsage::IndexBuffer);

    int width, height, channels;
    unsigned char *data = stbi_load(std::string(shaderPath + "/awesomeface.png").c_str(), &width, &height, &channels, 0);
    HardwareImage texture(ktm::uvec2(width, height), ImageFormat::RGBA8_SRGB, ImageUsage::SampledImage, 1, data);

    HardwareImage finalOutputImage(ktm::uvec2(1920, 1080), ImageFormat::RGBA16_FLOAT, ImageUsage::StorageImage);

    RasterizerPipeline rasterizer(readStringFile(shaderPath + "/vert.glsl"), readStringFile(shaderPath + "/frag.glsl"));

    ComputePipeline computer(readStringFile(shaderPath + "/compute.glsl"));

    auto startTime = std::chrono::high_resolution_clock::now();

    if (glfwInit() >= 0)
    {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        GLFWwindow *window0 = glfwCreateWindow(1920, 1080, "Cabbage Engine 0", nullptr, nullptr);
        HardwareDisplayer displayManager0(glfwGetWin32Window(window0));

        GLFWwindow *window1 = glfwCreateWindow(1920, 1080, "Cabbage Engine 1", nullptr, nullptr);
        HardwareDisplayer displayManager1(glfwGetWin32Window(window1));

        GLFWwindow *window2 = glfwCreateWindow(1920, 1080, "Cabbage Engine 2", nullptr, nullptr);
        HardwareDisplayer displayManager2(glfwGetWin32Window(window2));

        GLFWwindow *window3 = glfwCreateWindow(1920, 1080, "Cabbage Engine 3", nullptr, nullptr);
        HardwareDisplayer displayManager3(glfwGetWin32Window(window3));

        while (!glfwWindowShouldClose(window0) &&
               !glfwWindowShouldClose(window1) &&
               !glfwWindowShouldClose(window2) &&
               !glfwWindowShouldClose(window3))
        {
            glfwPollEvents();

            auto currentTime = std::chrono::high_resolution_clock::now();
            float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

            rasterizer["pushConsts.textureIndex"] = texture.storeDescriptor();
            rasterizer["pushConsts.model"] = ktm::rotate3d_axis(time * ktm::radians(90.0f), ktm::fvec3(0.0f, 0.0f, 1.0f));
            rasterizer["pushConsts.view"] = ktm::look_at_lh(ktm::fvec3(2.0f, 2.0f, 2.0f), ktm::fvec3(0.0f, 0.0f, 0.0f), ktm::fvec3(0.0f, 0.0f, 1.0f));
            rasterizer["pushConsts.proj"] = ktm::perspective_lh(ktm::radians(45.0f), 800.0f / 800.0f, 0.1f, 10.0f);
            rasterizer["pushConsts.viewPos"] = ktm::fvec3(2.0f, 2.0f, 2.0f);
            rasterizer["pushConsts.lightPos"] = ktm::fvec3(1.0f, 1.0f, 1.0f);
            rasterizer["pushConsts.lightColor"] = ktm::fvec3(10.0f, 10.0f, 10.0f);
            rasterizer["inPosition"] = postionBuffer;
            rasterizer["inColor"] = colorBuffer;
            rasterizer["inTexCoord"] = uvBuffer;
            rasterizer["inNormal"] = normalBuffer;
            rasterizer["outColor"] = finalOutputImage;
            rasterizer.recordGeomMesh(indexBuffer);
            rasterizer.executePipeline(ktm::uvec2(1920, 1080));

            //computer["pushConsts.imageID"] = finalOutputImage.storeDescriptor();
            //computer.executePipeline(ktm::uvec3(1920 / 8, 1080 / 8, 1));

            displayManager0 = finalOutputImage;
            displayManager1 = finalOutputImage;
            displayManager2 = finalOutputImage;
            displayManager3 = finalOutputImage;
        }

        glfwDestroyWindow(window0);
        glfwDestroyWindow(window1);
        glfwDestroyWindow(window2);
        glfwDestroyWindow(window3);

        glfwTerminate();
    }

    return 0;
}