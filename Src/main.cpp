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



#include"Compiler/SourceFilesPath.h"



struct CabbageGlobalContext
{
	//CabbageScene cabbageScene;

	struct UniformBufferObject
	{
		ktm::fvec3 lightPostion;
		ktm::fmat4x4 lightViewMatrix;
		ktm::fmat4x4 lightProjMatrix;

		ktm::fvec3 eyePosition;
		ktm::fvec3 eyeDir;
		ktm::fmat4x4 eyeViewMatrix;
		ktm::fmat4x4 eyeProjMatrix;
	}uniformBufferObjects;

	struct gbufferUniformBufferObject
	{
		ktm::fmat4x4 viewProjMatrix;
	}gbufferUniformBufferObjects;

	CabbageGlobalContext(ktm::uvec2 imageSize = ktm::uvec2(1920, 1080))
		:gbufferSize(imageSize), shadowMapSize(ktm::uvec2(2048, 2048)),
		//frameSurface(imageSize),
		finalOutputImage(gbufferSize, ImageFormat::RGBA16_FLOAT, ImageUsage::StorageImage),
		//physicalSimulator(sceneManager),
		shaderPath(
			[] {
				std::string resultPath = "";
				std::string runtimePath = std::filesystem::current_path().string();
				// std::replace(runtimePath.begin(), runtimePath.end(), '\\', '/');
				std::regex pattern(R"((.*)CabbageFramework\b)");
				std::smatch matches;
				if (std::regex_search(runtimePath, matches, pattern))
				{
					if (matches.size() > 1)
					{
						resultPath = matches[1].str() + "CabbageFramework";
					}
					else
					{
						throw std::runtime_error("Failed to resolve source path.");
					}
				}
				std::replace(resultPath.begin(), resultPath.end(), '\\', '/');
				return resultPath + "/SourceCode" + "/CabbageFramework/CabbageMultimedia";
			}()
				)
	{
		gbufferPostionImage = HardwareImage(gbufferSize, ImageFormat::RGBA16_FLOAT, ImageUsage::StorageImage);
		gbufferBaseColorImage = HardwareImage(gbufferSize, ImageFormat::RGBA16_FLOAT, ImageUsage::StorageImage);
		gbufferNormalImage = HardwareImage(gbufferSize, ImageFormat::RGBA16_FLOAT, ImageUsage::StorageImage);
		gbufferMotionVectorImage = HardwareImage(gbufferSize, ImageFormat::RG32_FLOAT, ImageUsage::StorageImage);

		shadowMapPostionImage = HardwareImage(shadowMapSize, ImageFormat::RGBA16_FLOAT, ImageUsage::StorageImage);
		shadowMapBaseColorImage = HardwareImage(shadowMapSize, ImageFormat::RGBA16_FLOAT, ImageUsage::StorageImage);
		shadowMapNormalImage = HardwareImage(shadowMapSize, ImageFormat::RGBA16_FLOAT, ImageUsage::StorageImage);
		shadowMapMotionVectorImage = HardwareImage(shadowMapSize, ImageFormat::RG32_FLOAT, ImageUsage::StorageImage);

		uniformBuffer = HardwareBuffer(sizeof(UniformBufferObject), BufferUsage::UniformBuffer);
		gbufferUniformBuffer = HardwareBuffer(sizeof(gbufferUniformBufferObject), BufferUsage::UniformBuffer);
	}

	//void shadowMapPipeline()
	//{
	//	ktm::fvec4 tempLight = ktm::rotate3d_axis(0.05f * frameIndex++ * ktm::radians(50.0f), ktm::fvec3(0.0f, 1.0f, 0.0f)) * ktm::fvec4(3.0f, 3.0f, 3.0f, 1.0f);
	//	//uniformBufferObjects.lightPostion = tempLight.xyz();
	//	uniformBufferObjects.lightPostion = ktm::fvec3(0.0f, 1.0f, 2.0f);   // ǰ��
	//	//uniformBufferObjects.lightPostion = ktm::fvec3(0.0f, 5.0f, -5.0f);  // ��
	//	//uniformBufferObjects.lightPostion = ktm::fvec3(-5.0f, 5.0f, 5.0f);  // ���
	//	//uniformBufferObjects.lightPostion = ktm::fvec3(5.0f, 5.0f, 0.0f);   // �Ҳ�
	//	//uniformBufferObjects.lightPostion = ktm::fvec3(0.0f, 5.0f, 0.0f);   // �Ϸ�

