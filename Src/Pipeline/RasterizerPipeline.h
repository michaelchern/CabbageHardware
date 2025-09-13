#pragma once
#include <cstdint>
#include <ktm/ktm.h>
#include <source_location>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

#include "Compiler/ShaderCodeCompiler.h"

#include "CabbageDisplayer.h"

// using namespace EmbeddedShader;

// Forward declare Vulkan handle types to avoid including Vulkan headers here
struct VkRenderPass_T;
typedef VkRenderPass_T *VkRenderPass;
struct VkPipeline_T;
typedef VkPipeline_T *VkPipeline;
struct VkPipelineLayout_T;
typedef VkPipelineLayout_T *VkPipelineLayout;
struct VkFramebuffer_T;
typedef VkFramebuffer_T *VkFramebuffer;

struct RasterizerPipeline
{
    struct GeomMeshDrawIndexed
    {
        uint32_t indexOffset = 0;
        uint32_t indexCount = 0;
        HardwareBuffer indexBuffer;
        std::vector<HardwareBuffer> vertexBuffers;

        HardwarePushConstant pushConstant;
    };

    RasterizerPipeline();
    ~RasterizerPipeline() = default;

    RasterizerPipeline(std::string vertexShaderCode, std::string fragmentShaderCode, uint32_t multiviewCount = 1,
                       EmbeddedShader::ShaderLanguage vertexShaderLanguage = EmbeddedShader::ShaderLanguage::GLSL, EmbeddedShader::ShaderLanguage fragmentShaderLanguage = EmbeddedShader::ShaderLanguage::GLSL,
                       const std::source_location &sourceLocation = std::source_location::current());

    // 新增：分离初始化接口，允许默认构造后再初始化
    void initialize(std::string vertexShaderCode, std::string fragmentShaderCode, uint32_t multiviewCount = 1,
                    EmbeddedShader::ShaderLanguage vertexShaderLanguage = EmbeddedShader::ShaderLanguage::GLSL,
                    EmbeddedShader::ShaderLanguage fragmentShaderLanguage = EmbeddedShader::ShaderLanguage::GLSL,
                    const std::source_location &sourceLocation = std::source_location::current());

    HardwareImage &getDepthImage()
    {
        return depthImage;
    }

    // void executePipeline(std::vector<GeomMeshDrawIndexed> GeomMeshes, HardwareImage depthImage, std::vector<HardwareImage> renderTarget);
    void executePipeline(ktm::uvec2 imageSize);

    std::variant<HardwarePushConstant, HardwareBuffer, HardwareImage> operator[](const std::string &resourceName)
    {
        if (auto *vertexResource = this->vertexResource.findShaderBindInfo(resourceName))
        {
            if (vertexResource->bindType == EmbeddedShader::ShaderCodeModule::ShaderResources::pushConstantMembers)
                return HardwarePushConstant(vertexResource->typeSize, vertexResource->byteOffset, &tempPushConstant);
            if (vertexResource->bindType == EmbeddedShader::ShaderCodeModule::ShaderResources::stageInputs)
                return tempVertexBuffers[vertexResource->location];
            if (vertexResource->bindType == EmbeddedShader::ShaderCodeModule::ShaderResources::stageOutputs)
                return renderTargets[vertexResource->location];
            if (vertexResource->bindType == EmbeddedShader::ShaderCodeModule::ShaderResources::uniformBuffers ||
                vertexResource->bindType == EmbeddedShader::ShaderCodeModule::ShaderResources::sampledImages)
                throw std::runtime_error("Unsupported vertex resource bind type for operator[]: " + resourceName);
        }
        else if (auto *fragmentResource = this->fragmentResource.findShaderBindInfo(resourceName))
        {
            if (fragmentResource->bindType == EmbeddedShader::ShaderCodeModule::ShaderResources::pushConstantMembers)
                return HardwarePushConstant(fragmentResource->typeSize, fragmentResource->byteOffset, &tempPushConstant);
            if (fragmentResource->bindType == EmbeddedShader::ShaderCodeModule::ShaderResources::stageOutputs)
                return renderTargets[fragmentResource->location];
            if (fragmentResource->bindType == EmbeddedShader::ShaderCodeModule::ShaderResources::stageInputs)
                return tempVertexBuffers[fragmentResource->location];
            if (fragmentResource->bindType == EmbeddedShader::ShaderCodeModule::ShaderResources::uniformBuffers ||
                fragmentResource->bindType == EmbeddedShader::ShaderCodeModule::ShaderResources::sampledImages)
                throw std::runtime_error("Unsupported fragment resource bind type for operator[]: " + resourceName);
        }
        return HardwarePushConstant();
    }

    RasterizerPipeline &startRecord(ktm::uvec2 imageSize);
    RasterizerPipeline &endRecord();
    RasterizerPipeline &operator<<(const RasterizerPipeline &)
    {
        return *this;
    }
    RasterizerPipeline &operator<<(const HardwareBuffer &indexBuffer);

    // void recordGeomMesh()
    //{
    //     TriangleGeomMesh temp;
    //     temp.indexOffset = indexOffset;
    //     temp.indexCount = indexCount;
    //     temp.indexBuffer = indexBuffer;
    //     temp.vertexBuffers = tempVertexBuffers;
    //     temp.pushConstant = tempPushConstant;
    //     geomMeshes.push_back(temp);
    // }

  private:
    void createRenderPass(int multiviewCount = 1);
    void createGraphicsPipeline(EmbeddedShader::ShaderCodeModule vertShaderCode, EmbeddedShader::ShaderCodeModule fragShaderCode);
    void createFramebuffers(ktm::uvec2 imageSize);

    uint32_t pushConstantSize;

    int multiviewCount = 1;

    VkRenderPass renderPass{};
    VkPipeline graphicsPipeline{};
    VkPipelineLayout pipelineLayout{};

    VkFramebuffer frameBuffers{};

    // std::vector<ResourceManager::ImageHardwareWrap> renderTarget;
    HardwareImage depthImage;

    EmbeddedShader::ShaderCodeModule vertShaderCode;
    EmbeddedShader::ShaderCodeModule fragShaderCode;

    // HardwareImage depthImage;
    std::vector<HardwareImage> renderTargets;

    // 编译器仅在 initialize 内部临时使用，无需作为成员保留

    // struct TriangleGeomMesh
    //{
    //     uint32_t indexOffset;
    //     uint32_t indexCount;
    //     HardwareBuffer indexBuffer;
    //     std::vector<HardwareBuffer> vertexBuffers;

    //    HardwarePushConstant pushConstant;
    //};
    // std::vector<TriangleGeomMesh> geomMeshes;

    HardwarePushConstant tempPushConstant;

    std::vector<HardwareBuffer> tempVertexBuffers;

    std::vector<EmbeddedShader::ShaderCodeModule::ShaderResources::ShaderBindInfo> vertexStageInputs;
    std::vector<EmbeddedShader::ShaderCodeModule::ShaderResources::ShaderBindInfo> vertexStageOutputs;
    std::vector<EmbeddedShader::ShaderCodeModule::ShaderResources::ShaderBindInfo> fragmentStageInputs;
    std::vector<EmbeddedShader::ShaderCodeModule::ShaderResources::ShaderBindInfo> fragmentStageOutputs;

    EmbeddedShader::ShaderCodeModule::ShaderResources vertexResource;
    EmbeddedShader::ShaderCodeModule::ShaderResources fragmentResource;

    bool initialized_ = false;
};