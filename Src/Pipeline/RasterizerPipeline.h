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
        ShaderCodeModule::ShaderResources::ShaderBindInfo *vertexResource = this->vertexResource.findShaderBindInfo(resourceName);
        if (vertexResource != nullptr)
        {
            switch (vertexResource->bindType)
            {
                case ShaderCodeModule::ShaderResources::BindType::pushConstantMembers:
                    return std::move(HardwarePushConstant(vertexResource->typeSize, vertexResource->byteOffset, &tempPushConstant));
                case ShaderCodeModule::ShaderResources::BindType::stageInputs:
                    return tempVertexBuffers[vertexResource->location];
            }
        }
        else
        {
            ShaderCodeModule::ShaderResources::ShaderBindInfo *fragmentResource = this->fragmentResource.findShaderBindInfo(resourceName);
            switch (fragmentResource->bindType)
            {
            case ShaderCodeModule::ShaderResources::BindType::pushConstantMembers:
                return std::move(HardwarePushConstant(fragmentResource->typeSize, fragmentResource->byteOffset, &tempPushConstant));
            case ShaderCodeModule::ShaderResources::BindType::stageOutputs:
                return renderTargets[fragmentResource->location];
            }
        }
        return std::move(HardwarePushConstant());
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

    std::vector<ShaderCodeModule::ShaderResources::ShaderBindInfo> vertexStageInputs;
    std::vector<ShaderCodeModule::ShaderResources::ShaderBindInfo> vertexStageOutputs;
    std::vector<ShaderCodeModule::ShaderResources::ShaderBindInfo> fragmentStageInputs;
    std::vector<ShaderCodeModule::ShaderResources::ShaderBindInfo> fragmentStageOutputs;

    ShaderCodeModule::ShaderResources vertexResource;
    ShaderCodeModule::ShaderResources fragmentResource;
};