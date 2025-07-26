#pragma once

#include <array>
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
#include <volk/volk.h>

#include "FeaturesChain.h"

class DeviceManager
{
  public:
    enum QueueType
    {
        GraphicsQueue = 0,
        ComputeQueue = 1,
        TransferQueue = 2
    };

    struct QueueUtils
    {
        std::shared_ptr<std::mutex> queueMutex;
        uint32_t queueFamilyIndex = -1;
        VkQueue vkQueue = VK_NULL_HANDLE;
        VkCommandPool commandPool = VK_NULL_HANDLE;
        VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
    };

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

    bool operator==(DeviceManager &other)
    {
        return physicalDevice == other.physicalDevice && logicalDevice == other.logicalDevice;
    }

    DeviceManager();

    ~DeviceManager();
     
    void initDeviceManager(const CreateCallback &createCallback, const VkInstance &vkInstance, const VkPhysicalDevice &physicalDevice);


    bool executeSingleTimeCommands(std::function<void(const VkCommandBuffer &commandBuffer)> commandsFunction, QueueType queueType);

    //bool waitALL();


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

    uint16_t getQueueFamilyNumber()
    {
        return queueFamilies.size();
    }

  private:
    void createDevices(const CreateCallback &createInfo, const VkInstance &vkInstance);

    void chooseMainDevice();

    void choosePresentQueueFamily();

    bool createCommandBuffers();

    void createTimelineSemaphore();

    //std::mutex deviceMutex;

    uint64_t semaphoreValue = 0;
    VkSemaphore timelineSemaphore;

    std::atomic_uint16_t currentGraphicsQueueIndex = 0;
    std::atomic_uint16_t currentComputeQueueIndex = 0;
    std::atomic_uint16_t currentTransferQueueIndex = 0;

    std::vector<QueueUtils> graphicsQueues;
    std::vector<QueueUtils> computeQueues;
    std::vector<QueueUtils> transferQueues;

    std::vector<VkQueueFamilyProperties> queueFamilies;
};