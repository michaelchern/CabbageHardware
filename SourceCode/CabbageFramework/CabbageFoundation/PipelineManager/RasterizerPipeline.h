#pragma once

#include"../HardwareManager/DeviceManager.h"
#include"../HardwareManager/ResourceManager.h"

#include"CabbageFramework/CabbageCommon/ShaderCodeCompiler/ShaderCodeCompiler.h"

#include "../CabbageFramework.h"


struct RasterizerPipeline
{
	struct GeomMeshDrawIndexed
	{
		uint32_t indexOffset;
		uint32_t indexCount;
		ResourceManager::BufferHardwareWrap indexBuffer;
		std::vector<ResourceManager::BufferHardwareWrap> vertexBuffers;

		void* pushContastValue = nullptr;
	};

	RasterizerPipeline() = default;
	~RasterizerPipeline() = default;

	RasterizerPipeline(std::string vertexShaderCode, std::string fragmentShaderCode, uint32_t multiviewCount = 1,
                       ShaderLanguage vertexShaderLanguage = ShaderLanguage::GLSL, ShaderLanguage fragmentShaderLanguage = ShaderLanguage::GLSL,
                       const std::source_location &sourceLocation = std::source_location::current());

    HardwareImage &getDepthImage()
    {
        return depthImage;
    }

	void executePipeline(std::vector<GeomMeshDrawIndexed> GeomMeshes, HardwareImage depthImage, std::vector<ResourceManager::ImageHardwareWrap> renderTarget);
    void executePipeline(ktm::uvec2 imageSize);

	HardwareResource &operator[](std::string resourceName)
    {
        std::string pushConstanName = vertexShaderCompiler.getShaderCode(ShaderLanguage::SpirV).shaderResources.pushConstantName;
        if (resourceName.substr(0, pushConstanName.size() + 1) == pushConstanName + ".")
        {
            std::string pushConstanMemberName = resourceName.substr(pushConstanName.size() + 1, resourceName.size());
            ShaderCodeModule::ShaderResources::ShaderBindInfo *resource = vertexShaderCompiler.getShaderCode(ShaderLanguage::SpirV).shaderResources.findPushConstantMembers(pushConstanMemberName);
            if (resource != nullptr)
            {
                tempPushConstantMember = HardwareResource(tempPushConstant, resource->byteOffset, resource->typeSize);
                return tempPushConstantMember;
            }
            else
            {
                ShaderCodeModule::ShaderResources::ShaderBindInfo *resource = fragmentShaderCompiler.getShaderCode(ShaderLanguage::SpirV).shaderResources.findPushConstantMembers(pushConstanMemberName);
                if (resource != nullptr)
                {
                    tempPushConstantMember = HardwareResource(tempPushConstant, resource->byteOffset, resource->typeSize);
                    return tempPushConstantMember;
                }
            }
        }
        else
        {
            {
                ShaderCodeModule::ShaderResources::ShaderBindInfo *resource = fragmentShaderCompiler.getShaderCode(ShaderLanguage::SpirV).shaderResources.findStageOutputs(resourceName);
                if (resource != nullptr)
                {
                    return renderTargets[resource->location];
                }
            }
            {
                ShaderCodeModule::ShaderResources::ShaderBindInfo *resource = vertexShaderCompiler.getShaderCode(ShaderLanguage::SpirV).shaderResources.findStageInputs(resourceName);
                if (resource != nullptr)
                {
                    return tempVertexBuffers[resource->location];
                }
            }
        }
        throw resourceName;
    }

    void recordGeomMesh(const HardwareBuffer &indexBuffer, uint32_t indexOffset = 0, uint32_t indexCount = std::numeric_limits<uint32_t>::max())
    {
        TriangleGeomMesh temp;
        temp.indexOffset = indexOffset;
        temp.indexCount = indexCount;
        temp.indexBuffer = indexBuffer;
        temp.vertexBuffers = tempVertexBuffers;
        temp.pushConstant = tempPushConstant;
        geomMeshes.push_back(temp);
    }

private:

	void createRenderPass(int multiviewCount = 1);
	void createGraphicsPipeline(ShaderCodeModule vertShaderCode, ShaderCodeModule fragShaderCode);
	void createFramebuffers(ktm::uvec2 imageSize);
	
	uint32_t pushConstantSize;

	int multiviewCount = 1;

	VkRenderPass renderPass;
	VkPipeline graphicsPipeline;
	VkPipelineLayout pipelineLayout;

	VkFramebuffer frameBuffers;

	std::vector<ResourceManager::ImageHardwareWrap> renderTarget;
    HardwareImage depthImage;

	ShaderCodeModule vertShaderCode;
	ShaderCodeModule fragShaderCode;

    //HardwareImage depthImage;
    std::vector<HardwareImage> renderTargets;

    ShaderCodeCompiler vertexShaderCompiler;
    ShaderCodeCompiler fragmentShaderCompiler;

    struct TriangleGeomMesh
    {
        uint32_t indexOffset;
        uint32_t indexCount;
        HardwareBuffer indexBuffer;
        std::vector<HardwareBuffer> vertexBuffers;

        HardwarePushConstant pushConstant;
    };
    std::vector<TriangleGeomMesh> geomMeshes;

    HardwarePushConstant tempPushConstant;
    HardwareResource tempPushConstantMember;
    std::vector<HardwareBuffer> tempVertexBuffers;
};