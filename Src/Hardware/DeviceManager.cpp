#include "DeviceManager.h"

DeviceManager::DeviceManager()
{
}

DeviceManager::~DeviceManager()
{
    /*if (logicalDevice != VK_NULL_HANDLE)
    {
        vkDeviceWaitIdle(logicalDevice);
        for (size_t i = 0; i < graphicsQueues.size(); i++)
        {
            if (graphicsQueues[i].commandPool != VK_NULL_HANDLE)
            {
                vkDestroyCommandPool(logicalDevice, graphicsQueues[i].commandPool, nullptr);
                graphicsQueues[i].commandPool = VK_NULL_HANDLE;
            }
            if (graphicsQueues[i].timelineSemaphore != VK_NULL_HANDLE)
            {
                vkDestroySemaphore(logicalDevice, graphicsQueues[i].timelineSemaphore, nullptr);
                graphicsQueues[i].timelineSemaphore = VK_NULL_HANDLE;
            }
        }
        for (size_t i = 0; i < computeQueues.size(); i++)
        {
            if (computeQueues[i].commandPool != VK_NULL_HANDLE)
            {
                vkDestroyCommandPool(logicalDevice, computeQueues[i].commandPool, nullptr);
                computeQueues[i].commandPool = VK_NULL_HANDLE;
            }
            if (computeQueues[i].timelineSemaphore != VK_NULL_HANDLE)
            {
                vkDestroySemaphore(logicalDevice, computeQueues[i].timelineSemaphore, nullptr);
                computeQueues[i].timelineSemaphore = VK_NULL_HANDLE;
            }
        }
        for (size_t i = 0; i < transferQueues.size(); i++)
        {
            if (transferQueues[i].commandPool != VK_NULL_HANDLE)
            {
                vkDestroyCommandPool(logicalDevice, transferQueues[i].commandPool, nullptr);
                transferQueues[i].commandPool = VK_NULL_HANDLE;
            }
            if (transferQueues[i].timelineSemaphore != VK_NULL_HANDLE)
            {
                vkDestroySemaphore(logicalDevice, transferQueues[i].timelineSemaphore, nullptr);
                transferQueues[i].timelineSemaphore = VK_NULL_HANDLE;
            }
        }
        vkDestroyDevice(logicalDevice, nullptr);
        logicalDevice = VK_NULL_HANDLE;
    }*/
}

/**
 * @brief 初始化设备管理器
 * @param createCallback 创建回调函数，用于获取设备扩展和特性需求
 * @param vkInstance Vulkan实例句柄
 * @param physicalDevice 物理设备句柄
 * @note 这是设备管理器的主要初始化函数，按顺序执行设备创建、队列选择和资源创建[3,4](@ref)
 */
void DeviceManager::initDeviceManager(const CreateCallback &createCallback, const VkInstance &vkInstance, const VkPhysicalDevice &physicalDevice)
{
    this->physicalDevice = physicalDevice;

    // 步骤1: 创建逻辑设备和获取队列家族信息
    createDevices(createCallback, vkInstance);

    // 步骤2: 选择呈现队列家族并分类队列类型
    choosePresentQueueFamily();

    // 步骤3: 创建命令缓冲区
    createCommandBuffers();

    // 步骤4: 创建时间线信号量用于同步
    createTimelineSemaphore();
}

/**
 * @brief 为每个队列创建时间线信号量
 * @note 时间线信号量用于跨队列同步，可以记录提交的顺序和状态[4](@ref)
 */
void DeviceManager::createTimelineSemaphore()
{
    // Lambda函数：为单个队列创建时间线信号量
    auto createTimelineSemaphore = [&](QueueUtils &queues) {
        VkSemaphoreTypeCreateInfo type_create_info{};
        type_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO_KHR;
        type_create_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE_KHR; // 指定为时间线类型
        type_create_info.initialValue = 0; // 初始值设为0

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        semaphoreInfo.pNext = &type_create_info; // 链接扩展结构

        // 创建信号量
        if (vkCreateSemaphore(logicalDevice, &semaphoreInfo, nullptr, &queues.timelineSemaphore) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create synchronization objects for a frame!");
        }
    };
    
    // 为所有图形队列创建时间线信号量
    for (size_t i = 0; i < graphicsQueues.size(); i++)
    {
        createTimelineSemaphore(graphicsQueues[i]);
    }

    // 为所有计算队列创建时间线信号量
    for (size_t i = 0; i < computeQueues.size(); i++)
    {
        createTimelineSemaphore(computeQueues[i]);
    }

    // 为所有传输队列创建时间线信号量
    for (size_t i = 0; i < transferQueues.size(); i++)
    {
        createTimelineSemaphore(transferQueues[i]);
    }
}

