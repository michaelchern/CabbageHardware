#pragma once

#include "Hardware/DeviceManager.h"
#include "Hardware/ResourceManager.h"

/**
 * @brief 硬件上下文管理类
 * 
 * 负责Vulkan实例的创建与管理、硬件设备的选择与初始化、
 * 全局资源池的维护等核心硬件抽象功能[6,8](@ref)
 */
struct HardwareContext
{
    /**
     * @brief 构造函数
     * 
     * 初始化硬件上下文，创建Vulkan实例并设置调试层[8](@ref)
     */
    HardwareContext();

    ~HardwareContext();

    /**
     * @brief 硬件工具集结构体
     * 
     * 封装了设备管理和资源管理的核心功能单元[7](@ref)
     */
    struct HardwareUtils
    {
        DeviceManager deviceManager; ///< 设备管理器，负责物理设备和逻辑设备管理[7](@ref)
        ResourceManager resourceManager; ///< 资源管理器，负责GPU资源分配与生命周期管理[7](@ref)
    };

    std::vector<std::shared_ptr<HardwareUtils>> hardwareUtils; ///< 可用的硬件工具集列表（支持多设备）[7](@ref)

    std::shared_ptr<HardwareUtils> mainDevice; ///< 主设备工具集，用于主要渲染和计算任务[7](@ref)

    [[nodiscard]] VkInstance getVulkanInstance() const { return vkInstance; }

    [[nodiscard]] bool getEnableValidationLayer() const { return enableDebugLayer; }

private:
    /**
     * @brief 准备特性链
     * 
     * 配置Vulkan实例和设备所需的特性扩展链[7,8](@ref)
     */
    void prepareFeaturesChain();
    CreateCallback hardwareCreateInfos{}; ///< 硬件创建回调函数集合[7](@ref)

    void createVkInstance(const CreateCallback &createInfo);

    VkInstance vkInstance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;
    bool enableDebugLayer = false;

    void chooseMainDevice();
};

/**
 * @brief 选择主设备
 * 
 * 根据性能特征和功能支持选择最适合的主硬件设备[7](@ref)
 */
extern HardwareContext globalHardwareContext;

// 全局资源池：用于跨设备共享资源[7](@ref)
extern std::unordered_map<uint64_t, ResourceManager::ImageHardwareWrap> imageGlobalPool; ///< 全局图像资源池[7](@ref)
extern std::unordered_map<uint64_t, ResourceManager::BufferHardwareWrap> bufferGlobalPool; ///< 全局缓冲区资源池[7](@ref)