#include"ComputePipeline.h"

#include<Hardware/GlobalContext.h>


ComputePipeline::ComputePipeline(std::string shaderCode, ShaderLanguage language, const std::source_location &sourceLocation)
    : shaderCodeCompiler(ShaderCodeCompiler(shaderCode, ShaderStage::ComputeShader, language, sourceLocation))
{
    this->shaderCode = shaderCodeCompiler.getShaderCode(ShaderLanguage::SpirV);
    this->pushConstant = HardwarePushConstant(shaderCodeCompiler.getShaderCode(ShaderLanguage::SpirV).shaderResources.pushConstantSize, 0);
}

//ComputePipeline::~ComputePipeline()
//{
//}

void ComputePipeline::executePipeline(ktm::uvec3 groupCount)
{
	if (pipelineLayout == VK_NULL_HANDLE && pipeline == VK_NULL_HANDLE)
	{
		//EmbededShaderCodeGenerator::addPreprocess("layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;");

		//std::string shaderFile = EmbededShader::runDslParse(shaderFunc);
		//std::vector<unsigned int> mSPIRVCode = PipelineBasics::RealtimeShaderCompiler(shaderFile);
		//std::vector<unsigned int> mSPIRVCode = EmbededShaderCodeGenerator::runPipelineParse(shaderFunc, fileName, lineIndex);

		VkComputePipelineCreateInfo computePipelineCreateInfo{};
		computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;

		VkShaderModule shaderModule = globalHardwareContext.resourceManager.createShaderModule(shaderCode);

		computePipelineCreateInfo.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		//computePipelineCreateInfo.stage.flags = VK_SHADER_STAGE_COMPUTE_BIT;
		computePipelineCreateInfo.stage.module = shaderModule;
		computePipelineCreateInfo.stage.pName = "main";
		computePipelineCreateInfo.stage.flags = 0;
		computePipelineCreateInfo.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;

		VkPushConstantRange pushConstant{};
		pushConstant.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
		pushConstant.offset = 0;
		//pushConstant.size = EmbededShader::testAst.getFinalPushConstSize();
		pushConstant.size = shaderCode.shaderResources.pushConstantSize;

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &globalHardwareContext.resourceManager.bindlessDescriptor.descriptorSetLayout;
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstant;

		if (vkCreatePipelineLayout(globalHardwareContext.deviceManager.mainDevice.logicalDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}
		computePipelineCreateInfo.layout = pipelineLayout;

		VkResult result = vkCreateComputePipelines(globalHardwareContext.deviceManager.mainDevice.logicalDevice, VK_NULL_HANDLE, 1, &computePipelineCreateInfo, nullptr, &pipeline);
		if ( result != VK_SUCCESS) {
			throw std::runtime_error("failed to create compute pipeline!");
		}

		vkDestroyShaderModule(globalHardwareContext.deviceManager.mainDevice.logicalDevice, shaderModule, nullptr);
	}

	if (pipelineLayout != VK_NULL_HANDLE && pipeline != VK_NULL_HANDLE)
	{
		auto runCommand = [&](VkCommandBuffer& commandBuffer)
			{
				vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
				vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0, 1, &globalHardwareContext.resourceManager.bindlessDescriptor.descriptorSet, 0, nullptr);

				//void* pushContastValue = EmbededShader::testAst.getFinalPushConstBytes();
                void* data = pushConstant.getData();
				if (data != nullptr)
				{
                    vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, shaderCode.shaderResources.pushConstantSize, data);
				}

				vkCmdDispatch(commandBuffer, groupCount.x, groupCount.y, groupCount.z);
			};

		globalHardwareContext.deviceManager.executeSingleTimeCommands(runCommand);
	}
}