/**
 * @brief 创建逻辑设备并初始化设备特性
 * @param initInfo 初始化信息回调
 * @param vkInstance Vulkan实例
 * @note 此函数负责检查扩展支持、设备特性并创建逻辑设备[1,3](@ref)
 */
void DeviceManager::createDevices(const CreateCallback &initInfo, const VkInstance &vkInstance)
{

    // userDevices.resize(deviceCount);
    // for (uint32_t index = 0; index < deviceCount; index++)
    {
        // physicalDevice = devices[index];

        // 获取所需的设备扩展列表
        std::set<const char *> inputExtensions = initInfo.requiredDeviceExtensions(vkInstance, physicalDevice);
        std::vector<const char *> requiredExtensions = std::vector<const char *>(inputExtensions.begin(), inputExtensions.end());

        // 设置设备特性查询链
        deviceFeaturesUtils.supportedProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
        deviceFeaturesUtils.rayTracingPipelineProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
        deviceFeaturesUtils.accelerationStructureProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR;
        deviceFeaturesUtils.rayTracingPipelineProperties.pNext = &deviceFeaturesUtils.accelerationStructureProperties;
        deviceFeaturesUtils.supportedProperties.pNext = &deviceFeaturesUtils.rayTracingPipelineProperties;
        deviceFeaturesUtils.accelerationStructureProperties.pNext = nullptr;

        // 获取物理设备属性（包括设备名称）
        vkGetPhysicalDeviceProperties2(physicalDevice, &deviceFeaturesUtils.supportedProperties);
        std::cout << "---------- GPU " << " : " << deviceFeaturesUtils.supportedProperties.properties.deviceName << "----------" << std::endl;

        // 检查扩展支持情况
        {
            uint32_t extensionCount;
            vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);
            std::vector<VkExtensionProperties> availableExtensions(extensionCount);
            vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());

            // 验证每个所需扩展是否可用
            for (size_t i = 0; i < requiredExtensions.size(); i++)
            {
                bool extensionSupported = false;
                for (size_t j = 0; j < availableExtensions.size(); j++)
                {
                    if (strcmp(requiredExtensions[i], availableExtensions[j].extensionName) == 0)
                    {
                        extensionSupported = true;
                        break;
                    }
                }
                if (!extensionSupported)
                {
                    std::cerr << "      Extensions Warning : Device not support : " << requiredExtensions[i] << std::endl;

                    requiredExtensions.erase(requiredExtensions.begin() + i);
                    i--;
                }
            }
        }

        // 获取物理设备特性并与应用需求进行与操作
        vkGetPhysicalDeviceFeatures2(physicalDevice, deviceFeaturesUtils.featuresChain.getChainHead());
        deviceFeaturesUtils.featuresChain = deviceFeaturesUtils.featuresChain & initInfo.requiredDeviceFeatures(vkInstance, physicalDevice);

        // 获取队列家族属性
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
        queueFamilies.resize(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

        // 准备队列创建信息
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::vector<std::vector<float>> queuePriorities(queueFamilies.size());

        // 为每个队列家族设置优先级和创建信息
        for (int i = 0; i < queueFamilies.size(); i++)
        {
            queuePriorities[i].resize(queueFamilies[i].queueCount);
            for (int j = 0; j < queuePriorities[i].size(); j++)
            {
                queuePriorities[i][j] = 1.0f;
            }

            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = i;
            queueCreateInfo.queueCount = queueFamilies[i].queueCount;
            queueCreateInfo.pQueuePriorities = queuePriorities[i].data();
            queueCreateInfos.push_back(queueCreateInfo);
        }

        // 设置设备创建信息
        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
        createInfo.ppEnabledExtensionNames = requiredExtensions.data();
        createInfo.enabledLayerCount = 0;
        createInfo.ppEnabledLayerNames = nullptr;
        createInfo.pEnabledFeatures = nullptr;
        createInfo.pNext = deviceFeaturesUtils.featuresChain.getChainHead(); // 设备特性链

        // 创建逻辑设备
        VkResult result = vkCreateDevice(physicalDevice, &createInfo, nullptr, &logicalDevice);
        if (result != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create logical device!");
        }
    }
}

