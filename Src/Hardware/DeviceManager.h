#pragma once

#include <array>
#include <chrono>
#include <mutex>
#include <memory>
#include <functional>
#include <iostream>
#include <optional>
#include <set>
#include <stdexcept>
#include <vector>

#if defined(_WIN32)
#define VK_USE_PLATFORM_WIN32_KHR
#elif defined(__linux__) || defined(__unix__)
#define VK_USE_PLATFORM_XLIB_KHR
#elif defined(__APPLE__)
#define VK_USE_PLATFORM_MACOS_MVK
#else
#error "Platform not supported by this example."
#endif

#define VK_NO_PROTOTYPES
#include <volk.h>

#include "FeaturesChain.h"

class DeviceManager
{
  public:

    /**
     * @brief 队列类型枚举
     * 
     * 定义设备支持的三种队列类型：
     * - GraphicsQueue: 图形队列，用于渲染操作
     * - ComputeQueue: 计算队列，用于通用计算
     * - TransferQueue: 传输队列，用于数据拷贝
     */
    enum QueueType
    {
        GraphicsQueue = 0,
        ComputeQueue = 1,
        TransferQueue = 2
    };

    /**
     * @brief 队列工具结构体
     * 
     * 封装单个队列的所有相关资源，包括互斥锁、时间线值、
     * 信号量、队列族索引和命令缓冲区等[3](@ref)
     */
    struct QueueUtils
    {
        std::shared_ptr<std::mutex> queueMutex; ///< 互斥锁，确保队列操作的线程安全[3](@ref)
        uint64_t timelineValue = 0; ///< 时间线值，用于时间线信号量的同步[3](@ref)
        VkSemaphore timelineSemaphore; ///< 时间线信号量，实现跨队列和跨命令缓冲区的同步[3](@ref)
        uint32_t queueFamilyIndex = -1; ///< 队列族索引，标识该队列所属的队列族[2](@ref)
        VkQueue vkQueue = VK_NULL_HANDLE; ///< Vulkan队列句柄，用于提交命令[2](@ref)
        VkCommandPool commandPool = VK_NULL_HANDLE; ///< 命令池，分配命令缓冲区的内存[2](@ref)
        VkCommandBuffer commandBuffer = VK_NULL_HANDLE; ///< 命令缓冲区，记录要提交到队列的命令[2](@ref)
    };

    /**
     * @brief 设备特性工具结构体
     * 
     * 封装设备支持的实例扩展、设备扩展
     * 物理设备属性和特性链等信息[4,5](@ref)
     */
    struct FeaturesUtils
    {
        std::set<const char *> instanceExtensions{};
        std::set<const char *> deviceExtensions{};

        VkPhysicalDeviceAccelerationStructurePropertiesKHR accelerationStructureProperties{};
        VkPhysicalDeviceRayTracingPipelinePropertiesKHR rayTracingPipelineProperties{};
        VkPhysicalDeviceProperties2 supportedProperties{};

        DeviceFeaturesChain featuresChain{};
    } deviceFeaturesUtils;

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice logicalDevice = VK_NULL_HANDLE;

    /**
     * @brief 设备比较运算符
     * @param other 另一个设备管理器
     * @return 如果物理设备和逻辑设备相同返回true
     */
    bool operator==(DeviceManager &other)
    {
        return physicalDevice == other.physicalDevice && logicalDevice == other.logicalDevice;
    }

    DeviceManager();

    ~DeviceManager();
     
    /**
     * @brief 初始化设备管理器
     * @param createCallback 创建回调函数
     * @param vkInstance Vulkan实例句柄  
     * @param physicalDevice 物理设备句柄
     */
    void initDeviceManager(const CreateCallback &createCallback, const VkInstance &vkInstance, const VkPhysicalDevice &physicalDevice);


    /**
     * @brief 开始命令记录
     * @param queueType 队列类型，默认为图形队列
     * @return 返回DeviceManager引用以支持链式调用
     */
    DeviceManager &startCommands(QueueType queueType = QueueType::GraphicsQueue);

    /**
     * @brief 结束命令记录并提交
     * @param waitSemaphoreInfos 等待信号量信息数组
     * @param signalSemaphoreInfos 信号信号量信息数组  
     * @param fence 围栏句柄
     * @return 返回DeviceManager引用以支持链式调用
     */
    DeviceManager &endCommands(std::vector<VkSemaphoreSubmitInfo> waitSemaphoreInfos = std::vector<VkSemaphoreSubmitInfo>(),
                               std::vector<VkSemaphoreSubmitInfo> signalSemaphoreInfos = std::vector<VkSemaphoreSubmitInfo>(),
                               VkFence fence = VK_NULL_HANDLE);

    
    DeviceManager &operator<<(const DeviceManager &)
    {
        return *this;
    }


    DeviceManager &operator<<(std::function<void(const VkCommandBuffer &commandBuffer)> commandsFunction);
    
    QueueUtils *currentRecordQueue = nullptr; ///< 当前正在记录的队列
 

    /**
     * @brief 根据条件筛选可用队列[6](@ref)
     * @param required 筛选条件函数
     * @return 满足条件的队列列表
     */
    std::vector<QueueUtils> pickAvailableQueues(std::function<bool(const QueueUtils &)> required)
    {
        std::vector<QueueUtils> result;
        for (size_t i = 0; i < graphicsQueues.size(); i++)
        {
            if (required(graphicsQueues[i]))
            {
                result.push_back(graphicsQueues[i]);
            }
        }
        for (size_t i = 0; i < computeQueues.size(); i++)
        {
            if (required(computeQueues[i]))
            {
                result.push_back(computeQueues[i]);
            }
        }
        for (size_t i = 0; i < transferQueues.size(); i++)
        {
            if (required(transferQueues[i]))
            {
                result.push_back(transferQueues[i]);
            }
        }
        return std::move(result);
    }

    /**
     * @brief 获取队列族数量
     * @return 队列族总数
     */
    uint16_t getQueueFamilyNumber()
    {
        return queueFamilies.size();
    }

  private:
    
    /**
     * @brief 创建设备
     * @param createInfo 创建信息回调
     * @param vkInstance Vulkan实例句柄
     */
    void createDevices(const CreateCallback &createInfo, const VkInstance &vkInstance);

    void chooseMainDevice();

    void choosePresentQueueFamily();

    bool createCommandBuffers();

    void createTimelineSemaphore();

    // 原子队列索引，保证多线程安全[4](@ref)
    std::atomic_uint16_t currentGraphicsQueueIndex = 0;
    std::atomic_uint16_t currentComputeQueueIndex = 0;
    std::atomic_uint16_t currentTransferQueueIndex = 0;

    std::vector<QueueUtils> graphicsQueues;
    std::vector<QueueUtils> computeQueues;
    std::vector<QueueUtils> transferQueues;

    std::vector<VkQueueFamilyProperties> queueFamilies;
};
