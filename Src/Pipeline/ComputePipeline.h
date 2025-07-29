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

	
    std::variant<HardwarePushConstant> operator[](const std::string& resourceName)
    {
        auto start = std::chrono::high_resolution_clock::now();
        ShaderCodeModule::ShaderResources::ShaderBindInfo *resource = shaderCodeCompiler.getShaderCode(ShaderLanguage::SpirV).shaderResources.tireTree.findShaderBindInfo(resourceName);
        auto timeD = std::chrono::duration<float, std::chrono::milliseconds::period>(std::chrono::high_resolution_clock::now() - start);
        std::cout << "Average time over " << timeD << std::endl;
        if (resource != nullptr && resource->bindType == ShaderCodeModule::ShaderResources::BindType::pushConstantMembers)
        {
            return std::move(HardwarePushConstant(resource->typeSize, resource->byteOffset, &pushConstant));
        }
        else
        {
            throw std::runtime_error("failed to find with name!" + resourceName);
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