	//	float sceneRadius = 3.0f; // �������뾶
	//	shadowMapFrustumSize = ktm::fvec2(sceneRadius * 2, sceneRadius * 2);
	//	//uniformBufferObjects.lightProjMatrix = ktm::ortho_lh(-sceneRadius, sceneRadius, -sceneRadius, sceneRadius, 0.1f, 10.0f);
	//	uniformBufferObjects.lightProjMatrix = ktm::perspective_lh(ktm::radians(100.0f), (float)shadowMapSize.x / (float)shadowMapSize.y, 0.1f, 50.0f);
	//	uniformBufferObjects.lightViewMatrix = ktm::look_at_lh(uniformBufferObjects.lightPostion, ktm::fvec3(0.0f, 0.0f, 0.0f), ktm::fvec3(0.0f, -1.0f, 0.0f));

	//	for (auto& model : sceneManager.sceneActors)
	//	{
	//		gbufferUniformBufferObjects.modelMatrix = model.second.modelMatrix;
	//		gbufferUniformBufferObjects.viewProjMatrix = uniformBufferObjects.lightProjMatrix * uniformBufferObjects.lightViewMatrix;

	//		gbufferUniformBuffer.copyFromData(&gbufferUniformBufferObjects, sizeof(gbufferUniformBufferObjects));

	//		shadowMapPipleLine["pushConsts.uniformBufferIndex"] = gbufferUniformBuffer.storeDescriptor();
	//		shadowMapPipleLine["pushConsts.boneIndex"] = model.second.bonesMatrixBuffer.storeDescriptor();

	//		for (auto& geom : model.second.geomMeshes)
	//		{
	//			shadowMapPipleLine["inPosition"] = geom.pointsBuffer;
	//			shadowMapPipleLine["inNormal"] = geom.normalsBuffer;
	//			shadowMapPipleLine["inTexCoord"] = geom.texCoordsBuffer;
	//			shadowMapPipleLine["boneIndexes"] = geom.boneIndexesBuffer;
	//			shadowMapPipleLine["jointWeights"] = geom.boneWeightsBuffer;
	//			shadowMapPipleLine["pushConsts.textureIndex"] = geom.textureIndex;

	//			shadowMapPipleLine.recordGeomMesh(geom.indexBuffer);
	//		}
	//	}

	//	shadowMapPipleLine["gbufferPostion"] = shadowMapPostionImage;
	//	shadowMapPipleLine["gbufferBaseColor"] = shadowMapBaseColorImage;
	//	shadowMapPipleLine["gbufferNormal"] = shadowMapNormalImage;
	//	shadowMapPipleLine["gbufferMotionVector"] = shadowMapMotionVectorImage;
	//	shadowMapPipleLine.executePipeline(shadowMapSize);
	//}

