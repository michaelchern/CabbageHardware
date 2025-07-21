#pragma once

#include "Hardware/DeviceManager.h"
#include "Hardware/ResourceManager.h"

struct HardwareContext
{
    HardwareContext();

    ~HardwareContext() = default;

    DeviceManager deviceManager;
    ResourceManager resourceManager;

    VkInstance getVulkanInstance();

private:
    void prepareFeaturesChain();
    CreateCallback hardwareCreateInfos{};

    void createVkInstance(const CreateCallback &createInfo);

    VkInstance vkInstance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;
};

extern HardwareContext globalHardwareContext;

extern std::unordered_map<uint64_t, ResourceManager::ImageHardwareWrap> imageGlobalPool;
extern std::unordered_map<uint64_t, ResourceManager::BufferHardwareWrap> bufferGlobalPool;