/**
 * @brief 选择呈现队列家族并分类队列类型
 * @note 根据队列能力将队列分类为图形、计算和传输队列[3,4](@ref)
 */
void DeviceManager::choosePresentQueueFamily()
{
    // 遍历所有队列家族
    for (int i = 0; i < queueFamilies.size(); i++)
    {
        QueueUtils tempQueueUtils;
        tempQueueUtils.queueFamilyIndex = i;

        // 为队列家族中的每个队列创建资源
        for (uint32_t queueIndex = 0; queueIndex < queueFamilies[i].queueCount; queueIndex++)
        {
            tempQueueUtils.queueMutex = std::make_shared<std::mutex>(); // 每个队列一个互斥锁
            vkGetDeviceQueue(logicalDevice, i, queueIndex, &tempQueueUtils.vkQueue);

            // 根据队列能力分类
            if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                graphicsQueues.push_back(tempQueueUtils);
            }
            else if (queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT)
            {
                computeQueues.push_back(tempQueueUtils);
            }
            else if (queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT)
            {
                transferQueues.push_back(tempQueueUtils);
            }
        }

        // 确保至少有一个图形队列，并设置回退队列
        if (!graphicsQueues.empty())
        {
            if (computeQueues.empty())
            {
                computeQueues.push_back(graphicsQueues[0]); // 图形队列兼作计算队列
            }
            if (transferQueues.empty())
            {
                transferQueues.push_back(graphicsQueues[0]); // 图形队列兼作传输队列
            }
        }
        else
        {
            throw std::runtime_error("No graphics queues found!");
        }
    }
}

/**
 * @brief 为所有队列创建命令池和命令缓冲区
 * @return 创建成功返回true
 * @note 每个队列都需要独立的命令池来分配命令缓冲区[4](@ref)
 */
bool DeviceManager::createCommandBuffers()
{
    // Lambda函数：为单个队列创建命令池和命令缓冲区
    auto createCommand = [&](DeviceManager::QueueUtils &queues) {
        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // 允许重置命令缓冲区
        poolInfo.queueFamilyIndex = queues.queueFamilyIndex;

        // 创建命令池
        VkResult result = vkCreateCommandPool(logicalDevice, &poolInfo, nullptr, &queues.commandPool);
        if (result != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create graphics command pool!");
        }

        // 分配命令缓冲区
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = queues.commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; // 主命令缓冲区
        allocInfo.commandBufferCount = 1;

        result = vkAllocateCommandBuffers(logicalDevice, &allocInfo, &queues.commandBuffer);
        if (result != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate command buffers!");
        }
    };

    // 为所有类型的队列创建命令缓冲区
    for (size_t i = 0; i < graphicsQueues.size(); i++)
    {
        createCommand(graphicsQueues[i]);
    }
    for (size_t i = 0; i < computeQueues.size(); i++)
    {
        createCommand(computeQueues[i]);
    }
    for (size_t i = 0; i < transferQueues.size(); i++)
    {
        createCommand(transferQueues[i]);
    }

    return true;
}

/**
 * @brief 开始命令录制过程
 * @param queueType 队列类型（图形/计算/传输）
 * @return 返回DeviceManager引用以支持链式调用
 * @note 此函数选择可用队列，开始命令缓冲区录制，并处理同步[4](@ref)
 */
DeviceManager &DeviceManager::startCommands(QueueType queueType)
{
    uint16_t queueIndex = 0;

    while (true)
    {
        // 根据队列类型选择当前队列
        switch (queueType)
        {
        case QueueType::GraphicsQueue:
            queueIndex = currentGraphicsQueueIndex.fetch_add(1) % graphicsQueues.size();
            currentRecordQueue = &graphicsQueues[queueIndex];
            break;
        case QueueType::ComputeQueue:
            queueIndex = currentComputeQueueIndex.fetch_add(1) % computeQueues.size();
            currentRecordQueue = &computeQueues[queueIndex];
            break;
        case QueueType::TransferQueue:
            queueIndex = currentTransferQueueIndex.fetch_add(1) % transferQueues.size();
            currentRecordQueue = &transferQueues[queueIndex];
            break;
        }

        // 尝试锁定队列互斥锁
        if (currentRecordQueue->queueMutex->try_lock())
        {
            // 检查时间线信号量值，确保前一个提交已完成
            uint64_t timelineCounterValue = 0;
            vkGetSemaphoreCounterValue(logicalDevice, currentRecordQueue->timelineSemaphore, &timelineCounterValue);
            if (timelineCounterValue >= currentRecordQueue->timelineValue)
            {
                break; // 队列可用，退出循环
            }
            else
            {
                currentRecordQueue->queueMutex->unlock(); // 队列忙，释放锁继续寻找
            }
        }

        std::this_thread::yield(); // 让出CPU时间片，避免忙等待
    }

    // 重置并开始命令缓冲区录制
    vkResetCommandBuffer(currentRecordQueue->commandBuffer, 0);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // 一次性使用

    vkBeginCommandBuffer(currentRecordQueue->commandBuffer, &beginInfo);

    return *this; // 支持链式调用
}

