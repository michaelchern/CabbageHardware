#pragma once

#include"../Hardware/DeviceManager.h"
#include"../Hardware/ResourceManager.h"

#include"Compiler/ShaderCodeCompiler.h"

#include "../CabbageDisplayer.h"


struct RasterizerPipeline
{
	struct GeomMeshDrawIndexed
	{
		uint32_t indexOffset;
		uint32_t indexCount;
		ResourceManager::BufferHardwareWrap indexBuffer;
		std::vector<ResourceManager::BufferHardwareWrap> vertexBuffers;

        HardwarePushConstant pushConstant;
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

	//void executePipeline(std::vector<GeomMeshDrawIndexed> GeomMeshes, HardwareImage depthImage, std::vector<HardwareImage> renderTarget);
    void executePipeline(ktm::uvec2 imageSize);

    std::variant<HardwarePushConstant, HardwareBuffer, HardwareImage> operator[](const std::string& resourceName)
    {
        //std::string pushConstanName = vertexShaderCompiler.getShaderCode(ShaderLanguage::SpirV).shaderResources.pushConstantName;
        ShaderCodeModule::ShaderResources::ShaderBindInfo *resource = vertexShaderCompiler.getShaderCode(ShaderLanguage::SpirV).shaderResources.findPushConstantMembers(resourceName);
        if (resource != nullptr)
        {
            //std::string pushConstanMemberName = resourceName.substr(pushConstanName.size() + 1, resourceName.size());

            if (resource != nullptr)
            {
                return std::move(HardwarePushConstant(resource->typeSize, resource->byteOffset, &tempPushConstant));
            }
            else
            {
                ShaderCodeModule::ShaderResources::ShaderBindInfo *resource = vertexShaderCompiler.getShaderCode(ShaderLanguage::SpirV).shaderResources.findPushConstantMembers(resourceName);
                if (resource != nullptr)
                {
                    return std::move(HardwarePushConstant(resource->typeSize, resource->byteOffset, &tempPushConstant));
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


    RasterizerPipeline &startRecord(ktm::uvec2 imageSize);
    RasterizerPipeline &endRecord();
    RasterizerPipeline &operator<<(const RasterizerPipeline &)
    {
        return *this;
    }
    RasterizerPipeline &operator<<(const HardwareBuffer &indexBuffer);

    //void recordGeomMesh()
    //{
    //    TriangleGeomMesh temp;
    //    temp.indexOffset = indexOffset;
    //    temp.indexCount = indexCount;
    //    temp.indexBuffer = indexBuffer;
    //    temp.vertexBuffers = tempVertexBuffers;
    //    temp.pushConstant = tempPushConstant;
    //    geomMeshes.push_back(temp);
    //}

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

	//std::vector<ResourceManager::ImageHardwareWrap> renderTarget;
    HardwareImage depthImage;

	ShaderCodeModule vertShaderCode;
	ShaderCodeModule fragShaderCode;

    //HardwareImage depthImage;
    std::vector<HardwareImage> renderTargets;

    ShaderCodeCompiler vertexShaderCompiler;
    ShaderCodeCompiler fragmentShaderCompiler;

    //struct TriangleGeomMesh
    //{
    //    uint32_t indexOffset;
    //    uint32_t indexCount;
    //    HardwareBuffer indexBuffer;
    //    std::vector<HardwareBuffer> vertexBuffers;

    //    HardwarePushConstant pushConstant;
    //};
    //std::vector<TriangleGeomMesh> geomMeshes;

    HardwarePushConstant tempPushConstant;

    std::vector<HardwareBuffer> tempVertexBuffers;
};