	void gbufferPipeline(ktm::fvec2 jitterOffset)
	{
		uniformBufferObjects.eyePosition = sceneCamera.cameraPosition;
		uniformBufferObjects.eyeDir = ktm::normalize(sceneCamera.cameraForward);
		//jitterOffset = { 0.0f, 0.0f };
		ktm::fmat4x4 jitterTranslationMatrix = ktm::translate3d(ktm::fvec3(jitterOffset.x / gbufferSize.x, jitterOffset.y / gbufferSize.y, 0.0));


		uniformBufferObjects.eyeViewMatrix = ktm::look_at_lh(uniformBufferObjects.eyePosition, ktm::normalize(sceneCamera.cameraForward), sceneCamera.cameraWorldUp);
		uniformBufferObjects.eyeProjMatrix = jitterTranslationMatrix * ktm::perspective_lh(ktm::radians(45.0f), (float)gbufferSize.x / (float)gbufferSize.y, 0.1f, 100.0f);

		gbufferUniformBufferObjects.viewProjMatrix = uniformBufferObjects.eyeProjMatrix * uniformBufferObjects.eyeViewMatrix;
		gbufferUniformBuffer.copyFromData(&gbufferUniformBufferObjects, sizeof(gbufferUniformBufferObjects));

		//for (auto& model : sceneManager.sceneActors)
		//{
  //          gbufferPipleLine["pushConsts.modelMatrix"] = model.second.modelMatrix;

		//	gbufferPipleLine["pushConsts.uniformBufferIndex"] = gbufferUniformBuffer.storeDescriptor();
		//	gbufferPipleLine["pushConsts.boneIndex"] = model.second.bonesMatrixBuffer.storeDescriptor();

		//	for (auto& geom : model.second.geomMeshes)
		//	{
		//		gbufferPipleLine["inPosition"] = geom.pointsBuffer;
		//		gbufferPipleLine["inNormal"] = geom.normalsBuffer;
		//		gbufferPipleLine["inTexCoord"] = geom.texCoordsBuffer;
		//		gbufferPipleLine["boneIndexes"] = geom.boneIndexesBuffer;
		//		gbufferPipleLine["jointWeights"] = geom.boneWeightsBuffer;
		//		gbufferPipleLine["pushConsts.textureIndex"] = geom.textureIndex;

		//		gbufferPipleLine.recordGeomMesh(geom.indexBuffer);
		//	}
		//}

		//gbufferPipleLine["gbufferPostion"] = gbufferPostionImage;
		//gbufferPipleLine["gbufferBaseColor"] = gbufferBaseColorImage;
		//gbufferPipleLine["gbufferNormal"] = gbufferNormalImage;
		//gbufferPipleLine["gbufferMotionVector"] = gbufferMotionVectorImage;
		//gbufferPipleLine.executePipeline(gbufferSize);
	}


	void compositePipeline(ktm::fvec3 sunDir = ktm::fvec3(0.0, 1.0, 0.0))
	{
		rendererPipleLine["pushConsts.gbufferSize"] = gbufferSize;
		rendererPipleLine["pushConsts.gbufferPostionImage"] = gbufferPostionImage.storeDescriptor();
		rendererPipleLine["pushConsts.gbufferBaseColorImage"] = gbufferBaseColorImage.storeDescriptor();
		rendererPipleLine["pushConsts.gbufferNormalImage"] = gbufferNormalImage.storeDescriptor();
		rendererPipleLine["pushConsts.gbufferDepthImage"] = gbufferPipleLine.getDepthImage().storeDescriptor();

		//rendererPipleLine["pushConsts.shadowMapSize"] = shadowMapSize;
		//rendererPipleLine["pushConsts.shadowMapFrustumSize"] = shadowMapFrustumSize;
		//rendererPipleLine["pushConsts.shadowMapDepthImage"] = shadowMapPipleLine.getDepthImage().storeDescriptor();
		//rendererPipleLine["pushConsts.shadowMapBaseColorImage"] = shadowMapBaseColorImage.storeDescriptor();

		//rendererPipleLine["pushConsts.guiOutputImage"] = guiOutputImage.storeDescriptor();
		rendererPipleLine["pushConsts.finalOutputImage"] = finalOutputImage.storeDescriptor();

		// without screenspace it will boom 
		//rendererPipleLine["pushConsts.camera_near"] = 0.1f;
		//rendererPipleLine["pushConsts.camera_far"] = 100.0f;


		rendererPipleLine["pushConsts.sun_dir"] = ktm::normalize(sunDir);


		rendererPipleLine["pushConsts.lightColor"] = ktm::fvec3(23.47f, 21.31f, 20.79f);

		uniformBuffer.copyFromData(&uniformBufferObjects, sizeof(uniformBufferObjects));
		rendererPipleLine["pushConsts.uniformBufferIndex"] = uniformBuffer.storeDescriptor();

		rendererPipleLine.executePipeline(ktm::uvec3(gbufferSize.x / 8, gbufferSize.y / 8, 1));
	}

