#include "DeviceManager.h"

DeviceManager::DeviceManager()
{
}

DeviceManager::~DeviceManager()
{
}

void DeviceManager::initDeviceManager(const CreateCallback &createCallback, const VkInstance &vkInstance, const VkPhysicalDevice &physicalDevice)
{
    this->physicalDevice = physicalDevice;

    createDevices(createCallback, vkInstance);

    choosePresentQueueFamily();

    createCommandBuffers();

    createTimelineSemaphore();
}

void DeviceManager::createTimelineSemaphore()
{
    // for (size_t i = 0; i < userDevices.size(); i++)
    {
        VkSemaphoreTypeCreateInfo type_create_info{};
        type_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO_KHR;
        type_create_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE_KHR;
        type_create_info.initialValue = 0;

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        semaphoreInfo.pNext = &type_create_info;

        if (vkCreateSemaphore(logicalDevice, &semaphoreInfo, nullptr, &timelineSemaphore) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create synchronization objects for a frame!");
        }
    }
}

void DeviceManager::createDevices(const CreateCallback &initInfo, const VkInstance &vkInstance)
{

    // userDevices.resize(deviceCount);
    // for (uint32_t index = 0; index < deviceCount; index++)
    {
        // physicalDevice = devices[index];

        std::set<const char *> inputExtensions = initInfo.requiredDeviceExtensions(vkInstance, physicalDevice);
        std::vector<const char *> requiredExtensions = std::vector<const char *>(inputExtensions.begin(), inputExtensions.end());

        deviceFeaturesUtils.supportedProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
        deviceFeaturesUtils.rayTracingPipelineProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
        deviceFeaturesUtils.accelerationStructureProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR;
        deviceFeaturesUtils.rayTracingPipelineProperties.pNext = &deviceFeaturesUtils.accelerationStructureProperties;
        deviceFeaturesUtils.supportedProperties.pNext = &deviceFeaturesUtils.rayTracingPipelineProperties;
        deviceFeaturesUtils.accelerationStructureProperties.pNext = nullptr;

        vkGetPhysicalDeviceProperties2(physicalDevice, &deviceFeaturesUtils.supportedProperties);
        std::cout << "---------- GPU " << " : " << deviceFeaturesUtils.supportedProperties.properties.deviceName << "----------" << std::endl;

        {
            uint32_t extensionCount;
            vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);
            std::vector<VkExtensionProperties> availableExtensions(extensionCount);
            vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());

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

        vkGetPhysicalDeviceFeatures2(physicalDevice, deviceFeaturesUtils.featuresChain.getChainHead());

        deviceFeaturesUtils.featuresChain = deviceFeaturesUtils.featuresChain & initInfo.requiredDeviceFeatures(vkInstance, physicalDevice);

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
        queueFamilies.resize(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

        std::vector<std::vector<float>> queuePriorities(queueFamilies.size());

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

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
        createInfo.ppEnabledExtensionNames = requiredExtensions.data();
        createInfo.enabledLayerCount = 0;
        createInfo.ppEnabledLayerNames = nullptr;
        createInfo.pEnabledFeatures = nullptr;
        createInfo.pNext = deviceFeaturesUtils.featuresChain.getChainHead();

        VkResult result = vkCreateDevice(physicalDevice, &createInfo, nullptr, &logicalDevice);
        if (result != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create logical device!");
        }
    }
}