/**
 * @brief 结束命令录制并提交到队列
 * @param waitSemaphoreInfos 等待的信号量信息
 * @param signalSemaphoreInfos 信号通知的信号量信息
 * @param fence 可选的栅栏用于CPU同步
 * @return 返回DeviceManager引用以支持链式调用
 * @note 此函数结束命令缓冲区，添加时间线信号量同步，并提交到队列[4](@ref)
 */
DeviceManager &DeviceManager::endCommands(std::vector<VkSemaphoreSubmitInfo> waitSemaphoreInfos,
                                          std::vector<VkSemaphoreSubmitInfo> signalSemaphoreInfos,
                                          VkFence fence)
{
    // 结束命令缓冲区录制
    vkEndCommandBuffer(currentRecordQueue->commandBuffer);

    // 设置命令缓冲区提交信息
    VkCommandBufferSubmitInfo commandBufferSubmitInfo{};
    commandBufferSubmitInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
    commandBufferSubmitInfo.commandBuffer = currentRecordQueue->commandBuffer;

    // 添加时间线信号量等待条件（等待前一个提交完成）
    VkSemaphoreSubmitInfo timelineWaitSemaphoreSubmitInfo{};
    timelineWaitSemaphoreSubmitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
    timelineWaitSemaphoreSubmitInfo.semaphore = currentRecordQueue->timelineSemaphore;
    timelineWaitSemaphoreSubmitInfo.value = currentRecordQueue->timelineValue++;
    timelineWaitSemaphoreSubmitInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    waitSemaphoreInfos.push_back(timelineWaitSemaphoreSubmitInfo);

    // 添加时间线信号量信号条件（递增时间线值）
    VkSemaphoreSubmitInfo timelineSignalSemaphoreSubmitInfo{};
    timelineSignalSemaphoreSubmitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
    timelineSignalSemaphoreSubmitInfo.semaphore = currentRecordQueue->timelineSemaphore;
    timelineSignalSemaphoreSubmitInfo.value = currentRecordQueue->timelineValue;
    timelineSignalSemaphoreSubmitInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    signalSemaphoreInfos.push_back(timelineSignalSemaphoreSubmitInfo);

    // 构建提交信息
    VkSubmitInfo2 submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
    submitInfo.waitSemaphoreInfoCount = static_cast<uint32_t>(waitSemaphoreInfos.size());
    submitInfo.pWaitSemaphoreInfos = waitSemaphoreInfos.data();
    submitInfo.signalSemaphoreInfoCount = static_cast<uint32_t>(signalSemaphoreInfos.size());
    submitInfo.pSignalSemaphoreInfos = signalSemaphoreInfos.data();
    submitInfo.commandBufferInfoCount = 1;
    submitInfo.pCommandBufferInfos = &commandBufferSubmitInfo;

    // 提交到队列
    VkResult result = vkQueueSubmit2(currentRecordQueue->vkQueue, 1, &submitInfo, fence);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to submit command buffer!");
    }

    // 释放队列互斥锁
    currentRecordQueue->queueMutex->unlock();

    return *this; // 支持链式调用
}

/**
 * @brief 重载流操作符用于添加命令到命令缓冲区
 * @param commandsFunction 包含Vulkan命令的函数
 * @return 返回DeviceManager引用以支持链式调用
 * @note 提供流畅的API用于添加渲染或计算命令[3](@ref)
 */
DeviceManager &DeviceManager::operator<<(std::function<void(const VkCommandBuffer &commandBuffer)> commandsFunction)
{
    commandsFunction(currentRecordQueue->commandBuffer);
    return *this;
}