	std::string shaderPath;

	ktm::uvec2 shadowMapSize;
	ktm::fvec2 shadowMapFrustumSize;
	HardwareImage shadowMapPostionImage;
	HardwareImage shadowMapBaseColorImage;
	HardwareImage shadowMapNormalImage;
	HardwareImage shadowMapMotionVectorImage;
	RasterizerPipeline shadowMapPipleLine = RasterizerPipeline(CabbageFiles::readStringFile(shaderPath + "/shaders/test.vert.glsl"), CabbageFiles::readStringFile(shaderPath + "/shaders/test.frag.glsl"));

	ktm::uvec2 gbufferSize;
	HardwareImage gbufferPostionImage;
	HardwareImage gbufferBaseColorImage;
	HardwareImage gbufferNormalImage;
	HardwareImage gbufferMotionVectorImage;
	RasterizerPipeline gbufferPipleLine = RasterizerPipeline(CabbageFiles::readStringFile(shaderPath + "/shaders/test.vert.glsl"), CabbageFiles::readStringFile(shaderPath + "/shaders/test.frag.glsl"));


	HardwareBuffer uniformBuffer;
	HardwareBuffer gbufferUniformBuffer;

	//HardwareImage guiOutputImage;
	HardwareImage finalOutputImage;

	ComputePipeline rendererPipleLine = ComputePipeline(CabbageFiles::readStringFile(shaderPath + "/shaders/test.comp.glsl"));


	ComputePipeline testComputePipleLine = ComputePipeline(CabbageFiles::readStringFile(shaderPath + "/shaders/comp.glsl"));

	// frameIndex = 90��270��450��630��ʱ�򣬹�Դ�ֱ������ҡ�����������ǰ
	// uint64_t frameIndex = 400;
	uint64_t frameIndex = 540;


	//HardwareFrameSurface frameSurface;


	//EngineOperateList::CameraManager sceneCamera;
	struct CameraManager
	{
		float cameraFov = 45.0f;
		ktm::fvec3 cameraPosition = ktm::fvec3(1.0f, 1.0f, 1.0f);
		ktm::fvec3 cameraForward = ktm::fvec3(-1.0f, -1.0f, -1.0f);
		ktm::fvec3 cameraWorldUp = ktm::fvec3(0.0f, 1.0f, 0.0f);
	}sceneCamera;

	//SceneManager sceneManager;

	//PhysicalSimulator physicalSimulator;

	ktm::fvec3 sunDir = ktm::fvec3(0.0, 1.0, 0.0);
	void updateEngine()
	{
		//sceneCamera = cmdList.sceneCamera;

		//sceneManager.updateScene(cmdList);
		//physicalSimulator.update();

		//cmdList.clearList();

		//shadowMapPipeline();

		gbufferPipeline(ktm::fvec2(0.0, 0.0));

		compositePipeline(sunDir);

		//{
  //          testComputePipleLine["pushConsts.finalOutputImage"] = finalOutputImage.storeDescriptor();
  //          testComputePipleLine["pushConsts.lightColor"] = ktm::fvec3(0.5f, 0.1f, 0.1f);
  //          testComputePipleLine.executePipeline(ktm::uvec3(gbufferSize.x / 8, gbufferSize.y / 8, 1));
		//}

		//if (globalHardwareContext.displayManagers.size() != CabbageEngine::sceneList.size())
		//{
		//    globalHardwareContext.displayManagers.resize(CabbageEngine::sceneList.size());
		//    for (size_t i = 0; i < CabbageEngine::sceneList.size(); i++)
		//    {
		//        globalHardwareContext.displayManagers[i].initDisplayManager(CabbageEngine::sceneList[i]->surface);
		//    }
		//}
		//for (size_t i = 0; i < CabbageEngine::sceneList.size(); i++)
		//{
		//    globalHardwareContext.displayManagers[i].displayFrame(CabbageEngine::sceneList[i]->surface, finalOutputImage.image);
		//}
	}
};

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