void DeviceManager::choosePresentQueueFamily()
{
    // for (int index = 0; index < userDevices.size(); index++)
    {
        for (int i = 0; i < queueFamilies.size(); i++)
        {
            QueueUtils tempQueueUtils;
            tempQueueUtils.queueFamilyIndex = i;

            for (uint32_t queueIndex = 0; queueIndex < queueFamilies[i].queueCount; queueIndex++)
            {
                tempQueueUtils.queueMutex = std::make_shared<std::mutex>();

                vkGetDeviceQueue(logicalDevice, i, queueIndex, &tempQueueUtils.vkQueue);

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
        }

        if (computeQueues.size() == 0)
        {
            computeQueues.push_back(graphicsQueues[0]);
        }

        if (transferQueues.size() == 0)
        {
            transferQueues.push_back(graphicsQueues[0]);
        }
    }
}

bool DeviceManager::createCommandBuffers()
{
    auto createCommand = [&](DeviceManager::QueueUtils &queues) {
        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = queues.queueFamilyIndex;
        VkResult result = vkCreateCommandPool(logicalDevice, &poolInfo, nullptr, &queues.commandPool);
        if (result != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create graphics command pool!");
        }

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = queues.commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;

        result = vkAllocateCommandBuffers(logicalDevice, &allocInfo, &queues.commandBuffer);
        if (result != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate command buffers!");
        }
    };

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

bool DeviceManager::executeSingleTimeCommands(std::function<void(const VkCommandBuffer &commandBuffer)> commandsFunction, QueueType queueType)
{
    QueueUtils* queue;

    switch (queueType)
    {
    case QueueType::GraphicsQueue: {
        currentGraphicsQueueIndex = (currentGraphicsQueueIndex + 1) % graphicsQueues.size();
        uint16_t queueIndex = currentGraphicsQueueIndex;
        while (true)
        {
            if (graphicsQueues[queueIndex].queueMutex->try_lock())
            {
                break;
            }
            else
            {
                currentGraphicsQueueIndex = (currentGraphicsQueueIndex + 1) % graphicsQueues.size();
                queueIndex = currentGraphicsQueueIndex;
            }
        }
        queue = &graphicsQueues[queueIndex];

        //std::cout << "Graphics Queue Index: " << queueIndex << std::endl;
        break;
    }

    case QueueType::ComputeQueue: {
        currentComputeQueueIndex = (currentComputeQueueIndex + 1) % computeQueues.size();
        uint16_t queueIndex = currentComputeQueueIndex;
        while (true)
        {
            if (computeQueues[queueIndex].queueMutex->try_lock())
            {
                break;
            }
            else
            {
                currentComputeQueueIndex = (currentComputeQueueIndex + 1) % computeQueues.size();
                queueIndex = currentComputeQueueIndex;
            }
        }
        queue = &computeQueues[queueIndex];

        //std::cout << "Compute Queue Index: " << queueIndex << std::endl;
        break;
    }

    case QueueType::TransferQueue: {
        currentTransferQueueIndex = (currentTransferQueueIndex + 1) % transferQueues.size();
        uint16_t queueIndex = currentTransferQueueIndex;
        while (true)
        {
            if (transferQueues[queueIndex].queueMutex->try_lock())
            {
                break;
            }
            else
            {
                currentTransferQueueIndex = (currentTransferQueueIndex + 1) % transferQueues.size();
                queueIndex = currentTransferQueueIndex;
            }
        }
        queue = &transferQueues[queueIndex];

        //std::cout << "Transfer Queue Index: " << queueIndex << std::endl;
        break;
    }
    }


    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = queue->commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(queue->commandBuffer, &beginInfo);

    commandsFunction(queue->commandBuffer);

    vkEndCommandBuffer(queue->commandBuffer);

    VkCommandBufferSubmitInfo commandBufferSubmitInfo{};
    commandBufferSubmitInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
    commandBufferSubmitInfo.commandBuffer = queue->commandBuffer;

    VkSemaphoreSubmitInfo waitSemaphoreSubmitInfo{};
    waitSemaphoreSubmitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
    waitSemaphoreSubmitInfo.semaphore = timelineSemaphore;
    waitSemaphoreSubmitInfo.value = semaphoreValue++;
    waitSemaphoreSubmitInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;

    VkSemaphoreSubmitInfo signalSemaphoreSubmitInfo{};
    signalSemaphoreSubmitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
    signalSemaphoreSubmitInfo.semaphore = timelineSemaphore;
    signalSemaphoreSubmitInfo.value = semaphoreValue;
    signalSemaphoreSubmitInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;

    VkSubmitInfo2 submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
    submitInfo.waitSemaphoreInfoCount = 1;
    submitInfo.pWaitSemaphoreInfos = &waitSemaphoreSubmitInfo;
    submitInfo.signalSemaphoreInfoCount = 1;
    submitInfo.pSignalSemaphoreInfos = &signalSemaphoreSubmitInfo;
    submitInfo.commandBufferInfoCount = 1;
    submitInfo.pCommandBufferInfos = &commandBufferSubmitInfo;

    VkResult result = vkQueueSubmit2(queue->vkQueue, 1, &submitInfo, VK_NULL_HANDLE);

    queue->queueMutex->unlock();

    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to submit command buffer!");
    }

    return true;
}

//bool DeviceManager::waitALL()
//{
//    std::unique_lock<std::mutex> lock(deviceMutex);
//
//    VkSemaphoreWaitInfo waitInfo{};
//    waitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
//    waitInfo.semaphoreCount = 1;
//    waitInfo.pSemaphores = &timelineSemaphore;
//    waitInfo.pValues = &semaphoreValue;
//    vkWaitSemaphores(logicalDevice, &waitInfo, UINT64_MAX);
//
//    return true;
//}
