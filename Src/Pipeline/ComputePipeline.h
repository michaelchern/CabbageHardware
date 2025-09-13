#pragma once

#include <ktm/ktm.h>
#include <source_location>
#include <string>
#include <variant>

#include "Compiler/ShaderCodeCompiler.h"

#include "CabbageDisplayer.h"

// using namespace EmbeddedShader;

// Forward declarations of Vulkan handle types to avoid heavy Vulkan headers here
struct VkPipelineLayout_T;
typedef VkPipelineLayout_T *VkPipelineLayout;
struct VkPipeline_T;
typedef VkPipeline_T *VkPipeline;

struct ComputePipeline
{
    ComputePipeline() = default;
    ~ComputePipeline() = default;

    // 保留原有带参构造（向后兼容）
    ComputePipeline(std::string shaderCode, EmbeddedShader::ShaderLanguage language = EmbeddedShader::ShaderLanguage::GLSL, const std::source_location &sourceLocation = std::source_location::current());

    // 新增：分离初始化接口，允许默认构造后再初始化
    void initialize(std::string shaderCode, EmbeddedShader::ShaderLanguage language = EmbeddedShader::ShaderLanguage::GLSL, const std::source_location &sourceLocation = std::source_location::current());

    std::variant<HardwarePushConstant> operator[](const std::string &resourceName)
    {
        EmbeddedShader::ShaderCodeModule::ShaderResources::ShaderBindInfo *resource = shaderResource.findShaderBindInfo(resourceName);
        if (resource != nullptr && resource->bindType == EmbeddedShader::ShaderCodeModule::ShaderResources::BindType::pushConstantMembers)
        {
            return std::move(HardwarePushConstant(resource->typeSize, resource->byteOffset, &pushConstant));
        }
        else
        {
            throw std::runtime_error("failed to find with name!" + resourceName);
        }
    }

    // init and execute is all used this API
    // please insure that the shader of init and execute is the same
    void executePipeline(ktm::uvec3 groupCount);

  private:
    VkPipelineLayout pipelineLayout{};
    VkPipeline pipeline{};

    bool initialized_ = false; // 初始化状态标记

    HardwarePushConstant pushConstant;
    // HardwarePushConstant tempPushConstantMember;
    EmbeddedShader::ShaderCodeModule::ShaderResources shaderResource;

    EmbeddedShader::ShaderCodeModule shaderCode;
};