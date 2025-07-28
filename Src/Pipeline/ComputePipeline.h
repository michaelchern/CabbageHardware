#pragma once

#include <ktm/ktm.h>

#include"../Hardware/DeviceManager.h"
#include"../Hardware/ResourceManager.h"

#include"Compiler/ShaderCodeCompiler.h"

#include"../CabbageDisplayer.h"


struct ComputePipeline
{
	ComputePipeline() = default;
	~ComputePipeline() = default;

	
    ComputePipeline(std::string shaderCode, ShaderLanguage language = ShaderLanguage::GLSL, const std::source_location &sourceLocation = std::source_location::current());

	
    std::variant<HardwarePushConstant> operator[](std::string resourceName)
    {
        // std::string pushConstanName = shaderCodeCompiler.getShaderCode(ShaderLanguage::SpirV).shaderResources.pushConstantName;
        // std::string pushConstanMemberName = resourceName.substr(pushConstanName.size() + 1, resourceName.size());
        ShaderCodeModule::ShaderResources::ShaderBindInfo *resource = shaderCodeCompiler.getShaderCode(ShaderLanguage::SpirV).shaderResources.findPushConstantMembers(resourceName);
        if (resource != nullptr)
        {
            return std::move(HardwarePushConstant(resource->typeSize, resource->byteOffset, &pushConstant));
        }
        else
        {
            throw std::runtime_error("failed to find with name!");
        }
    }

	//init and execute is all used this API
	//please insure that the shader of init and execute is the same
	void executePipeline(ktm::uvec3 groupCount);


private:
	VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
	VkPipeline pipeline = VK_NULL_HANDLE;

	HardwarePushConstant pushConstant;
    //HardwarePushConstant tempPushConstantMember;

    ShaderCodeCompiler shaderCodeCompiler;
	ShaderCodeModule shaderCode;
};