#pragma once

#include<set>
#include<stdexcept>
#include<functional>

#define VK_NO_PROTOTYPES
#include<volk/volk.h>

struct DeviceFeaturesChain
{
  public:
    DeviceFeaturesChain();

    VkPhysicalDeviceFeatures2 *getChainHead();

    DeviceFeaturesChain operator&(DeviceFeaturesChain features);
    DeviceFeaturesChain operator&(VkPhysicalDeviceFeatures features);
    DeviceFeaturesChain operator&(VkPhysicalDeviceFeatures2 features);
    DeviceFeaturesChain operator&(VkPhysicalDeviceVulkan11Features features);
    DeviceFeaturesChain operator&(VkPhysicalDeviceVulkan12Features features);
    DeviceFeaturesChain operator&(VkPhysicalDeviceVulkan13Features features);
    DeviceFeaturesChain operator&(VkPhysicalDeviceAccelerationStructureFeaturesKHR features);
    DeviceFeaturesChain operator&(VkPhysicalDeviceRayTracingPipelineFeaturesKHR features);
    DeviceFeaturesChain operator&(VkPhysicalDeviceRayQueryFeaturesKHR features);

    DeviceFeaturesChain operator|(DeviceFeaturesChain features);
    DeviceFeaturesChain operator|(VkPhysicalDeviceFeatures features);
    DeviceFeaturesChain operator|(VkPhysicalDeviceFeatures2 features);
    DeviceFeaturesChain operator|(VkPhysicalDeviceVulkan11Features features);
    DeviceFeaturesChain operator|(VkPhysicalDeviceVulkan12Features features);
    DeviceFeaturesChain operator|(VkPhysicalDeviceVulkan13Features features);
    DeviceFeaturesChain operator|(VkPhysicalDeviceAccelerationStructureFeaturesKHR features);
    DeviceFeaturesChain operator|(VkPhysicalDeviceRayTracingPipelineFeaturesKHR features);
    DeviceFeaturesChain operator|(VkPhysicalDeviceRayQueryFeaturesKHR features);

  private:
    VkPhysicalDeviceFeatures2 deviceFeatures2{};
    VkPhysicalDeviceVulkan11Features deviceFeatures11{};
    VkPhysicalDeviceVulkan12Features deviceFeatures12{};
    VkPhysicalDeviceVulkan13Features deviceFeatures13{};

    VkPhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeatures{};
    VkPhysicalDeviceRayTracingPipelineFeaturesKHR rayTracingPipelineFeatures{};
    VkPhysicalDeviceRayQueryFeaturesKHR rayQueryFeatures{};

    friend DeviceFeaturesChain;
};

struct CreateCallback
{
    std::function<std::set<const char *>(const VkInstance &instance, const VkPhysicalDevice &device)> requiredInstanceExtensions =
        [](const VkInstance &instance, const VkPhysicalDevice &device) { return std::set<const char *>(); };

    std::function<std::set<const char *>(const VkInstance &instance, const VkPhysicalDevice &device)> requiredDeviceExtensions =
        [](const VkInstance &instance, const VkPhysicalDevice &device) { return std::set<const char *>(); };

    std::function<DeviceFeaturesChain(const VkInstance &instance, const VkPhysicalDevice &device)> requiredDeviceFeatures =
        [](const VkInstance &instance, const VkPhysicalDevice &device) { return DeviceFeaturesChain(); };
};
